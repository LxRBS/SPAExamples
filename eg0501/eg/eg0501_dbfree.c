#include <malloc.h>
void mn(){
    int * p = (int *)malloc(sizeof(int));
    free(p);
    free(p);
    int * q = (int *)malloc(sizeof(int));
    free(q);
    q = (int *)malloc(sizeof(int));
    free(q);
}
