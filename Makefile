# OS Makefile

# Kernel executable
KERNEL = dekernel
KERNEL := $(KERNEL:%=kernel/%)
# Module executables
MODULES = init idle \
          server/name \
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
          #test/test_srr test/test_srr_worker test/test_ns test/test_ns_child \
          #test/test_time test/test_serial \
          #test/test_ee test/test_eater test/test_ender
MODULES := $(MODULES:%=modules/%)
# Additional data to be posted
EXTRAPOST = 
# The post title
TITLE = dekernel

# What gets done when you just type "make"
all: kernel modules post

# Posting
POSTFILES = $(KERNEL) $(MODULES) $(EXTRAPOST)
post: .autopost

repost: 
	rm -f .autopost
	$(MAKE) post

.autopost: $(POSTFILES)  Makefile
	452post $(TITLE) $(POSTFILES)
	@echo IGNORING -452postemu $(TITLE) $(POSTFILES)
	@touch .autopost

# Kernel
kernel: force_look
	$(MAKE) -C kernel

# Modules
modules: force_look
	$(MAKE) -C modules

# Cleaning
clean: force_look
	$(MAKE) clean -C kernel
	$(MAKE) clean -C modules


force_look:
	true
