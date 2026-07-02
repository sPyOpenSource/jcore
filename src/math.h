#ifndef __MYOS__MATH_H
#define __MYOS__MATH_H

extern "C" {
    double fmod(double x, double y);
    float fmodf(float x, float y);
    double sin(double x);
    float sinf(float x);
    double cos(double x);
    float cosf(float x);
    double sqrt(double x);
    float sqrtf(float x);
    double floor(double x);
    float floorf(float x);
    double ceil(double x);
    float ceilf(float x);
    double pow(double x, double y);
    float powf(float x, float y);
}

#endif
