# Target library
lib := libfs.a
objs := disk.o fs.o

CC := gcc
CFLAGS := -Wall -Wextra -MMD

ifneq ($(V),1)
Q = @
endif

all: $(lib)
deps := $(patsubst %.o,%.d,$(objs))
-include $(deps)

%.o: %.c
	@echo "CC $@"
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

libfs.a: $(objs)
	ar -rcs $@ $(objs)

clean:
	@echo "clean"
	$(Q)rm -f $(lib) $(objs) $(deps)
## TODO: Phase 1
