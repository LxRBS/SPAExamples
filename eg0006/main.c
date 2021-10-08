#include <stdio.h>


int const A[] = {6, 12, 23, 31, 35, 46, 49, 54, 62, 78, 81, 88, 93, 97};
int const N = 14;

int linearSearch(int const a[], int n, int value){
    for(int i=0;i<n;++i){
        if(a[i]==value){
            return i;
        }else if(a[i]>value){
            break;
        }
    }
    return -1;
}

int main(){
    int a = 54, b = 77;
    int ra = linearSearch(A, N, a);
    int rb = linearSearch(A, N, b);
    printf("%d %d\n", ra, rb);
    return 0;
}
