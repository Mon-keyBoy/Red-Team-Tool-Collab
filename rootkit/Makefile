KMOD= rootkit
SRCS= rootkit_lkm.c
CC= clang15
LD?= /usr/local/bin/ld.lld15 
CFLAGS+= -fno-stack-protector -O2 -g


.include <bsd.kmod.mk>
