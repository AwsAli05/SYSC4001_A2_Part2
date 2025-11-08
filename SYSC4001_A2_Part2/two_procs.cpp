#include <iostream>
#include <unistd.h>   // fork, pipe, read, write, getpid
#include <cstring>    // strerror
#include <chrono>
#include <thread>

int main() {
    // Two unnamed pipes:
    // p2c: parent -> child
    // c2p: child  -> parent
    int p2c[2], c2p[2];
    if (pipe(p2c) == -1 || pipe(c2p) == -1) {
        std::perror("pipe");
        return 1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        std::perror("fork");
        return 1;
    }

    // Make output flush each line
    std::cout.setf(std::ios::unitbuf);

    if (pid == 0) {
        // --- Child: P2 ---
        // Close unused ends
        close(p2c[1]); // child reads from p2c[0]
        close(c2p[0]); // child writes to c2p[1]

        long counter2 = 0;
        char token;
        for (;;) {
            // Wait for parent token (blocks until parent writes)
            if (read(p2c[0], &token, 1) != 1) break;

            std::cout << "Process 2 (PID " << getpid() << "): " << counter2++ << "\n";

            // Optional small delay just to slow display (not for ordering)
            std::this_thread::sleep_for(std::chrono::milliseconds(200));

            // Signal parent it's their turn
            if (write(c2p[1], "x", 1) != 1) break;
        }
        // Clean up
        close(p2c[0]);
        close(c2p[1]);
        return 0;
    } else {
        // --- Parent: P1 ---
        // Close unused ends
        close(p2c[0]); // parent writes to p2c[1]
        close(c2p[1]); // parent reads from c2p[0]

        long counter1 = 0;
        // Kickstart child by giving first token
        if (write(p2c[1], "x", 1) != 1) {
            std::perror("write");
            return 1;
        }

        char token;
        for (;;) {
            // Parent waits for child to finish its print
            if (read(c2p[0], &token, 1) != 1) break;

            std::cout << "Process 1 (PID " << getpid() << "): " << counter1++ << "\n";

            // Optional small delay to slow display
            std::this_thread::sleep_for(std::chrono::milliseconds(200));

            // Give turn back to child
            if (write(p2c[1], "x", 1) != 1) break;
        }
        // Clean up
        close(p2c[1]);
        close(c2p[0]);
        return 0;
    }
}
