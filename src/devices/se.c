#include <devices.h>
#include <se.h>

se_state se;

DEVICE(SE, {
    .address = 0x70012000,
    .size = 0x2000
});