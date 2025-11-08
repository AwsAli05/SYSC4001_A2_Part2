#include <iostream>
#include <thread>
#include <chrono>

int main() {
    long long counter = 0;
    long long cycle = 0;

    for (;;) {
        if (counter % 3 == 0) {
            std::cout << "Cycle number: " << cycle << " – " << counter
                      << " is a multiple of 3\n";
        } else {
            std::cout << "Cycle number: " << cycle << "\n";
        }
        counter--;   // decrement here (important)
        cycle++;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}
