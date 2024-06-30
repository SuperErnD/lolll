#include <devices.h>
#include <unicorn/unicorn.h>
#include <pmc.h>
#include <se.h>

extern se_state se;
pmc_state pmc;

void pmc_callback (uc_engine* uc, uc_mem_type type, uint64_t address, int size, long valuel, void* user_data) {
    device* dev = (device*) user_data;
    (void)dev;
    (void)uc;
    uint64_t reg = address - dev->address;
    (void)size;
    (void)reg;
    if(reg == 0xf4 && type == UC_MEM_WRITE) {
        if(valuel == 0) {
            // поднять сало
            se.enabled = true;
        }

        if(valuel == 1) {
            // опустить сало
            se.enabled = false;
        }
    }
}

DEVICE(PMC, {
    .address = 0x7000e400,
    .size = 0xc00,
    .callback = pmc_callback
});