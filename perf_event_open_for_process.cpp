
#include <stdint.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <inttypes.h>
#include <emmintrin.h> // For _mm_clflush
#include <numa.h>
#include <numaif.h>
#include <iostream>
#include <cstring>   // For strerror
#include <errno.h>   // For errno
#include <unistd.h>     // For read
#include <string.h>     // For memset
#include <sys/ioctl.h>  // For ioctl
#include <linux/perf_event.h> // For perf_event_attr, PERF_EVENT_IOC_RESET, etc.
#include <asm/unistd.h> // For syscall numbers like __NR_perf_event_open
#include <linux/hw_breakpoint.h> /* Definition of HW_* constants */
#include <sys/syscall.h>         /* Definition of SYS_* constants */
#include <time.h>
#include <unistd.h>

#include "device_specific_handler.h"

int perf_fd;  // File descriptor for the perf_event


static long perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
                            int cpu, int group_fd, unsigned long flags) {
    int ret = syscall(__NR_perf_event_open, hw_event, pid, cpu, group_fd, flags);
    if (ret == -1) {
        perror("perf_event_open failed");
        printf("errno: %d\n", errno);  // Print error code for debugging
    }
    return ret;
}

void reset_monitoring(int fd) {
    ioctl(fd, PERF_EVENT_IOC_RESET, 0);
}

void reset_monitoring_helper() {
    reset_monitoring(perf_fd);
}

void start_monitoring(int fd) {
    ioctl(fd, PERF_EVENT_IOC_ENABLE, 0);
}

void start_monitoring_helper() {
    start_monitoring(perf_fd);
}

void stop_monitoring(int fd) {
    ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);
}

void stop_monitoring_helper() {
    stop_monitoring(perf_fd);
}


long long read_stats(int fd) {
    long long count;
    read(fd, &count, sizeof(long long));
    return count;
}

long long read_stats_helper() {
    return read_stats(perf_fd);
}


int setup_perf_event_for_process(pid_t pid) {
    struct perf_event_attr pe;
    memset(&pe, 0, sizeof(struct perf_event_attr));

    // Configure the performance event for LLC cache misses
    pe.type = PERF_TYPE_HW_CACHE;
    pe.size = sizeof(struct perf_event_attr);
    pe.config = PERF_COUNT_HW_CACHE_LL |
                (PERF_COUNT_HW_CACHE_OP_READ << 8) |
                (PERF_COUNT_HW_CACHE_RESULT_MISS << 16);  // LLC read misses
    pe.disabled = 1;  // Start disabled
    pe.exclude_kernel = 0;  // Exclude kernel events
    pe.exclude_hv = 1;      // Exclude hypervisor events
    pe.inherit = 1;  // Inherit counters across threads and subprocesses

    // Open the performance event for the specified process and all threads
    int fd = perf_event_open(&pe, pid, -1, -1, 0);
    if (fd == -1) {
        perror("perf_event_open failed");
        printf("errno: %d\n", errno);
    }
    return fd;
}


void setup_perf_event_for_process_helper() {
    pid_t pid = getpid();
    //pid_t ppid = getppid();// if you want to monitor parent process
    perf_fd = setup_perf_event_for_process(pid);  // Set up the global perf_fd
}
