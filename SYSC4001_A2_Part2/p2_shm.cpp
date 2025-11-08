#include <iostream>
#include <thread>
#include <chrono>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int main() {
    key_t key = ftok("shmfile", 65);
    int shmid = shmget(key, 2 * sizeof(int), 0666);

    if (shmid < 0) {
        perror("shmget");
        return 1;
    }

    int* shared = (int*) shmat(shmid, nullptr, 0);
    if (shared == (void*)-1) {
        perror("shmat");
        return 1;
    }

    int multiple = shared[0];

    // Wait until P1 grows shared counter to >100
    while (shared[1] <= 100) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    // Now participate actively
    while (shared[1] <= 500) {
        if (shared[1] % multiple == 0) {
            std::cout << "[P2] Counter: " << shared[1]
                      << " – Multiple of " << multiple << "\n";
        } else {
            std::cout << "[P2] Counter: " << shared[1] << "\n";
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    shmdt(shared);
    std::cout << "[P2] Exiting.\n";
    return 0;
}
