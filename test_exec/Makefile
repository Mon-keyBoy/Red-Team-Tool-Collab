SRCS= fuckme.c
OBJS= fuckme.o
CC= clang15
LD?= /usr/local/bin/ld.lld15
CFLAGS += -D_KERNEL
bb= /usr/src/sys
CFLAGS += -I$(bb) -I$(bb)/amd64/include -I$(bb)/netinet -I$(bb)/sys


all: fuckme

fuckme: $(OBJS)
	$(CC) $(CFLAGS) -o fuckme $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) my_exec