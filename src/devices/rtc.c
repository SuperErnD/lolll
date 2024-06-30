#include <time.h>
#include <devices.h>

long freq = 32000; // by default its 32 khz

long get_time_in_milliseconds() {
    static struct timespec start;
    static long milliseconds = 0;
    static long error = 0;
    static int initialized = 0;
    long interval = 1000000000 / freq; // interval in nanoseconds

    if (!initialized) {
        clock_gettime(1, &start);
        initialized = 1;
    }

    struct timespec current;
    clock_gettime(1, &current);
    long elapsed = (current.tv_sec - start.tv_sec) * 1000000000L + (current.tv_nsec - start.tv_nsec);

    if (elapsed >= interval) {
        milliseconds++;
        start = current;
        error += elapsed - interval;
        while (error >= interval) {
            milliseconds++;
            error -= interval;
        }
    }

    return milliseconds;
}

void rtc_callback (uc_engine* uc, uc_mem_type type, uint64_t address, int size, long valuel, void* user_data) {
    device* dev = (device*) user_data;
    (void)dev;
    (void)uc;
    uint64_t reg = address - dev->address;
    (void)size;
    (void)reg;
    if(type == UC_MEM_READ) {
        uint32_t temp;
        if(reg == 0x10) {

            temp = (uint32_t)get_time_in_milliseconds();
            uc_mem_write(uc, address, &temp, sizeof(uint32_t));
        }
    }
}

DEVICE(RTC, {
    .address = 0x7000e000,
    .size = 0x400,
    .callback = rtc_callback
});