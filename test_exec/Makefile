SRCS= fuckme.c
OBJS= fuckme.o
CC= clang15
LD?= /usr/local/bin/ld.lld15
CFLAGS += -D_KERNEL

all: my_exec

my_exec: $(OBJS)
	$(CC) $(CFLAGS) -o my_exec $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) my_exec