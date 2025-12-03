#pragma once
#include "axiocrypto.h"
#include "axiocrypto_util.h"

typedef struct mbedtls_gcm_context {
    uint8_t handle[32];
    ALGORITHM algo;
}
mbedtls_gcm_context;
