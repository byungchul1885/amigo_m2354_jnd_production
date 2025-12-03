#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "axiocrypto.h"
#include "axiocrypto_util.h"
#include "mbedtls/ecdsa.h"
#include "mbedtls/bignum.h"

#if defined(MBEDTLS_ECDSA_SIGN_ALT)
int mbedtls_ecdsa_sign( mbedtls_ecp_group *grp, mbedtls_mpi *r, mbedtls_mpi *s,
                const mbedtls_mpi *d, const unsigned char *buf, size_t blen,
                int (*f_rng)(void *, unsigned char *, size_t), void *p_rng )
{
    CRYPTO_STATUS ret =0;
    ctx_handle_t handle;
    uint8_t *dbuf;
    uint8_t *sig;
    uint32_t sigsz = 64;
    (void)grp;
    (void)f_rng;
    (void)p_rng;

    dbuf = calloc(32, sizeof(uint8_t));
    sig = calloc(64, sizeof(uint8_t));
    ret = axiocrypto_random(handle, sizeof(ctx_handle_t));
    if (ret) { goto cleanup; }
    ret = axiocrypto_allocate_slot(handle, ASYM_ECDSA_P256, 0);
    if (ret) { goto cleanup; }
    ret = mbedtls_mpi_write_binary((mbedtls_mpi *)d, dbuf, 32);
    if (ret) { goto cleanup; }
    ret = axiocrypto_asym_putkey(handle, ASYM_ECDSA_P256, dbuf, 32, axiocrypto_crc(dbuf, 32), NULL, 0, 0, 0);
    if (ret) { goto cleanup; }
    ret = axiocrypto_asym_sign(handle, buf, blen, 1, sig, &sigsz);
    if (ret) { goto cleanup; }
    ret = mbedtls_mpi_read_binary(r, sig, 32);
    if (ret) { goto cleanup; }
    ret = mbedtls_mpi_read_binary(s, &sig[32], 32);
    if (ret) { goto cleanup; }
cleanup:
    memset(dbuf, 0, 32);
    free(dbuf);
    memset(sig, 0, 64);
    free(sig);
    axiocrypto_free_slot(handle, ASYM_ECDSA_P256);
    return ret;
}
#endif

#if defined(MBEDTLS_ECDSA_VERIFY_ALT)
// buf is hashed msg
int mbedtls_ecdsa_verify( mbedtls_ecp_group *grp,
                          const unsigned char *buf, size_t blen,
                          const mbedtls_ecp_point *Q,
                          const mbedtls_mpi *r,
                          const mbedtls_mpi *s)
{
    CRYPTO_STATUS ret =0;
    ctx_handle_t handle;
    uint8_t q[64];
    uint8_t sig[64];

    (void)grp;
    axiocrypto_handle_init(handle, NULL);
    ret = axiocrypto_random(handle, sizeof(ctx_handle_t));
    if (ret) { return ret; }
    ret = axiocrypto_allocate_slot(handle, ASYM_ECDSA_P256, 0);
    if (ret) { return ret; }

    ret = mbedtls_mpi_write_binary(&Q->X, q, 32);
    if (ret) { goto cleanup; }
    ret = mbedtls_mpi_write_binary(&Q->Y, &q[32], 32);
    if (ret) { goto cleanup; }
    ret = mbedtls_mpi_write_binary(r, sig, 32);
    if (ret) { goto cleanup; }
    ret = mbedtls_mpi_write_binary(s, &sig[32], 32);
    if (ret) { goto cleanup; }

    ret = axiocrypto_asym_putkey(handle, ASYM_ECDSA_P256, NULL, 0, 0, q, 64, axiocrypto_crc(q, 64), CTX_ATTR_NONE);
    if (ret) { goto cleanup; }

    ret = axiocrypto_asym_verify(handle, buf, blen, 1, sig, 64);
    if (ret && ret != CRYPTO_SIG_ACCEPT) { goto cleanup; }

cleanup:
    axiocrypto_free_slot(handle, ASYM_ECDSA_P256);
    if (ret == CRYPTO_SIG_ACCEPT) {
        ret = 0;
    } else if (ret == CRYPTO_SIG_REJECT) {
        ret = MBEDTLS_ERR_ECP_VERIFY_FAILED;
    }
    return ret;
}
#endif

#if defined(MBEDTLS_ECDSA_GENKEY_ALT)
int mbedtls_ecdsa_genkey( mbedtls_ecdsa_context *ctx, mbedtls_ecp_group_id gid,
                  int (*f_rng)(void *, unsigned char *, size_t), void *p_rng )
{
    CRYPTO_STATUS ret =0;
    (void)gid;
    (void)f_rng;
    (void)p_rng;
    uint8_t Qbuf[64] = {0,};
    uint8_t dbuf[32];
    ctx_handle_t handle;

    ret = axiocrypto_random(handle, 32);
    if (ret) { goto cleanup; }
    ret = axiocrypto_allocate_slot(handle, ASYM_ECDSA_P256, 0);
    if (ret) { goto cleanup; }

    ret = axiocrypto_random(dbuf, 32);
    if (ret) { goto cleanup; }

    ret = mbedtls_mpi_read_binary(&ctx->d, dbuf, 32);
    if (ret) { goto cleanup; }

    ret = axiocrypto_asym_putkey(handle, ASYM_ECDSA_P256, dbuf, 32, axiocrypto_crc(dbuf, 32), NULL, 0, 0, CTX_ATTR_NONE);
    if (ret) { goto cleanup; }

    ret = axiocrypto_asym_getkey(handle, ASYM_ECDSA_P256, Qbuf, 64);
    if (ret) { goto cleanup; }

    ret = mbedtls_mpi_read_binary(&ctx->Q.X, Qbuf, 32);
    if (ret) { goto cleanup; }

    ret = mbedtls_mpi_read_binary(&ctx->Q.Y, &Qbuf[32], 32);
    if (ret) { goto cleanup; }

cleanup:
    memset(dbuf, 0, 32);
    ret = axiocrypto_free_slot(handle, ASYM_ECDSA_P256);
    return ret;
}
#endif
