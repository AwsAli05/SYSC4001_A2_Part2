#include <iostream>
#include <csignal>
#include <chrono>
#include <thread>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <unistd.h>

struct SharedData {
    int multiple;     // e.g., 3
    int counter;      // shared counter from P1
    int done;         // 0 = running, 1 = end
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
    // Create shared memory (size for SharedData)
    int shmid = shmget(SHM_KEY, sizeof(SharedData), IPC_CREAT | 0666);
    if (shmid < 0) { perror("shmget"); return 1; }

    // Attach
    auto *data = static_cast<SharedData*>(shmat(shmid, nullptr, 0));
    if (data == (void*)-1) { perror("shmat"); return 1; }

    // Create semaphore set of size 1, init to 1
    int semid = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    if (semid < 0) { perror("semget"); return 1; }
    // Initialize only if newly created: SETVAL is idempotent here for simplicity
    if (semctl(semid, 0, SETVAL, 1) < 0) { perror("semctl SETVAL"); return 1; }

    // Initialize shared data
    sem_lock(semid);
    data->multiple = 3;         // change if you want a different base
    data->counter  = 0;
    data->done     = 0;
    sem_unlock(semid);

    // Exec Process 2
    pid_t pid = fork();
    if (pid < 0) { perror("fork"); return 1; }
    if (pid == 0) {
        // Child replaces itself with proc2
        execl("./proc2_sem", "proc2_sem", (char*)nullptr);
        perror("execl proc2_sem"); // only runs if exec fails
        _exit(127);
    }

    // Process 1 loop
    for (;;) {
        int c, m, done;
        {
            sem_lock(semid);
            if (!data->done) {
                data->counter += 1;
            }
            c = data->counter;
            m = data->multiple;
            done = data->done;
            sem_unlock(semid);
        }
        if (done) break;

        if (c % m == 0) {
            std::cout << "P1 PID " << getpid()
                      << " | Cycle " << c << " – " << c << " is a multiple of " << m << "\n";
        } else {
            std::cout << "P1 PID " << getpid() << " | Cycle " << c << "\n";
        }

        if (c > 500) {
            sem_lock(semid);
            data->done = 1;
            sem_unlock(semid);
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    // Cleanup (mark segments for removal when both detach)
    shmdt((void*)data);
    shmctl(shmid, IPC_RMID, nullptr);
    semctl(semid, 0, IPC_RMID);
    return 0;
}
