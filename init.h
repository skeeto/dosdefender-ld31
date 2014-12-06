/* NOTICE: This must be included first! */
asm (".code16gcc\n"
     "call  _main\n"
     "mov   $0x4C,%ah\n"
     "int   $0x21\n");
