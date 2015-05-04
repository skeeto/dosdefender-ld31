HOST    = x86_64-w64-mingw32
CC      = $(HOST)-gcc
OBJCOPY = $(HOST)-objcopy
DOS     = dosbox
CFLAGS  = -std=gnu99 -Wall -Wextra -Os -nostdlib -m32 -march=i386 \
  -Wno-unused-function \
  -ffreestanding -fomit-frame-pointer -fwrapv -fno-strict-aliasing \
  -fno-leading-underscore \
  -Wl,--nmagic,-static,-Tcom.ld

dosdef.com : dosdef.exe
dosdef.exe : dosdef.c *.h

.PHONY : all clean test

clean :
	$(RM) *.com

test : dosdef.com
	$(DOS) $^

%.exe : %.c
	$(CC) -o $@ $(CFLAGS) $<

%.com : %.exe
	$(OBJCOPY) -O binary $< $@
