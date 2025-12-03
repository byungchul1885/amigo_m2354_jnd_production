#include <stdio.h>
#include <string.h>
#include "mbedtls/gcm.h"

void mbedtls_gcm_init( mbedtls_gcm_context *ctx )
{
    CRYPTO_STATUS ret;
    memset(ctx, 0, sizeof(mbedtls_gcm_context));
    ret = axiocrypto_random(ctx->handle, 32);
    if (ret) { goto cleanup; }
cleanup:
    if (ret) {
        printf("%s err: %d\n", __func__, ret);
    }
}

void mbedtls_gcm_free( mbedtls_gcm_context *ctx )
{
    axiocrypto_free_slot(ctx->handle, ctx->algo);
    memset(ctx, 0, sizeof(mbedtls_gcm_context));
}

int mbedtls_gcm_setkey( mbedtls_gcm_context *ctx,
                        mbedtls_cipher_id_t cipher,
                        const unsigned char *key,
                        unsigned int keybits )
{
    CRYPTO_STATUS ret;
    ALGORITHM algo = SYM_AES;
    switch(keybits) {
        case 128:
        case 192:
        case 256:
            if (cipher == MBEDTLS_CIPHER_ID_ARIA) {
                ctx->algo = SYM_ARIA;
                break;
            } if (cipher == MBEDTLS_CIPHER_ID_AES) {
                ctx->algo = SYM_AES;
                break;
            } else {
                ret = MBEDTLS_ERR_GCM_BAD_INPUT;
                goto cleanup;
            }
        default:
            ret = MBEDTLS_ERR_GCM_BAD_INPUT;
            goto cleanup;
    }

    ret = axiocrypto_allocate_slot(ctx->handle, algo, 0);
    if (ret) { goto cleanup; }
    ret = axiocrypto_sym_putkey(ctx->handle, key, (keybits>>3), axiocrypto_crc(key, (keybits>>3)), 0);
    if (ret) { goto cleanup; }
cleanup:
    return ret;
}

int mbedtls_gcm_crypt_and_tag( mbedtls_gcm_context *ctx,
                       int mode,
                       size_t length,
                       const unsigned char *iv,
                       size_t iv_len,
                       const unsigned char *add,
                       size_t add_len,
                       const unsigned char *input,
                       unsigned char *output,
                       size_t tag_len,
                       unsigned char *tag )
{
    CRYPTO_STATUS ret;
    uint32_t osz = length;
    if (mode == MBEDTLS_GCM_ENCRYPT) {
        osz = length;
        ret = axiocrypto_sym_enc_GCM(ctx->handle, ctx->algo, input, length,
                                     add, add_len, tag, tag_len, iv, iv_len, output, &osz);
    } else {
        ret = axiocrypto_sym_dec_GCM(ctx->handle, ctx->algo, input, length,
                                     add, add_len, tag, tag_len, iv, iv_len, output, &osz);
        if (ret == CRYPTO_GCM_ACCEPT) {
            ret = 0;
        } else if (ret == CRYPTO_GCM_REJECT) {
            ret = MBEDTLS_ERR_GCM_AUTH_FAILED;
        }
    }
    return ret;
}

int mbedtls_gcm_auth_decrypt( mbedtls_gcm_context *ctx,
                      size_t length,
                      const unsigned char *iv,
                      size_t iv_len,
                      const unsigned char *add,
                      size_t add_len,
                      const unsigned char *tag,
                      size_t tag_len,
                      const unsigned char *input,
                      unsigned char *output )
{
    CRYPTO_STATUS ret;
    uint32_t osz = length;
    ret = axiocrypto_sym_dec_GCM(ctx->handle, ctx->algo, input, length,
                                 add, add_len, tag, tag_len, iv, iv_len, output, &osz);
    if (ret == CRYPTO_GCM_ACCEPT) {
        ret = 0;
    } else {
        ret = MBEDTLS_ERR_GCM_AUTH_FAILED;
    }
    return ret;
}

int mbedtls_gcm_starts( mbedtls_gcm_context *ctx,
                int mode,
                const unsigned char *iv,
                size_t iv_len,
                const unsigned char *add,
                size_t add_len )
{
	(void)ctx;
	(void)mode;
	(void)iv;
	(void)iv_len;
	(void)add;
	(void)add_len;

	return MBEDTLS_ERR_GCM_BAD_INPUT;
}

int mbedtls_gcm_finish( mbedtls_gcm_context *ctx,
                unsigned char *tag,
                size_t tag_len )
{
	(void)ctx;
	(void)tag;
	(void)tag_len;

	return MBEDTLS_ERR_GCM_BAD_INPUT;
}

int mbedtls_gcm_update( mbedtls_gcm_context *ctx,
                size_t length,
                const unsigned char *input,
                unsigned char *output )
					
{
		(void)ctx;
	(void)length;
	(void)input;
	(void)output;

	return MBEDTLS_ERR_GCM_BAD_INPUT;
}
