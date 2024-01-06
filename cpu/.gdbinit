add-auto-load-safe-path ./.gdbinit
set style enable on
set logging on
targ ext localhost:2331
set remote hardware-breakpoint-limit 2
set remote hardware-watchpoint-limit 2
restore build/cpu.bin binary 0x80000000
file build/cpu.elf
set $pc=0x80000000
b main
c

