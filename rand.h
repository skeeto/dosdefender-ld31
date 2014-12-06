#ifndef RANDOM_H
#define RANDOM_H

#include "int.h"

#define RAND_MAX 4294967295

static uint32_t rand_seed = 375226057;

static uint32_t rand()
{
    rand_seed = rand_seed * 1103515245 + 12345;
    return rand_seed;
}

#endif
