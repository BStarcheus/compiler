#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

// Runtime functions

bool getbool() {
    int b;
    scanf("%i", &b);
    getchar(); // eat newline
    return b != 0;
}

int getinteger() {
    int i;
    scanf("%i", &i);
    getchar(); // eat newline
    return i;
}

float getfloat() {
    float f;
    scanf("%f", &f);
    getchar(); // eat newline
    return f;
}

char* getstring() {
    char *s = malloc(256 * sizeof(char));
    fgets(s, 256, stdin);

    // Remove newline char
    if ((strlen(s) > 0) && (s[strlen(s) - 1] == '\n')) {
        s[strlen(s) - 1] = '\0';
    }
    
    // TODO Prevent memory leak :/
    // See commits of what I tried: 
    //      3647f3d5cdb83b9b241a984e497715caa6462185
    //      c1e315dd653813f6ad9bdf7038f4b028c9a3e54e
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

float _sqrt(int value) {
    return sqrtf((float)value);
}

void outOfBoundsError() {
    printf("Error: Index out of bounds\n");
    exit(1);
}