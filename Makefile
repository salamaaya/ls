CC=	cc
CFLAGS=	-ansi -g -Wall -Werror -Wextra -Wformat=2 -Wjump-misses-init \
	-Wlogical-op -Wpedantic -Wshadow

PROG=	ls
OBJS=	ls.o cmp.o	

all: ${PROG}

${PROG}: ${OBJS}
	@echo $@ depends on $?
	${CC} ${CFLAGS} ${OBJS} -o ${PROG}

clean:
	rm -f $(PROG) *.o *~
