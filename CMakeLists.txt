cmake_minimum_required(VERSION 3.26)
project(lolnxe)

include(cmake/CPM.cmake)

CPMAddPackage("gh:unicorn-engine/unicorn#dev")

include_directories(include)

add_executable(lolnxe
    src/main.c
    src/log.c
    src/devices.c
    src/devices/clock.c
    src/devices/apb_misc.c
    src/devices/fuse.c
    src/devices/se.c
    src/devices/pmc.c
    src/devices/sysreg.c
    src/devices/timer.c
    src/devices/mc.c
    src/devices/sysctr0.c
    src/devices/pinmux.c
    src/devices/gpio.c
    src/devices/i2c.c
    src/devices/rtc.c
    src/devices/emc.c
)

target_link_libraries(lolnxe unicorn)