#include <stdio.h>

int main(){

    int a = 32|31;
    if(a)
        printf("yes %d\n", a);
    else
        printf("No : %d\n", a);
}