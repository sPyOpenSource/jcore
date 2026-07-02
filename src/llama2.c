#include "llama2.h"
#include <stddef.h>

#define memcpy __builtin_memcpy
#define memset __builtin_memset

#ifdef __ARM_NEON
#include <arm_neon.h>
#endif

// Scratch bump allocator for RunState (model weights are linked separately)
#define SCRATCH_SIZE (10 * 1024 * 1024)
static char scratch[SCRATCH_SIZE];
static unsigned scratch_off;

static void scratch_reset(void) { scratch_off = 0; }

static void *scratch_alloc(int sz) {
    unsigned a = (scratch_off + 15) & ~15;
    if (a + sz > SCRATCH_SIZE) return 0;
    void *p = scratch + a;
    scratch_off = a + sz;
    memset(p, 0, sz);
    return p;
}

// --- Minimal math for baremetal ---

static float sqrtf_(float x) {
    float r;
    __asm__("vsqrt.f32 %0, %1" : "=t"(r) : "t"(x));
    return r;
}

static float expf_(float x) {
    if (x < -20.0f) return 0.0f;
    if (x > 20.0f) return 4.85e8f;
    const float ln2 = 0.69314718056f;
    const float inv_ln2 = 1.44269504089f;
    float n = (float)(int)(x * inv_ln2);
    float t = x - n * ln2;
    float y = 1.0f + t * (0.99999994f + t * (0.49999870f + t * (0.16666728f
        + t * (0.04166540f + t * 0.00832981f))));
    union { float f; int i; } u;
    u.i = ((int)n + 127) << 23;
    return y * u.f;
}

static float sinf_(float x) {
    const float pi = 3.141592653589793f;
    const float pi2 = 6.283185307179586f;
    x = x - (float)(int)(x / pi2) * pi2;
    if (x < 0) x += pi2;
    int sign = 1;
    if (x > pi) { x -= pi; sign = -1; }
    if (x > pi * 0.5f) x = pi - x;
    float x2 = x * x;
    float y = x + x * x2 * (-1.0f/6.0f + x2 * (1.0f/120.0f
        + x2 * (-1.0f/5040.0f + x2 * (1.0f/362880.0f))));
    return sign * y;
}

static float cosf_(float x) { return sinf_(x + 1.5707963267948966f); }

// --- RMSNorm ---

static void rmsnorm(float *o, float *x, float *weight, int n) {
    float ss = 0.0f;
    for (int j = 0; j < n; j++) ss += x[j] * x[j];
    ss = 1.0f / sqrtf_(ss / n + 1e-5f);
    for (int j = 0; j < n; j++) o[j] = weight[j] * (ss * x[j]);
}

// --- Softmax ---

static void softmax(float *x, int n) {
    float mx = x[0];
    for (int i = 1; i < n; i++) if (x[i] > mx) mx = x[i];
    float sum = 0.0f;
    for (int i = 0; i < n; i++) { x[i] = expf_(x[i] - mx); sum += x[i]; }
    for (int i = 0; i < n; i++) x[i] /= sum;
}

// --- Matmul (NEON-accelerated) ---

