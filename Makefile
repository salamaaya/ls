CC=	cc
CFLAGS=	-ansi -g -Wall -Werror -Wextra -Wformat=2 -Wjump-misses-init \
	-Wlogical-op -Wshadow

PROG=	ls
OBJS=	ls.o cmp.o print.o	

all: ${PROG}

${PROG}: ${OBJS}
	@echo $@ depends on $?
	${CC} ${CFLAGS} ${OBJS} -o ${PROG}

%.o: %.c
	${CC} ${CFLAGS} -c $< -o $@

clean:
	rm -f ${PROG} ${OBJS}
