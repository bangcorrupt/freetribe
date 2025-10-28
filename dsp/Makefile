# Thanks to Job Vranish (https://spin.atomicobject.com/2016/08/26/makefile-c-projects/)

MODULE ?= default 

BFIN_TOOLCHAIN ?= /opt/bfin-elf/bin/

BFIN_LIB := /opt/bfin-elf/bfin-elf/include/
INC_DIRS := $(BFIN_LIB)

TARGET_EXEC := bfin.elf
TARGET_BIN := bfin.ldr

BUILD_DIR := ./build
SRC_DIR := ./src
LIB_DIR := ./lib
MODULE_DIR ?= $(SRC_DIR)/modules/$(MODULE)

# Defining the cross compiler tool prefix
PREFIX := bfin-elf-

# Compiler, Linker and Archiver with respect to the toolchain
CC := $(BFIN_TOOLCHAIN)${PREFIX}gcc
CXX := $(BFIN_TOOLCHAIN)${PREFIX}gcc
LDR := $(BFIN_TOOLCHAIN)$(PREFIX)ldr
LD := $(BFIN_TOOLCHAIN)${PREFIX}ld
BIN := $(BFIN_TOOLCHAIN)$(PREFIX)objcopy

CPU := bf527

# C compilation options
# OPTIMISE = -g -O3
OPTIMISE = -Os
CFLAGS := -mcpu=$(CPU) $(OPTIMISE) -mfast-fp  -fdata-sections -ffunction-sections -Wall 
# Assembly compilation options
ASM_FLAGS := -x assembler-with-cpp

# Passed to gcc, not ld.
LDFLAGS := $(LPATH)  -lm -lbfdsp -lbffastfp \
				-T dsp.lds -Xlinker -Map -Xlinker $(BUILD_DIR)/boot.map \
				-Wl,--gc-sections 

LDRFLAGS := --bits 8 --bmode spi_slave --verbose


# Find all the C, C++ and assembly files we want to compile.
# Note the single quotes around the * expressions. The shell will incorrectly expand these otherwise, but we want to send the * directly to the find command.
SRCS := $(shell find $(SRC_DIR)  \( -name '*.cpp' -or -name '*.c' -or -name '*.S' \) \
		-not -path "$(SRC_DIR)/modules/*")

# Add libs, excluding libfixmath root directory.
SRCS += $(shell find $(LIB_DIR)  \( -name '*.cpp' -or -name '*.c' -or -name '*.S' \) \
		-not -path "$(LIB_DIR)/aleph-dsp/lib/libfixmath/*" \
		-not -path "$(LIB_DIR)/aleph-dsp/test/*")

# Add libfixmath src directory.
SRCS += $(shell find $(LIB_DIR)/aleph-dsp/lib/libfixmath/libfixmath  -name '*.cpp' -or \
		-name '*.c' -or -name '*.S')

SRCS += $(shell find $(MODULE_DIR)  -name '*.cpp' -or -name '*.c' -or -name '*.S')

# Prepends BUILD_DIR and appends .o to every src file.
# As an example, ./your_dir/hello.cpp turns into: ./build/./your_dir/hello.cpp.o 
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

# String substitution (suffix version without %).
# As an example, ./build/hello.cpp.o turns into: ./build/hello.cpp.d
DEPS := $(OBJS:.o=.d)

# Every folder in ./src will need to be passed to GCC so that it can find header files.
INC_DIRS += $(shell find $(SRC_DIR) -type d -not -path "$(SRC_DIR)/modules/*" )

# Add libs, excluding libfixmath root directory.
INC_DIRS += $(shell find $(LIB_DIR) -type d -not -path "$(LIB_DIR)/aleph-dsp/lib/libfixmath/*" )

# Add libfixmath src directory.
INC_DIRS += $(shell find $(LIB_DIR)/aleph-dsp/lib/libfixmath/libfixmath -type d)

INC_DIRS += $(shell find $(MODULE_DIR) -type d)

# Add a prefix to INC_DIRS. So moduleA would become -ImoduleA. GCC understands this -I flag.
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

# The -MMD and -MP flags together generate Makefiles for us.
# These files will have .d instead of .o as the output.
CPPFLAGS := $(INC_FLAGS) -MMD -MP -D ARCH_BFIN=1 -D __ADSPBF523__


# Convert ELF to LDR.
$(BUILD_DIR)/$(TARGET_BIN): $(BUILD_DIR)/$(TARGET_EXEC)
	$(LDR) -T $(CPU) -c $(LDRFLAGS) $(BUILD_DIR)/$(TARGET_BIN) $(BUILD_DIR)/$(TARGET_EXEC)

# The final build step.
$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

# Build step ASM source.
$(BUILD_DIR)/%.s.o: %.s
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

# Include the .d makefiles. The - at the front suppresses the errors of missing
# Makefiles. Initially, all the .d files will be missing, and we don't want those
# errors to show up.
-include $(DEPS) 
