#pragma once

typedef struct mbedtls_sha256_context {
    uint8_t handle[32];
}
mbedtls_sha256_context;
