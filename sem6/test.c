#include <stdio.h>
#include <stdlib.h>

int main() {
    char *ptr = (char *)malloc(2); // Allocating 2 bytes
    if (!ptr) {
        perror("malloc failed");
        return 1;
    }

    ptr[0] = 'A';
    ptr[1] = 'B';

    printf("Allocated memory contains: %c%c\n", ptr[0], ptr[1]);

    free(ptr); // Always free allocated memory
    return 0;
}
