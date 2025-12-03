#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "axiocrypto.h"
#include "axiocrypto_util.h"
#include "mbedtls/aria.h"

void mbedtls_aria_init( mbedtls_aria_context *ctx )
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
    ret = axiocrypto_allocate_slot(ctx->handle, SYM_ARIA, 0);
    if (ret) {
        return;
    }
}

void mbedtls_aria_free( mbedtls_aria_context *ctx )
{
    CRYPTO_STATUS ret;
    if (NULL == ctx)
        return;
    ret = axiocrypto_free_slot(ctx->handle, SYM_ARIA);
    if (ret) {
        return;
    }
}

static int mbedtls_aria_setkey( mbedtls_aria_context *ctx,
                             const unsigned char *key,
                             unsigned int keybits )
{
    CRYPTO_STATUS ret;
    switch(keybits) {
        case 128:
        case 192:
        case 256:
            ctx->algo = SYM_ARIA;
            break;
        default:
            ret = MBEDTLS_ERR_ARIA_BAD_INPUT_DATA;
            goto cleanup;
    }

    ret = axiocrypto_sym_putkey(ctx->handle, key, (keybits>>3), axiocrypto_crc(key, (keybits>>3)), 0);
    if (ret) { goto cleanup; }
cleanup:
    return ret;
}

int mbedtls_aria_setkey_enc( mbedtls_aria_context *ctx,
                             const unsigned char *key,
                             unsigned int keybits )
{
    CRYPTO_STATUS ret;
    ret = mbedtls_aria_setkey(ctx, key, keybits);
    if (ret) { goto cleanup; }
    ctx->mode = MBEDTLS_ARIA_ENCRYPT;
cleanup:
    return ret;
}

int mbedtls_aria_setkey_dec( mbedtls_aria_context *ctx,
                             const unsigned char *key,
                             unsigned int keybits )
{
    CRYPTO_STATUS ret;
    ret = mbedtls_aria_setkey(ctx, key, keybits);
    if (ret) { goto cleanup; }
    ctx->mode = MBEDTLS_ARIA_DECRYPT;
cleanup:
    return ret;
}

static int mbedtls_aria_crypt( mbedtls_aria_context *ctx,
                            SYM_MODE opmode,
                            int mode,
                            size_t length,
                            unsigned char iv[MBEDTLS_ARIA_BLOCKSIZE],
                            const unsigned char *input,
                            unsigned char *output )
{
    CRYPTO_STATUS ret;
    uint32_t osz = 0, sz;
    uint32_t isz = length;
    if (mode == MBEDTLS_ARIA_ENCRYPT) {
        ret = axiocrypto_sym_enc_init(ctx->handle, ctx->algo, opmode,
                iv, MBEDTLS_ARIA_BLOCKSIZE);
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
                iv, MBEDTLS_ARIA_BLOCKSIZE);
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

int mbedtls_aria_crypt_cbc( mbedtls_aria_context *ctx,
                            int mode,
                            size_t length,
                            unsigned char iv[MBEDTLS_ARIA_BLOCKSIZE],
                            const unsigned char *input,
                            unsigned char *output )
{
    return mbedtls_aria_crypt(ctx, SYM_MODE_CBC, mode, length, iv, input, output);
}

int mbedtls_aria_crypt_ctr( mbedtls_aria_context *ctx,
                            size_t length,
                            size_t *nc_off,
                            unsigned char nonce_counter[MBEDTLS_ARIA_BLOCKSIZE],
                            unsigned char stream_block[MBEDTLS_ARIA_BLOCKSIZE],
                            const unsigned char *input,
                            unsigned char *output )
{
    (void)nc_off;
    (void)stream_block;
    return mbedtls_aria_crypt(ctx, SYM_MODE_CTR, ctx->mode, length, nonce_counter, input, output);
}


int mbedtls_aria_crypt_ecb( mbedtls_aria_context *ctx,
                            const unsigned char input[MBEDTLS_ARIA_BLOCKSIZE],
                            unsigned char output[MBEDTLS_ARIA_BLOCKSIZE] )
								
{
	(void)ctx;
	(void)input;
	(void)output;

	return MBEDTLS_ERR_ARIA_BAD_INPUT_DATA;
}

