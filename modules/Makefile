# Modules Makefile
#
# C and .S (assembly) files are handled.

# Add any new modules you make here
MODULES = init idle
MODULES += server/name \
           server/time server/time_notifier server/time_courier \
           server/serial server/serial_notifier server/serial_courier \
           server/serial_buffer_in server/serial_buffer_out \
           train/train \
           train/sensors train/sensors_courier train/sensors_keepalive \
           train/track train/track_courier \
           train/engineer train/engineer_coordinator train/engineer_coordinator_courier \
           train/track_display train/train_display train/time_writer \
           train/heartbeat \
           train/train_destination_generator
MODULES += test/test_srr test/test_srr_worker test/test_ns test/test_ns_child  \
           test/test_time test/test_serial \
           test/test_ee test/test_eater test/test_ender

# Objects that are linked in with all modules
COMMON_MODULE_OBJECTS = lib/crt0.o lib/usyscall.o lib/usyscall_wrapper.o lib/userstubs.o lib/terminal.o lib/trainlib.o

# What gets done when you just type "make"
all: $(MODULES)

# Cleanup
clean:
	rm -f $(MODULES) *.o lib/*.o server/*.o train/*.o test/*.o

CC = i586-elf-gcc
LD = i586-elf-gcc
CP = cp

CS452DIR=/u3/cs452/i586-3.3.3

# Build flags
CFLAGS = -std=c99 -nostdinc -nostdinc++ -nodefaultlibs -nostdlib -Wall -I. -Ilib/ -I../kernel/include/kern -I../kernel/include/server -I$(CS452DIR)/include -I$(CS452DIR)/i586-elf/include -I$(CS452DIR)/include/cs452 -I$(CS452DIR)/lib/gcc-lib/i586-elf/3.3.3/include/ 
LDFLAGS = -L$(CS452DIR)/lib -Wl,-T,loader.app.x -Wl,--no-check-sections
LDADD = -lstdc++ -lgcc -lc 

# Compilation
%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Linking
$(MODULES): %: %.o $(COMMON_MODULE_OBJECTS) loader.app.x
	$(LD) $(CFLAGS) $(LDFLAGS) $(filter %.o,$^) $(LDADD) -o $@

