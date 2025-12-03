#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "axiocrypto.h"
#include "axiocrypto_util.h"
#include "mbedtls/aes.h"

void mbedtls_aes_init( mbedtls_aes_context *ctx )
{
    CRYPTO_STATUS ret;
    if (NULL == ctx)
        return;
    ctx->mode = 0;
    ctx->algo = 0;
    if (axiocrypto_handle_is_blank(ctx->handle)) {
        ret = axiocrypto_random(ctx->handle, 32);
        if (ret) {
            printf("fail: random %d\n", ret);
            return;
        }
    }
    ret = axiocrypto_allocate_slot(ctx->handle, SYM_AES, 0);
    if (ret) {
        return;
    }
}

void mbedtls_aes_free( mbedtls_aes_context *ctx )
{
    CRYPTO_STATUS ret;
    if (NULL == ctx)
        return;
    ret = axiocrypto_free_slot(ctx->handle, SYM_AES);
    if (ret) {
        return;
    }
}

static int mbedtls_aes_setkey( mbedtls_aes_context *ctx,
                             const unsigned char *key,
                             unsigned int keybits )
{
    CRYPTO_STATUS ret = CRYPTO_SUCCESS;
    switch(keybits) {
        case 128:
        case 192:
        case 256:
            ctx->algo = SYM_AES;
            break;
        default:
            ret = MBEDTLS_ERR_AES_BAD_INPUT_DATA;
            goto cleanup;
    }
    ret = axiocrypto_sym_putkey(ctx->handle, key, (keybits>>3), axiocrypto_crc(key, (keybits>>3)), 0);
    if (ret) { goto cleanup; }
cleanup:
    return ret;
}

int mbedtls_aes_setkey_enc( mbedtls_aes_context *ctx,
                             const unsigned char *key,
                             unsigned int keybits )
{
    CRYPTO_STATUS ret = CRYPTO_SUCCESS;
    ret = mbedtls_aes_setkey(ctx, key, keybits);
    if (ret) { goto cleanup; }
    ctx->mode = MBEDTLS_AES_ENCRYPT;
cleanup:
    return ret;
}

int mbedtls_aes_setkey_dec( mbedtls_aes_context *ctx,
                             const unsigned char *key,
                             unsigned int keybits )
{
    CRYPTO_STATUS ret = CRYPTO_SUCCESS;
    ret = mbedtls_aes_setkey(ctx, key, keybits);
    if (ret) { goto cleanup; }
    ctx->mode = MBEDTLS_AES_DECRYPT;
cleanup:
    return ret;
}

static int mbedtls_aes_crypt( mbedtls_aes_context *ctx,
                            SYM_MODE opmode,
                            int mode,
                            size_t length,
                            unsigned char iv[16],
                            const unsigned char *input,
                            unsigned char *output )
{
    CRYPTO_STATUS ret = CRYPTO_SUCCESS;
    uint32_t osz = 0, sz;
    uint32_t isz = length;
    if (mode == MBEDTLS_AES_ENCRYPT) {
        ret = axiocrypto_sym_enc_init(ctx->handle, ctx->algo, opmode,
                iv, 16);
        if (ret) { goto cleanup; }
        isz = length; osz = length;
        ret = axiocrypto_sym_enc_update(ctx->handle, input, isz, output, &osz);
        if (ret) { goto cleanup; }
        sz = osz;
        osz = length - sz;
        ret = axiocrypto_sym_enc_final(ctx->handle, &output[sz], &osz);
        if (ret) { goto cleanup; }
    } else {
        ret = axiocrypto_sym_dec_init(ctx->handle, ctx->algo, opmode,
                iv, 16);
        if (ret) { goto cleanup; }
        isz = length; osz = length;
        ret = axiocrypto_sym_dec_update(ctx->handle, input, isz, output, &osz);
        if (ret) { goto cleanup; }
        sz = osz;
        osz = length - sz;
        ret = axiocrypto_sym_dec_final(ctx->handle, &output[sz], &osz);
        if (ret) { goto cleanup; }
    }
cleanup:
    return ret;
}

int mbedtls_aes_crypt_ecb( mbedtls_aes_context *ctx,
                    int mode,
                    const unsigned char input[16],
                    unsigned char output[16] )
{
    return mbedtls_aes_crypt(ctx, SYM_MODE_ECB, mode, 16, NULL, input, output);
}

int mbedtls_aes_crypt_cbc( mbedtls_aes_context *ctx,
                            int mode,
                            size_t length,
                            unsigned char iv[16],
                            const unsigned char *input,
                            unsigned char *output )
{
    return mbedtls_aes_crypt(ctx, SYM_MODE_CBC, mode, length, iv, input, output);
}

int mbedtls_aes_crypt_ctr( mbedtls_aes_context *ctx,
                            size_t length,
                            size_t *nc_off,
                            unsigned char nonce_counter[16],
                            unsigned char stream_block[16],
                            const unsigned char *input,
                            unsigned char *output )
{
    (void)nc_off;
    (void)stream_block;
    return mbedtls_aes_crypt(ctx, SYM_MODE_CTR, ctx->mode, length, nonce_counter, input, output);
}

