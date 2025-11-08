#include <iostream>
#include <chrono>
#include <thread>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <unistd.h>

struct SharedData {
    int multiple;
    int counter;
    int done;
};

static const key_t SHM_KEY = 0x1234;
static const key_t SEM_KEY = 0x5678;

static void sem_lock(int semid) {
    sembuf op{0, -1, 0};
    semop(semid, &op, 1);
}
static void sem_unlock(int semid) {
    sembuf op{0, +1, 0};
    semop(semid, &op, 1);
}

int main() {
    // Attach to existing IPC objects (created by proc1)
    int shmid = -1, semid = -1;

    // Retry until proc1 creates them (simple, robust)
    for (;;) {
        semid = semget(SEM_KEY, 1, 0666);
        shmid = shmget(SHM_KEY, sizeof(SharedData), 0666);
        if (semid >= 0 && shmid >= 0) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    auto *data = static_cast<SharedData*>(shmat(shmid, nullptr, 0));
    if (data == (void*)-1) { perror("shmat"); return 1; }

    std::cerr << "[P2] PID=" << getpid() << " attached (sem/shm ready)\n";

    // Wait until shared counter > 100
    for (;;) {
        int c, done;
        sem_lock(semid);
        c = data->counter;
        done = data->done;
        sem_unlock(semid);
        if (done) break;
        if (c > 100) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    // Now react to the shared counter and multiple; end when done==1
    for (;;) {
        int c, m, done;
        sem_lock(semid);
        c = data->counter;   // read-only for P2
        m = data->multiple;
        done = data->done;
        sem_unlock(semid);

        if (done) break;

        if (c % m == 0) {
            std::cout << "P2 PID " << getpid()
                      << " | Counter " << c << " – " << c << " is a multiple of " << m << "\n";
        } else {
            std::cout << "P2 PID " << getpid() << " | Counter " << c << "\n";
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }

    shmdt((void*)data);
    return 0;
}
