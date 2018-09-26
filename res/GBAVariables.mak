# If you're running a different distro you'll likely have
# to change the GCC_VERSION variable. It can be found by running
# arm-none-eabi-gcc --version
GCC_VERSION = $(shell arm-none-eabi-gcc --version | grep -o '[0-9]*\.[0-9]*\.[0-9]*' | head -n1)

CROSS   := arm-none-eabi-
AS      := $(CROSS)as
CC      := $(CROSS)gcc
LD      := $(CROSS)ld
OBJCOPY := $(CROSS)objcopy

OBJECTS += res/crt0.o res/libc_sbrk.o

ARMINC = /usr/arm-none-eabi/include
ARMLIB = /usr/arm-none-eabi/lib
GCCLIB = /usr/lib/gcc/arm-none-eabi/$(GCC_VERSION)

CFLAGS   += -Wall -Werror -std=gnu11 -pedantic -Wextra

MODEL    = -mthumb-interwork
CFLAGS   += $(MODEL) -marm -mlong-calls -MMD -MP -I $(ARMINC)
LINKFLAGS = -nostartfiles -L $(ARMLIB) \
	  -L $(ARMLIB) \
	  -L $(GCCLIB) \
	  -T res/arm-gba.ld

