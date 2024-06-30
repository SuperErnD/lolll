#include <sys/time.h>
#include <timer.h>
#include <devices.h>

timer_state timer;

void timer_init(uc_engine* uc, void* devptr){
    struct timeval start;
    gettimeofday(&start, NULL);

    long start_time_us = start.tv_sec * 1000000L + start.tv_usec;

    timer.start_time = start_time_us;
}

void timer_callback (uc_engine* uc, uc_mem_type type, uint64_t address, int size, long valuel, void* user_data) {
    device* dev = (device*) user_data;
    (void)dev;
    (void)uc;
    uint64_t reg = address - dev->address;
    (void)size;
    (void)reg;
    if(type == UC_MEM_READ) {
        uint32_t temp;
        if(reg == 0x10) {
            struct timeval start;
            gettimeofday(&start, NULL);

            long time_us = start.tv_sec * 1000000L + start.tv_usec;

            temp = (uint32_t)(time_us - timer.start_time);
            uc_mem_write(uc, address, &temp, sizeof(uint32_t));
        }
    }
}


DEVICE(TIMER, {
    .address = 0x60005000,
    .size = 0x400,
    .init = timer_init,
    .callback = timer_callback
});

