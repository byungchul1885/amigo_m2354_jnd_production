#pragma once
#include "axiocrypto.h"

typedef struct mbedtls_aria_context
{
    uint8_t handle[32];
    int mode;
    ALGORITHM algo;
}
mbedtls_aria_context;
