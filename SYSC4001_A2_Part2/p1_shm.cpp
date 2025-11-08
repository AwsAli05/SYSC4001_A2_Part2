#include <iostream>
#include <thread>
#include <chrono>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

int main() {
    // Shared memory key and size (2 integers)
    key_t key = ftok("shmfile", 65);
    int shmid = shmget(key, 2 * sizeof(int), 0666 | IPC_CREAT);

    if (shmid < 0) {
        perror("shmget");
        return 1;
    }

    int* shared = (int*) shmat(shmid, nullptr, 0);
    if (shared == (void*)-1) {
        perror("shmat");
        return 1;
    }

    // Initialize shared memory
    shared[0] = 10;   // multiple
    shared[1] = 0;   // shared counter

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        execl("./p2_shm", "p2_shm", NULL);
        perror("execl");
        _exit(127);
    }

    // Process 1 loop
    while (shared[1] <= 500) {
        if (shared[1] % shared[0] == 0) {
            std::cout << "[P1] Counter: " << shared[1]
                      << " – Multiple of " << shared[0] << "\n";
        } else {
            std::cout << "[P1] Counter: " << shared[1] << "\n";
        }

        shared[1]++; // increment shared counter
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    // Wait for P2 to exit
    wait(nullptr);

    // Cleanup shared memory
    shmdt(shared);
    shmctl(shmid, IPC_RMID, nullptr);

    std::cout << "[P1] Finished. Shared memory removed.\n";
    return 0;
}
