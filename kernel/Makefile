# Kernel Makefile
#
# C and .S (assembly) files are handled.

# Name of the kernel executable
KERNEL = dekernel

# What objects are linked to make the kernel
# multiboot.o should come first here, because GRUB may not find the
# multiboot header otherwise
KERNEL_OBJECTS = main/multiboot.o main/kmain.o core/memory.o core/interrupt.o core/isr.o lib/video.o lib/stubs.o lib/abscopy.o process/module.o process/process.o process/schedule.o process/contextswitch.o process/ksyscall.o process/message.o

# What gets done when you just type "make"
all: $(KERNEL)

# Cleanup
clean:
	rm -f $(KERNEL) main/*.o main/*.d core/*.o core/*.d lib/*.o lib/*.d process/*.o process/*.d

CC = i586-elf-gcc
LD = i586-elf-gcc

CS452DIR=/u3/cs452/i586-3.3.3

# Common for both kernel and modules
CFLAGS = -std=c99 -nostdinc -nostdinc++ -nodefaultlibs -nostdlib -Wall -Iinclude -I$(CS452DIR)/include -I$(CS452DIR)/i586-elf/include -I$(CS452DIR)/include/cs452 -I$(CS452DIR)/lib/gcc-lib/i586-elf/3.3.3/include/
LDFLAGS = -L$(CS452DIR)/lib -Wl,-T,loader.kernel.x
LDADD = -lstdc++ -lgcc -lc 

# Compilation
%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Linking
$(KERNEL): $(KERNEL_OBJECTS) loader.kernel.x
	$(LD) $(filter %.o,$^) $(CFLAGS) $(LDFLAGS) $(LDADD)  -o $@

# Dependency handling
%.d: %.c
	$(CC) -M $(CFLAGS) $< > $@.$$$$; \
              sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
              rm -f $@.$$$$

%.d: %.S
	$(CC) -M $(CFLAGS) $< > $@.$$$$; \
              sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
              rm -f $@.$$$$

CSOURCES = $(wildcard *.c)
ASMSOURCES = $(wildcard *.S)
DEPENDS = $(CSOURCES:.c=.d) $(ASMSOURCES:.S=.d)

include $(DEPENDS)
