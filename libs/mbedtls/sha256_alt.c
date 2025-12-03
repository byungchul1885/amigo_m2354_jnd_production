#include <string.h>
#include <stdlib.h>
#include "mbedtls/sha256.h"
#include "axiocrypto.h"

void mbedtls_sha256_init( mbedtls_sha256_context *ctx )
{
    memset(ctx, 0, sizeof(mbedtls_sha256_context));
}

void mbedtls_sha256_free( mbedtls_sha256_context *ctx )
{
    memset(ctx, 0, sizeof(mbedtls_sha256_context));
}

int mbedtls_sha256_starts_ret( mbedtls_sha256_context *ctx, int is224 )
{
    CRYPTO_STATUS ret;
    (void)is224;
    ret = axiocrypto_random(ctx->handle, 32);
    if (ret) {
        goto cleanup;
    }
    ret = axiocrypto_hash_init(ctx->handle, HASH_SHA_256);
    if (ret) {
        goto cleanup;
    }
cleanup:
    return ret;
}

int mbedtls_sha256_update_ret( mbedtls_sha256_context *ctx,
                               const unsigned char *input,
                               size_t ilen )
{
    CRYPTO_STATUS ret;
    ret = axiocrypto_hash_update(ctx->handle, input, ilen);
    if (ret) {
        goto cleanup;
    }
cleanup:
    return ret;
}

int mbedtls_sha256_finish_ret( mbedtls_sha256_context *ctx,
                               unsigned char output[32] )
{
    CRYPTO_STATUS ret;
    ret = axiocrypto_hash_final(ctx->handle, output, 32);
    if (ret) {
        goto cleanup;
    }
cleanup:
    return ret;
}

void mbedtls_sha256_clone( mbedtls_sha256_context *dst,
                           const mbedtls_sha256_context *src )
{
    (void)dst;
    (void)src;
}

int mbedtls_internal_sha256_process( mbedtls_sha256_context *ctx,
                                const unsigned char data[64] )
{
    (void)ctx;
    (void)data;
    return -1;
}
