#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    
    int *shared_mem = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    
    *shared_mem = 0;  
    
    pid_t pid = fork();
    
    if (pid == 0) { 
        printf("Child init: %d\n", *shared_mem);
        *shared_mem += 10;
        printf("Child modified: %d\n", *shared_mem);
        exit(0);
    } 
    else { 
        wait(NULL); 
        printf("Parent modified: %d\n", *shared_mem);

    }
    
    return 0;
}

