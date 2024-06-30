#include <stdio.h>
#include <log.h>
#include <devices.h>
#include <unicorn/unicorn.h>
#include <fcntl.h>
#include <getopt.h>
#include <soc.h>

uc_engine* uc;

bool init_unicorn() {
    uc_err err;

    log_trace("Setupping unicorn instance (a emu for tegra boot processor)");

    err = uc_open(UC_ARCH_ARM, UC_MODE_ARM, &uc);

    if (err) {
        log_fatal("Failed on uc_open() with error returned: %u (%s)", err,
               uc_strerror(err));
        return false;
    }

    return true;
}

int mem_read_unmapped(uc_engine* eng, uc_mem_type type, uint64_t address, int size, long value, void* user_data) {
    (void)eng;
    (void)size;
    (void)value;
    (void)user_data;
    log_error("detected %s on unmapped 0x%lx-0x%lx; value is 0x%lx", type == UC_MEM_READ_UNMAPPED ? "read" : "write", address, address + size, value);
    return 1;
}

void mem_info(uc_engine* uc, uc_mem_type type, uint64_t address, int size, long value, void* user_data){
    (void)uc;
    (void)size;
    (void)value;
    (void)user_data;
    (void)type;
    if(type == UC_MEM_READ)
        uc_mem_read(uc, address, &value, size);
    uint32_t r_pc;
    uc_reg_read(uc, UC_ARM_REG_PC, &r_pc);
    if(address >= 0x40040000)
        log_info("detected %s at 0x%lx-0x%lx with value=0x%lx at pc=0x%x", type == UC_MEM_READ ? "read" : "write", address, address+size, value, r_pc);
}

void add_hooks() {
    uc_hook mem_unmapped_read_hook;
    uc_err err = uc_hook_add(uc, &mem_unmapped_read_hook,
                UC_HOOK_MEM_UNMAPPED, mem_read_unmapped, NULL, 1, 0);
    if (err){
        log_fatal("unmapped read&write hook add failed with error %u %s!", err, uc_strerror(err));
    }

        uc_hook mem_info_hook;
        err = uc_hook_add(uc, &mem_info_hook,
                    UC_HOOK_MEM_READ | UC_HOOK_MEM_WRITE, mem_info, NULL, 1, 0);
        if (err){
            printf("mem read&write hook add failed with error %u %s!\n", err, uc_strerror(err));
        }
}

void load_file(char* filename, uint64_t address, uint64_t size) {
    char *data = (char*)malloc(size);
    int fd;
    if ((fd = open(filename, O_RDONLY)) < 0)
        log_fatal("Failed to open file.");
    if (read(fd, data, size) < 1)
        log_fatal("Failed to read file.");
    close(fd);
    uc_err err = uc_mem_write(uc, address, data, size);
    if (err)
        log_fatal("Failed write memory with error returned %u: %s", err, uc_strerror(err));
    log_trace("memory writed!");
    free(data);
}

void map_mem(uint64_t address, uint64_t size){
    uc_err err = uc_mem_map(uc, address, size, UC_PROT_ALL);
    if (err)
        log_fatal("Failed map memory at 0x%lx, size 0x%lx with error returned %u: %s\n", address, size, err, uc_strerror(err));
}

void emu_start(uint32_t start){
    //signal(SIGINT, emu_exit);
    uc_err err = uc_emu_start(uc, start, 0, 0, 0);
    if (err) {
        log_error("Failed on uc_emu_start() with error returned %u: %s",
        err, uc_strerror(err));
    }

    uint32_t r_pc;
    uc_reg_read(uc, UC_ARM_REG_PC, &r_pc);
    log_info("PC: 0x%x", r_pc);
}

unsigned int round_to_nearest_1024(unsigned int num) {
    return (num + 1023) & ~1023;
}

void devices_probe() {
    devices_list* devices = (devices_list*)get_devices();
    for(;;){
        device* dev = devices->this;
        log_trace("Adding device %s...", dev->name);
        map_mem(dev->address, round_to_nearest_1024(dev->size));
        if(dev->callback){
            log_trace("Adding device callback...");
            uc_hook* hook = malloc(sizeof(uc_hook));
            uc_err err = uc_hook_add(uc, hook, UC_HOOK_MEM_READ | UC_HOOK_MEM_WRITE, dev->callback, dev, dev->address,
                    dev->address + dev->size - 1);
            if (err) {
                log_fatal("Adding device hook failed with error %u!", err);
            }
        }
        if(dev->init)
            dev->init(uc, (void*)dev);
        devices = devices->next;
        if(!devices)
            break;
    }
}

int main(int argc, char *argv[]) {
    log_trace("Hello world!");

    char* payload = NULL;

    // parse arguments
    int opt;

    while ((opt = getopt(argc, argv, "p:")) != -1) {
        switch (opt) {
        case 'p': payload = optarg; break;
        default:
            fprintf(stderr, "Usage: %s [-p] [payload...]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if(!payload) {
        log_fatal("No payload provided.");
        return 1;
    }

    if(!init_unicorn()) {
        return 1;
    }

    add_hooks();

    map_mem(IRAM_START, IRAM_LEN);
    map_mem(EXCP_START, EXCP_LEN);
    map_mem(TZ_START, TZ_LEN);

    load_file(payload, PAYLOAD_ADDR, PAYLOAD_LEN);

    devices_probe();

    log_trace("Running a emulation!");

    emu_start(PAYLOAD_ADDR);

    return 0;
}