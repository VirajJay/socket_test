#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

/* Function Prototypes */
void create_thread(void);
void* func_thr(void* args);

void main()
{
    create_thread();
    printf("Hello world\n");
    pause();
}

void create_thread(void)
{
    pthread_t thr_1;
    int* pNum = malloc(sizeof(int));
    *pNum = 434;
    printf("pNum: %lx\n", pNum);
    printf("num: %d\n", *pNum);

    pthread_create(&thr_1, NULL, func_thr, pNum);

    // free(pNum);

    printf("pNum: %lx\n", pNum);
    printf("num: %d\n", *pNum);
}

void* func_thr(void* args)
{
    int* plol = (int*) args;
    int lol = *plol;
    printf("lol: %d\n", lol);

    free(plol);
}
