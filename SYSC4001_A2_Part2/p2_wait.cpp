#include <iostream>
#include <thread>
#include <chrono>
#include <unistd.h>

int main() {
    setvbuf(stdout, nullptr, _IOLBF, 0);

    long long counter = 0;
    long long cycle = 0;

    std::cerr << "[P2] PID=" << getpid() << " started.\n";

    while (true) {
        if (counter % 3 == 0)
            std::cout << "Cycle number: " << cycle << " – " << counter << " is a multiple of 3\n";
        else
            std::cout << "Cycle number: " << cycle << "\n";

        counter--;        // decrement
        cycle++;
        std::this_thread::sleep_for(std::chrono::milliseconds(250));

        if (counter < -500) {
            std::cerr << "[P2] reached " << counter << ". Exiting now.\n";
            return 0;     // ← this exit is what P1 waits for
        }
    }
}
