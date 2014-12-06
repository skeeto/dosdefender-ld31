static uint16_t kb_read()
{
    uint16_t key;
    asm volatile ("mov $1, %%ah\n"
                  "int $0x16\n"
                  "jnz get%=\n"
                  "mov $0, %%ax\n"
                  "jmp done%=\n"
                  "get%=:\n"
                  "mov $0, %%ah\n"
                  "int $0x16\n"
                  "jmp done%=\n"
                  "done%=:\n"
                  : "=a"(key));
    return key;
}
