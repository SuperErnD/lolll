from unicorn import *
from unicorn.arm_const import *
from capstone import *
from capstone.arm_const import *

iram_start = 0x40000000
iram_entry = 0x40008000
iram_end   = 0x40040000
clock_start= 0x60006000
clock_end  = 0x60006000 + 1024 * 2
excpvec_str= 0x6000F000
excpvec_end= 0x6000F000 + 1024
apbmisc_str= 0x70000000
apbmisc_end= 0x70000000 + 1024

def hook_mem_access(uc, access, address, size, value, user_data):
    if access == UC_MEM_WRITE:
        print(">>> Memory is being WRITE at 0x%x, data size = %u, data value = 0x%x" \
                %(address, size, value))
    else:   # READ
        print(">>> Memory is being READ at 0x%x, data size = %u" \
                %(address, size))
    
    return True

def hook_mem_invalid(uc, access, address, size, value, user_data):
    if access == UC_MEM_WRITE_UNMAPPED:
        print(">>> Missing memory is being WRITE at 0x%x, data size = %u, data value = 0x%x" \
                %(address, size, value))
        return True
    elif access == UC_MEM_READ_UNMAPPED:
        print(">>> Missing memory is being READ at 0x%x, data size = %u" \
                %(address, size))
        return True
    else:
        # return False to indicate we want to stop emulation
        return False

def hook_code(uc: Uc, address, size, user_data):
    for i in md.disasm(uc.mem_read(address, size), address):
        print("0x%x:\t%s\t%s" %(i.address, i.mnemonic, i.op_str))
    
    #print(">>> Tracing instruction at 0x%x, instruction size = 0x%x" %(address, size))

print("starting emulation lol")

payload = b''

with open("payload.bin", 'rb') as f:
    payload = f.read()

mu = Uc(UC_ARCH_ARM, UC_MODE_ARM)
print(iram_end - iram_start)
print((iram_end - iram_start) % 1024)
mu.mem_map(iram_start, (iram_end - iram_start))
mu.mem_map(clock_start, (clock_end - clock_start))
mu.mem_map(excpvec_str, (excpvec_end - excpvec_str))
mu.mem_map(apbmisc_str, (apbmisc_end - apbmisc_str))

md = Cs(CS_ARCH_ARM, CS_MODE_ARM)

mu.hook_add(UC_HOOK_CODE, hook_code, begin=0, end=0xffffffff)

mu.hook_add(UC_HOOK_MEM_WRITE, hook_mem_access)
mu.hook_add(UC_HOOK_MEM_READ, hook_mem_access)
mu.hook_add(UC_HOOK_MEM_READ_UNMAPPED | UC_HOOK_MEM_WRITE_UNMAPPED, hook_mem_invalid)

mu.mem_write(iram_entry, payload)

try:
    mu.emu_start(iram_entry, 0)
finally:
    print("Uh-oh!")
    print("Dumping registers..\n R0 = 0x%x R1 = 0x%x R2 = 0x%x R3 = 0x%x\n R4 = 0x%x R5 = 0x%x R6 = 0x%x R7 = 0x%x \n R8 = 0x%x R9 = 0x%x R10 = 0x%x R11 = 0x%x\n R12 = 0x%x SP = 0x%x LR = 0x%x PC=0x%x" % (
        mu.reg_read(UC_ARM_REG_R0), mu.reg_read(UC_ARM_REG_R1), mu.reg_read(UC_ARM_REG_R2), mu.reg_read(UC_ARM_REG_R3),
        mu.reg_read(UC_ARM_REG_R4), mu.reg_read(UC_ARM_REG_R5), mu.reg_read(UC_ARM_REG_R6), mu.reg_read(UC_ARM_REG_R7),
        mu.reg_read(UC_ARM_REG_R8), mu.reg_read(UC_ARM_REG_R9), mu.reg_read(UC_ARM_REG_R10), mu.reg_read(UC_ARM_REG_R11),
        mu.reg_read(UC_ARM_REG_R12), mu.reg_read(UC_ARM_REG_R13), mu.reg_read(UC_ARM_REG_R14), mu.reg_read(UC_ARM_REG_R15),
    ))
