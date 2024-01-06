target extended-remote localhost:3333

monitor halt
symbol-file build/bfin.elf

b module_process

continue
