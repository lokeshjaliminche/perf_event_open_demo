#include <stdint.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <inttypes.h>
#include <emmintrin.h> // For _mm_clflush

void setup_perf_event_for_process_helper();
void reset_monitoring_helper();
void start_monitoring_helper();
void stop_monitoring_helper();
long long read_stats_helper();



