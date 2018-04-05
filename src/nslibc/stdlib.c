#include <stdlib.h>

void srand(unsigned int seed) {
    randseed = seed;
}

int rand() {
    val = ((randseed * 1103515245) + 12345) & 0x7fffffff;
    randseed = val;
    return val;
}
