# Thanks to Job Vranish (https://spin.atomicobject.com/2016/08/26/makefile-c-projects/)

APP ?= demo

TARGET_EXEC := cpu.elf
TARGET_BIN := cpu.bin

# Library paths.
LIB_C ?= /usr/arm-none-eabi/lib
LIB_GCC ?= /lib/gcc/arm-none-eabi/14.2.0/

LPATH := -L"${LIB_C}" \
      	 -L"${LIB_GCC}"

NEWLIB ?= /usr/arm-none-eabi/include/
INC_DIRS := $(NEWLIB)

BUILD_DIR := ./build
SRC_DIR := ./src
LIB_DIR := ./lib
APP_DIR ?= $(SRC_DIR)/apps/$(APP)

# Defining the cross compiler tool prefix.
PREFIX := arm-none-eabi-

# Compiler, Linker and Archiver with respect to the toolchain.
CC=${PREFIX}gcc
CXX=${PREFIX}gcc
LD=${PREFIX}ld
BIN=$(PREFIX)objcopy

# C compilation options.
NEST_INT ?= 0
OPTIMISE ?= -g3 -Og
# OPTIMISE ?= -O3
CPU ?= arm926ej-s
CFLAGS := -mcpu=$(CPU) $(OPTIMISE) -fdata-sections -ffunction-sections -fanalyzer -fstack-usage -Wstack-usage=128 -Wall 

# Assembly compilation options.
ASM_FLAGS := -x assembler-with-cpp

# Linker options.
LDFLAGS := -e start -u start -u __aeabi_uidiv -u __aeabi_idiv $(LPATH) -lm -lc -lgcc  \
					 -T cpu.lds -Xlinker -Map -Xlinker $(BUILD_DIR)/boot.map -specs=picolibc.specs \
					 -Wl,--gc-sections,--print-memory-usage,--no-warn-rwx-segments 
# Binary options.
BIN_FLAGS := -O binary


# Find all the C, C++ and assembly files we want to compile.
# Note the single quotes around the * expressions.
# The shell will incorrectly expand these otherwise,
# but we want to send the * directly to the find command.
SRCS := $(shell find $(SRC_DIR)  \( -name '*.cpp' -or -name '*.c' -or -name '*.S' \) \
		-not -path "$(SRC_DIR)/apps/*")


SRCS += $(shell find $(LIB_DIR)  \( -name '*.cpp' -or -name '*.c' -or -name '*.S' \) \
		-not -path "$(LIB_DIR)/LEAF/*")

SRCS += $(shell find $(LIB_DIR)/LEAF/leaf/  -name '*.cpp' -or -name '*.c' -or -name '*.S')

SRCS += $(shell find $(APP_DIR)  -name '*.cpp' -or -name '*.c' -or -name '*.S')

# Prepends BUILD_DIR and appends .o to every src file.
# As an example, ./your_dir/hello.cpp turns into: ./build/./your_dir/hello.cpp.o 
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

# String substitution (suffix version without %).
# As an example, ./build/hello.cpp.o turns into: ./build/hello.cpp.d
DEPS := $(OBJS:.o=.d)

# Every folder in ./src will need to be passed to GCC so that it can find header files.
INC_DIRS += $(shell find $(SRC_DIR) -type d -not -path "$(SRC_DIR)/apps/*" )

INC_DIRS += $(shell find $(LIB_DIR) -type d -not -path "$(SRC_DIR)/LEAF/*" )


INC_DIRS += $(shell find $(LIB_DIR)/LEAF/leaf/ -type d)

INC_DIRS += $(shell find $(APP_DIR) -type d)

# Add a prefix to INC_DIRS. So moduleA would become -ImoduleA. GCC understands this -I flag.
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

# The -MMD and -MP flags together generate Makefiles for us.
# These files will have .d instead of .o as the output.
CPPFLAGS := $(INC_FLAGS) -MMD -MP -DNESTED_INTERRUPTS=${NEST_INT}

# TODO: AISGen.

# Extract binary from ELF.
$(BUILD_DIR)/$(TARGET_BIN): $(BUILD_DIR)/$(TARGET_EXEC)
	$(BIN) $(BIN_FLAGS) $(BUILD_DIR)/$(TARGET_EXEC) $(BUILD_DIR)/$(TARGET_BIN)

# Link.
$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	# $(LD) $(OBJS) -o $@ $(LDFLAGS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

# Build step ASM source.
$(BUILD_DIR)/%.S.o: %.S
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(ASM_FLAGS) -c $< -o $@

# Build step for C source.
$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# Build step for C++ source.
$(BUILD_DIR)/%.cpp.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@


.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

# Include the .d makefiles. The - at the front suppresses
# the errors of missing Makefiles. 
# Initially, all the .d files will be missing, 
# and we don't want those errors to show up.
-include $(DEPS) 
