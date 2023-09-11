CC      = gcc
DOS     = dosbox
CFLAGS  = -std=gnu99 -Wall -Wextra -Os -nostdlib -m16 -march=i386 \
  -Wno-unused-function \
  -ffreestanding -fomit-frame-pointer -fwrapv -fno-strict-aliasing \
  -fno-leading-underscore -fno-pic -fno-stack-protector \
  -fcf-protection=none \
  -Wl,--nmagic,-static,-Tcom.ld

dosdef.com : dosdef.c *.h

.PHONY : all clean test

clean :
	$(RM) *.com

test : dosdef.com
	$(DOS) $^

%.com : %.c
	$(CC) -o $@ $(CFLAGS) $<
