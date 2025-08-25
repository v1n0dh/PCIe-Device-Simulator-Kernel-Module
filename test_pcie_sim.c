#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <stdint.h>

#define DEVICE_PATH "/dev/pcie_sim"

// IOCTL commands matching kernel module
#define PCIE_WRITE_REG 0x01
#define PCIE_READ_REG  0x02

#define NUM_THREADS 4

// Thread function to write and read PCIe register
void* thread_func(void* arg) {
    int fd = *(int*)arg;
    uint32_t value = rand() % 256;
    uint32_t read_val;

    // Write value
    if (ioctl(fd, PCIE_WRITE_REG, &value) < 0) {
        perror("IOCTL write failed");
        pthread_exit(NULL);
    }
    printf("Thread %lu: Wrote %u to register 0\n", pthread_self(), value);

    // Read value
    if (ioctl(fd, PCIE_READ_REG, &read_val) < 0) {
        perror("IOCTL read failed");
        pthread_exit(NULL);
    }
    printf("Thread %lu: Read %u from register 0\n", pthread_self(), read_val);

    pthread_exit(NULL);
}

int main() {
    int fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return EXIT_FAILURE;
    }
    printf("Opened device %s\n", DEVICE_PATH);

    // Single-threaded test
    uint32_t value = 42;
    uint32_t read_val;

    printf("=== Single-threaded test ===\n");
    if (ioctl(fd, PCIE_WRITE_REG, &value) < 0) {
        perror("IOCTL write failed");
        close(fd);
        return EXIT_FAILURE;
    }
    printf("Wrote %u to register 0\n", value);

    if (ioctl(fd, PCIE_READ_REG, &read_val) < 0) {
        perror("IOCTL read failed");
        close(fd);
        return EXIT_FAILURE;
    }
    printf("Read %u from register 0\n", read_val);

    // Multi-threaded test
    printf("\n=== Multi-threaded test with %d threads ===\n", NUM_THREADS);
    pthread_t threads[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_create(&threads[i], NULL, thread_func, &fd) != 0) {
            perror("Failed to create thread");
            close(fd);
            return EXIT_FAILURE;
        }
    }

    // Wait for all threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("\nAll threads finished. Check 'dmesg' for simulated interrupts and register logs.\n");

    close(fd);
    return EXIT_SUCCESS;
}
