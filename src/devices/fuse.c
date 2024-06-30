#include <devices.h>
#include <unicorn/unicorn.h>
#include <log.h>

void fuse_init(uc_engine* uc, void* devptr){
    device* dev = (device*)devptr;
    uint8_t* fuse = malloc(dev->size);
    *(uint32_t*)(fuse + (0x1C8 + 4 * 4)) = (4 & 0xF) << 16;
    *(uint32_t*)(fuse + 0x1A4) = 0xffffffff;
    *(uint32_t*)(fuse + 0x1A8) = 0xffffffff;
    *(uint32_t*)(fuse + 0x1AC) = 0xffffffff;
    *(uint32_t*)(fuse + 0x1B0) = 0xffffffff;
    *(uint32_t*)(fuse + 0x1B4) = 0xffffffff;
    //fuse[0x130] = 0x40;
    uc_mem_write(uc, dev->address, fuse, dev->size);
    free(fuse);
    log_debug("fuse writed!");
}

DEVICE(FUSE, {
    .address = 0x7000f000,
    .size = 2520, // if trust the trm
    .init = fuse_init
});
