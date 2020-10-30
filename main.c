#include <stdio.h>
#include <pthread.h>
#include <string.h>

pthread_mutex_t x_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t x_cond = PTHREAD_COND_INITIALIZER;

int main() {
    printf("Hello, World!\n");
    return 0;
}