static void matmul(float *xout, float *x, float *w, int n, int d) {
    int i = 0;
#ifdef __ARM_NEON
    for (; i + 4 <= d; i += 4) {
        float32x4_t sum0 = vdupq_n_f32(0);
        float32x4_t sum1 = vdupq_n_f32(0);
        float32x4_t sum2 = vdupq_n_f32(0);
        float32x4_t sum3 = vdupq_n_f32(0);
        float *w0 = w + (i + 0) * n;
        float *w1 = w + (i + 1) * n;
        float *w2 = w + (i + 2) * n;
        float *w3 = w + (i + 3) * n;
        int j = 0;
        for (; j + 4 <= n; j += 4) {
            float32x4_t xv = vld1q_f32(x + j);
            sum0 = vmlaq_f32(sum0, xv, vld1q_f32(w0 + j));
            sum1 = vmlaq_f32(sum1, xv, vld1q_f32(w1 + j));
            sum2 = vmlaq_f32(sum2, xv, vld1q_f32(w2 + j));
            sum3 = vmlaq_f32(sum3, xv, vld1q_f32(w3 + j));
        }
        xout[i+0] = sum0[0] + sum0[1] + sum0[2] + sum0[3];
        xout[i+1] = sum1[0] + sum1[1] + sum1[2] + sum1[3];
        xout[i+2] = sum2[0] + sum2[1] + sum2[2] + sum2[3];
        xout[i+3] = sum3[0] + sum3[1] + sum3[2] + sum3[3];
        for (; j < n; j++) {
            xout[i+0] += w0[j] * x[j];
            xout[i+1] += w1[j] * x[j];
            xout[i+2] += w2[j] * x[j];
            xout[i+3] += w3[j] * x[j];
        }
    }
#endif
    for (; i < d; i++) {
        float sum = 0.0f;
        for (int j = 0; j < n; j++) sum += w[i * n + j] * x[j];
        xout[i] = sum;
    }
}

// --- Build Transformer from linked model data ---

int build_transformer(Transformer *t, const char *model_data) {
    const int *hdr = (const int *)model_data;
    Config *p = &t->config;
    p->dim = hdr[0];
    p->hidden_dim = hdr[1];
    p->n_layers = hdr[2];
    p->n_heads = hdr[3];
    p->n_kv_heads = hdr[4];
    int vs = hdr[5];
    p->seq_len = hdr[6];
    int shared = vs > 0;
    p->vocab_size = vs < 0 ? -vs : vs;

    // Validate
    if (p->dim <= 0 || p->hidden_dim <= 0 || p->n_layers <= 0) return -1;
    if (p->n_heads <= 0 || p->n_kv_heads <= 0 || p->n_heads % p->n_kv_heads != 0) return -1;

    int head_size = p->dim / p->n_heads;
    int kv_dim = (p->dim * p->n_kv_heads) / p->n_heads;
    unsigned long long nl = p->n_layers;

    // Weight pointers: the weights start at byte offset 7*4 = 28 (7 int32 header)
    const float *wp = (const float *)(model_data + 28);
    TransformerWeights *w = &t->weights;

    w->shared_weights = shared;
    w->token_embedding_table = (float *)wp; wp += p->vocab_size * p->dim;
    w->rms_att_weight = (float *)wp; wp += nl * p->dim;
    w->wq = (float *)wp; wp += nl * p->dim * p->dim;
    w->wk = (float *)wp; wp += nl * p->dim * kv_dim;
    w->wv = (float *)wp; wp += nl * p->dim * kv_dim;
    w->wo = (float *)wp; wp += nl * p->dim * p->dim;
    w->rms_ffn_weight = (float *)wp; wp += nl * p->dim;
    w->w1 = (float *)wp; wp += nl * p->dim * p->hidden_dim;
    w->w2 = (float *)wp; wp += nl * p->hidden_dim * p->dim;
    w->w3 = (float *)wp; wp += nl * p->dim * p->hidden_dim;
    w->rms_final_weight = (float *)wp; wp += p->dim;
    wp += p->seq_len * head_size / 2; // skip old freq_cis_real
    wp += p->seq_len * head_size / 2; // skip old freq_cis_imag
    w->wcls = shared ? w->token_embedding_table : (float *)wp;

    // Allocate run state
    scratch_reset();
    RunState *s = &t->state;
    s->head_size = head_size;
    s->x = scratch_alloc(p->dim * 4);
    s->xb = scratch_alloc(p->dim * 4);
    s->xb2 = scratch_alloc(p->dim * 4);
    s->hb = scratch_alloc(p->hidden_dim * 4);
    s->hb2 = scratch_alloc(p->hidden_dim * 4);
    s->q = scratch_alloc(p->dim * 4);
    s->att = scratch_alloc(p->n_heads * p->seq_len * 4);
    s->logits = scratch_alloc(p->vocab_size * 4);
    s->key_cache = scratch_alloc(nl * p->seq_len * kv_dim * 4);
    s->value_cache = scratch_alloc(nl * p->seq_len * kv_dim * 4);
    s->rope_sin = scratch_alloc(p->seq_len * (head_size / 2) * 4);
    s->rope_cos = scratch_alloc(p->seq_len * (head_size / 2) * 4);

    if (!s->x || !s->xb || !s->xb2 || !s->hb || !s->hb2 || !s->q
        || !s->att || !s->logits || !s->key_cache || !s->value_cache
        || !s->rope_sin || !s->rope_cos) return -2;

    // Precompute RoPE sin/cos tables
    for (int pos = 0; pos < p->seq_len; pos++) {
        for (int i = 0; i < head_size; i += 2) {
            float freq = 1.0f;
            // powf(10000, 2*i/head_size) where i is already the dim index
            // In the original: freq = 1/powf(10000, head_dim/head_size) where head_dim = i % head_size
            // So head_dim = i (since i < head_size in this inner loop):
            float e = (float)i / (float)head_size;
            float pow_val = 1.0f;
            // 10000^e = expf(e * ln(10000))
            pow_val = expf_(e * 9.210340371976184f);
            freq = 1.0f / pow_val;
            float val = pos * freq;
            s->rope_sin[pos * (head_size/2) + i/2] = sinf_(val);
            s->rope_cos[pos * (head_size/2) + i/2] = cosf_(val);
        }
    }

    return 0;
}

