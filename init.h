/* NOTICE: This must be included first! */
asm ("call  dosmain\n"
     "mov   $0x4C,%ah\n"
     "int   $0x21\n");
