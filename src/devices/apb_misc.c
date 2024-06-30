#include <devices.h>
#include <unicorn/unicorn.h>
#include <log.h>

void apb_init(uc_engine* uc, void* devptr){
    device* dev = (device*)devptr;
    uint8_t* apb = malloc(dev->size);
    *(uint32_t*)(apb + 0x804) = (10 & 0xF) << 4; // its a mariko so we should set a 0x2 (to be >= 0x2)
    //fuse[0x130] = 0x40;
    uc_mem_write(uc, dev->address, apb, dev->size);
    free(apb);
    log_debug("apb writed!");
}

DEVICE(APB_MISC, {
    .address = 0x70000000,
    .size = 0x1000,
    .init = apb_init
});