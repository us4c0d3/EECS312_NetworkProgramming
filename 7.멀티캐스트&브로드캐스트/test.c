#include <stdio.h>
#include <stdlib.h>
#include <time.h>
int main() {
    srand(time(NULL));

    // from 10000 to 50000 random numbers print loop
    for(int i = 0; i < 10000; i++) {
        printf("%d\n", rand() % 40001 + 10000);
    }
}