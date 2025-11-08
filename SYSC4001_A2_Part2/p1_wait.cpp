#include <iostream>
#include <thread>
#include <chrono>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    setvbuf(stdout, nullptr, _IOLBF, 0);

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        execl("./p2_wait", "p2_wait", (char*)nullptr);
        perror("execl");
        _exit(127);
    }

    // Parent process (P1)
    long long counter = 0;
    long long cycle = 0;

    std::cerr << "[P1] PID=" << getpid() << " waiting for P2 (PID=" << pid << ")\n";

    while (true) {
        // Check if P2 has finished yet
        int status;
        pid_t result = waitpid(pid, &status, WNOHANG);
        if (result == pid) {  // P2 is done
            std::cerr << "[P1] Detected P2 exit. Ending P1.\n";
            break;
        }

        // Normal display logic
        if (counter % 3 == 0)
            std::cout << "Cycle number: " << cycle << " – " << counter << " is a multiple of 3\n";
        else
            std::cout << "Cycle number: " << cycle << "\n";

        counter++;
        cycle++;
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    return 0;
}
