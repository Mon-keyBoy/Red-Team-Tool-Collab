SRCS= fuckme.c
OBJS= fuckme.o
CC= clang15
LD?= /usr/local/bin/ld.lld15
CFLAGS += -D_KERNEL