// --- Forward pass ---

float *forward(Transformer *t, int token, int pos) {
    Config *p = &t->config;
    TransformerWeights *w = &t->weights;
    RunState *s = &t->state;

    int dim = p->dim;
    int kv_dim = (dim * p->n_kv_heads) / p->n_heads;
    int kv_mul = p->n_heads / p->n_kv_heads;
    int hidden_dim = p->hidden_dim;
    int head_size = s->head_size;
    float *x = s->x;

    // Token embedding
    memcpy(x, w->token_embedding_table + token * dim, dim * 4);

    for (unsigned long long l = 0; l < (unsigned long long)p->n_layers; l++) {
        // Attention RMSNorm
        rmsnorm(s->xb, x, w->rms_att_weight + l * dim, dim);

        // KV cache pointers for this position
        int loff = l * p->seq_len * kv_dim;
        float *k_ptr = s->key_cache + loff + pos * kv_dim;
        float *v_ptr = s->value_cache + loff + pos * kv_dim;

        // QKV matmuls
        matmul(s->q, s->xb, w->wq + l * dim * dim, dim, dim);
        matmul(k_ptr, s->xb, w->wk + l * dim * kv_dim, dim, kv_dim);
        matmul(v_ptr, s->xb, w->wv + l * dim * kv_dim, dim, kv_dim);

        // RoPE: rotate q and k
        for (int i = 0; i < dim; i += 2) {
            int half = i / 2;
            int head_half = half % (head_size / 2);
            float fcr = s->rope_cos[pos * (head_size / 2) + head_half];
            float fci = s->rope_sin[pos * (head_size / 2) + head_half];
            int rotn = i < kv_dim ? 2 : 1;
            for (int v = 0; v < rotn; v++) {
                float *vec = v == 0 ? s->q : k_ptr;
                float v0 = vec[i];
                float v1 = vec[i + 1];
                vec[i]     = v0 * fcr - v1 * fci;
                vec[i + 1] = v0 * fci + v1 * fcr;
            }
        }

        // Multihead attention
        for (int h = 0; h < p->n_heads; h++) {
            float *q = s->q + h * head_size;
            float *att = s->att + h * p->seq_len;
            for (int t2 = 0; t2 <= pos; t2++) {
                float *k = s->key_cache + loff + t2 * kv_dim + (h / kv_mul) * head_size;
                float score = 0.0f;
                for (int i = 0; i < head_size; i++) score += q[i] * k[i];
                att[t2] = score / sqrtf_(head_size);
            }
            softmax(att, pos + 1);
            float *xb = s->xb + h * head_size;
            memset(xb, 0, head_size * 4);
            for (int t2 = 0; t2 <= pos; t2++) {
                float *v = s->value_cache + loff + t2 * kv_dim + (h / kv_mul) * head_size;
                float a = att[t2];
                for (int i = 0; i < head_size; i++) xb[i] += a * v[i];
            }
        }

        // Attention output projection
        matmul(s->xb2, s->xb, w->wo + l * dim * dim, dim, dim);

        // Residual
        for (int i = 0; i < dim; i++) x[i] += s->xb2[i];

        // FFN RMSNorm
        rmsnorm(s->xb, x, w->rms_ffn_weight + l * dim, dim);

        // FFN: w1 and w3
        matmul(s->hb, s->xb, w->w1 + l * dim * hidden_dim, dim, hidden_dim);
        matmul(s->hb2, s->xb, w->w3 + l * dim * hidden_dim, dim, hidden_dim);

        // SwiGLU
        for (int i = 0; i < hidden_dim; i++) {
            float val = s->hb[i];
            val *= 1.0f / (1.0f + expf_(-val));
            val *= s->hb2[i];
            s->hb[i] = val;
        }

        // FFN output projection
        matmul(s->xb, s->hb, w->w2 + l * dim * hidden_dim, hidden_dim, dim);

        // Residual
        for (int i = 0; i < dim; i++) x[i] += s->xb[i];
    }

    // Final RMSNorm
    rmsnorm(x, x, w->rms_final_weight, dim);

    // Classifier
    matmul(s->logits, x, w->wcls, dim, p->vocab_size);

    return s->logits;
}

