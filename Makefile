CC = cc
CFLAGS = -ansi -g -Wall -Werror -Wextra -Wformat=2 -Wjump-misses-init \
	-Wlogical-op -Wpedantic -Wshadow

PROG = ls

.PHONY: all clean

all: $(PROG)

$(PROG): ls.c
	$(CC) $(CFLAGS) -o $@ ls.c

clean:
	rm -f $(PROG) *.o *~
