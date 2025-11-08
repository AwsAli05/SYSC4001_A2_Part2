// proc1.cpp
#include <iostream>
#include <thread>
#include <chrono>
#include <unistd.h>     // fork, execl
#include <cstdlib>      // exit
#include <sys/types.h>

int main() {
    // Fork once
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        // Child: replace with proc2
        execl("./proc2", "proc2", (char*)nullptr);
        // If execl returns, it failed
        perror("execl");
        _exit(127);
    }

    // Parent: run Process 1 (incrementing cycles)
    // We display *cycle number* exactly like the example (no negatives, no prefixes)
    long long cycle = 0;
    for (;;) {
        if (cycle % 3 == 0) {
            std::cout << "Cycle number: " << cycle << " – " << cycle
                      << " is a multiple of 3\n";
        } else {
            std::cout << "Cycle number: " << cycle << "\n";
        }
        ++cycle;
        std::this_thread::sleep_for(std::chrono::milliseconds(400));
    }
}