// --- Sampling ---

int sample_argmax(float *logits, int n) {
    int max_i = 0;
    float max_p = logits[0];
    for (int i = 1; i < n; i++) {
        if (logits[i] > max_p) { max_i = i; max_p = logits[i]; }
    }
    return max_i;
}

// --- Decode (simplified, shows printable bytes, token IDs otherwise) ---

static void print_token(int token, void (*putchar)(char)) {
    if (token == 0) return;                  // <unk>
    if (token == 1) return;                  // <s> BOS
    if (token == 2) { putchar('\n'); return; } // </s> EOS
    if (token >= 3 && token <= 258) {          // raw byte
        char c = (char)(token - 3);
        if (c >= 32 && c <= 126) { putchar(c); return; }
        if (c == '\n') { putchar('\r'); putchar('\n'); return; }
        if (c == '\t') { putchar('\t'); return; }
        return; // skip non-printables
    }
    // Show merge token ID as decimal
    int v = token;
    char digits[8];
    int nd = 0;
    if (v == 0) { digits[0] = '0'; nd = 1; }
    else { while (v) { digits[nd++] = '0' + (v % 10); v /= 10; } }
    putchar('[');
    for (int i = nd - 1; i >= 0; i--) putchar(digits[i]);
    putchar(']');
}

// --- Generation loop (outputs to user-provided putchar function) ---

void generate(Transformer *t, int steps, void (*putchar)(char)) {
    Config *p = &t->config;
    if (steps <= 0 || steps > p->seq_len) steps = p->seq_len;

    int token = 1; // BOS
    int next = 0;
    int pos = 0;

    while (pos < steps) {
        float *logits = forward(t, token, pos);

        next = sample_argmax(logits, p->vocab_size);
        pos++;

        if (next == 1) break; // BOS = stop

        print_token(next, putchar);

        if (next == 2) break; // EOS = stop

        token = next;
    }
}
