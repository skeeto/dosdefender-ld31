ENTRY(_main);
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
