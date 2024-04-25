add-auto-load-safe-path ./.gdbinit
set style enable on
set logging on
set pagination off
targ ext localhost:2331
set remote hardware-breakpoint-limit 2
set remote hardware-watchpoint-limit 2
load build/cpu.elf
file build/cpu.elf
set $pc=0x80000000
# b start_boot
# b knl_init
# b app_init
# b svc_display_task
c

