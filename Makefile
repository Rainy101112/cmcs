CC      := gcc
C_FLAGS := -Wall -Wextra -O2 -ggdb -I include

C_SOURCES  := $(shell find * -name "*.c")
C_HEADERS  := $(shell find * -name "*.h")
OBJS       := $(C_SOURCES:%.c=%.o)
DEPS       := $(OBJS:%.o=%.d)

all: server

%.o: %.c
	@printf "  CC      $@\n"
	@$(CC) $(C_FLAGS) -c -o $@ $<

server: $(OBJS)
	@printf "  LD      $@\n"
	@$(CC) $(C_FLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -f $(OBJS) server
