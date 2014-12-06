ENTRY(_main);
EXTERN(_heap);
SECTIONS
{
    . = 0x0100;
    .text : AT(0x0100)
    {
        _text = .;
        *(.text);
        _text_end = .;
    }
    .data :
    {
        _data = .;
        *(.bss);
        *(.bss*);
        *(.data);
        *(.rodata*);
        *(COMMON)
        _data_end = .;
    }
    . = ALIGN(4);
    .heap :
    {
       _heap = .;
    }
    /DISCARD/ :
    {
        *(.sig*);
        *(.note*);
        *(.iplt*);
        *(.igot*);
        *(.rel*);
        *(.comment);
    }
}
