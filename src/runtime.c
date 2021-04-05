#include <stdbool.h>
#include <stdio.h>
#include <math.h>

bool getbool() {
    int b;
    scanf("%i", &b);
    return b != 0;
}

int getinteger() {
    int i;
    scanf("%i", &i);
    return i;
}

float getfloat() {
    float f;
    scanf("%f", &f);
    return f;
}

char* getstring() { // BROKEN
    char* s;
    scanf("%s", s);
    return s;
}

bool putbool(bool value) {
    printf("%i\n", value);
    return true;
}

bool putinteger(int value) {
    printf("%i\n", value);
    return true;
}

bool putfloat(float value) {
    printf("%f\n", value);
    return true;
}

bool putstring(char* value) {
    printf("%s\n", value);
    return true;
}

float sqrt_(int value) {
    return sqrtf((float)value);
}