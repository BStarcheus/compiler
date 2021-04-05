#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

// Array of string pointers to be freed later
static const int arrLen = 2;
static char* allocatedStrings[arrLen];
// Track the index to insert into
static int allocatedStringsIndex = 0;
// If the array is full, and wrapped around to start storing from front again
static bool wrapped = false;

/*
Issue:  If you wrap around and overwrite elements of array, 
        undefined behavior with LLVM values.
        Some LLVM variables still have access to that index of the array.
*/

// Store the string pointer in the array to be freed later
// Wrap around to beginning if array full
void storeStringPtr(char* ptr) {
    if (allocatedStringsIndex == arrLen) {
        wrapped = true;
        allocatedStringsIndex = 0;
    }

    if (wrapped) {
        // Free the pointer so you can store another
        free(allocatedStrings[allocatedStringsIndex]);
    }
    allocatedStrings[allocatedStringsIndex++] = ptr;
}

void freeAllStrings() {
    int start = 0;
    int end = allocatedStringsIndex;
    if (wrapped) {
        // Every element is used. Free the whole array.
        end = arrLen;
    }
    for (int i = start; i < end; i++) {
        free(allocatedStrings[i]);
    }
}

// Runtime functions

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

char* getstring() {
    char *s = malloc(256 * sizeof(char));
    fgets(s, 256, stdin);

    // Remove newline char
    if ((strlen(s) > 0) && (s[strlen(s) - 1] == '\n')) {
        s[strlen(s) - 1] = '\0';
    }
    
    // Prevent memory leaks.
    // Store this pointer so all strings can be freed
    // after the entire program is done running.
    // The better solution would be freeing after they are 
    // no longer needed, but that requires a lot more logic.
    storeStringPtr(s);
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