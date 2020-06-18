/*
 * mkbootimg/img.c
 *
 * Copyright (C) 2017 - 2020 bzt (bztsrc@gitlab)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * This file is part of the BOOTBOOT Protocol package.
 * @brief Write disk image to file
 *
 */
#include "main.h"

/**
 * Assemble and write out disk image
 */
void img_write(char *fn)
{
    FILE *f, *d;
    int i, n, lastpercent, k;
    char key[64], *tmp, *buf;
    unsigned long int size, pos;
    size_t s;
    time_t c = 0;

    buf = malloc(1024*1024);
    if(!buf) { fprintf(stderr,"mkbootimg: unable to allocate memory\r\n"); exit(2); }

    f=fopen(fn,"wb");
    if(!f) { fprintf(stderr,"mkbootimg: unable to write %s\n", fn); exit(3); }
    /* write out primary GPT table (and optional ISO9660 header) */
    fwrite(gpt,es*512,1,f);
    /* write out ESP */
    fwrite(esp,esp_size,1,f);
    fseek(f,(es+esiz)*512,SEEK_SET);
    /* write out other partitions */
    for(k = 1; k < np; k++) {
        size = 0;
        sprintf(key, "partitions.%d.%s", k, "file");
        tmp = json_get(json, key);
        if(tmp && *tmp) {
            d = fopen(tmp, "rb");
            free(tmp);
            if(d) {
                while((s = fread(buf, 1, 1024*1024, d)) != 0) {
                    fwrite(buf, 1, s, f);
                    size += s;
                    if(c > t + 1) {
                        pos = ftell(f);
                        n = pos * 100L / (tsize + 1);
                        if(n != lastpercent) {
                            lastpercent = n;
                            printf("\rmkbootimg: writing [");
                            for(i = 0; i < 20; i++) printf(i < n/5 ? "#" : " ");
                            printf("] %3d%% ", n);
                            fflush(stdout);
                        }
                    } else
                        time(&c);
                }
                fclose(d);
            }
        }
        fseek(f,gpt_parts[k] - size,SEEK_CUR);
    }
    /* write out backup GPT table */
    fseek(f,tsize-63*512,SEEK_SET);
    fwrite(gpt+1024,62*512,1,f);
    fwrite(gpt2,512,1,f);
    fclose(f);
    printf("\r\x1b[K\rmkbootimg: %s saved.\r\n", fn);
    free(buf);
}
