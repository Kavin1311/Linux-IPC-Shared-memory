#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>

#define TEXT_SZ 2048

struct shared_use_st {
    int written_by_you;
    char some_text[TEXT_SZ];
};

int main() {
    int running = 1;
    void *shared_memory = NULL;  // Properly initialize shared_memory
    struct shared_use_st *shared_stuff; 
    char buffer[BUFSIZ];
    int shmid;

    // Create shared memory segment
    shmid = shmget((key_t)1234, sizeof(struct shared_use_st), 0666 | IPC_CREAT);
    if (shmid == -1) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }

    // Attach to shared memory
    shared_memory = shmat(shmid, (void *)0, 0);
    if (shared_memory == (void *)-1) {
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }

    printf("Memory Attached at %p\n", shared_memory);
    shared_stuff = (struct shared_use_st *)shared_memory;

    while (running) {
        // Wait for the previous write to finish
        while (shared_stuff->written_by_you == 1) {
            sleep(1);  // Sleep instead of busy waiting
            printf("Waiting for client...\n");
        }

        // Get user input
        printf("Enter Some Text: ");
        if (fgets(buffer, BUFSIZ, stdin) == NULL) {
            perror("fgets failed");
            continue;  // Handle error but keep running
        }

        // Copy text to shared memory safely
        strncpy(shared_stuff->some_text, buffer, TEXT_SZ);
        shared_stuff->written_by_you = 1;

        // Exit condition
        if (strncmp(buffer, "end", 3) == 0) {
            running = 0;
        }
    }

    // Detach from shared memory
    if (shmdt(shared_memory) == -1) {
        perror("shmdt failed");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}

