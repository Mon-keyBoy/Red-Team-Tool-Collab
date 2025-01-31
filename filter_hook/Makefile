KMOD= revShellRootkit
SRCS= rev_shells.c
CC= clang15
LD?= /usr/local/bin/ld.lld15

# disable SSP (stack smashing protection) for compilation
CFLAGS += -fno-stack-protector
LDFLAGS += -fno-stack-protector

.include <bsd.kmod.mk>
