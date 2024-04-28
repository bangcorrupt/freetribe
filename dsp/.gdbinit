target extended-remote localhost:3333

monitor halt
symbol-file build/bfin.elf

b main 
b module_init
b module_process

continue
