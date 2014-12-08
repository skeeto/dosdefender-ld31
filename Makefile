CC      = gcc
OBJCOPY = objcopy
DOS     = dosbox
CFLAGS  = -std=gnu99 -Wall -Os -nostdlib -m32 -march=i386 \
  -Wno-unused-function \
  -ffreestanding -fomit-frame-pointer -fwrapv -fno-strict-aliasing \
  -fno-leading-underscore \
  -Wl,--nmagic,-static,--build-id=none,-Tcom.ld

dosdef.com : dosdef.c *.h

.PHONY : all clean test

clean :
	$(RM) *.com *.o

test : dosdef.com
	$(DOS) $^

%.com : %.c
	$(CC) -o $@.o $(CFLAGS) $<
	$(OBJCOPY) -O binary $@.o $@
