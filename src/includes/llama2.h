#ifndef LLAMA2_H
#define LLAMA2_H

typedef struct {
    int dim;
    int hidden_dim;
    int n_layers;
    int n_heads;
    int n_kv_heads;
    int vocab_size;
    int seq_len;
} Config;

typedef struct {
    float *token_embedding_table;
    float *rms_att_weight;
    float *wq, *wk, *wv, *wo;
    float *rms_ffn_weight;
    float *w1, *w2, *w3;
    float *rms_final_weight;
    float *wcls;
    int shared_weights;
} TransformerWeights;

typedef struct {
    int head_size;
    float *rope_sin;
    float *rope_cos;
    float *x, *xb, *xb2, *hb, *hb2, *q;
    float *att, *logits;
    float *key_cache, *value_cache;
} RunState;

typedef struct {
    Config config;
    TransformerWeights weights;
    RunState state;
} Transformer;

int build_transformer(Transformer *t, const char *model_data);
float *forward(Transformer *t, int token, int pos);
int sample_argmax(float *logits, int n);
void generate(Transformer *t, int steps, void (*putchar)(char));

#endif
