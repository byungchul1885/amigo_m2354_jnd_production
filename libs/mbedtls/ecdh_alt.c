#include <stdint.h>
#include <string.h>
#include "mbedtls/ecdh.h"
#include "axiocrypto.h"
#include "axiocrypto_util.h"

/*
 * d is for destination. not source
 */
int mbedtls_ecdh_gen_public( mbedtls_ecp_group *grp, mbedtls_mpi *d, mbedtls_ecp_point *Q,
                     int (*f_rng)(void *, unsigned char *, size_t),
                     void *p_rng )
{
    CRYPTO_STATUS ret;
    ctx_handle_t handle;
    uint8_t dbuf[32] = {0,};
    uint8_t qbuf[64] = {0,};
    (void)grp;
    (void)f_rng;
    (void)p_rng;
    ret = axiocrypto_random(handle, sizeof(ctx_handle_t));
    if (ret) { goto cleanup; }
    ret = axiocrypto_allocate_slot(handle, ASYM_ECDH_P256, 0);
    if (ret) { goto cleanup; }
    ret = axiocrypto_random(dbuf, 32);
    if (ret) { goto cleanup; }
    ret = axiocrypto_asym_putkey(handle, ASYM_ECDH_P256, dbuf, 32, axiocrypto_crc(dbuf, 32), NULL, 0, 0, 0);
    if (ret) { goto cleanup; }
    ret = axiocrypto_asym_getkey(handle, ASYM_ECDH_P256, qbuf, 64);
    if (ret) { goto cleanup; }
    ret = mbedtls_mpi_read_binary(&Q->X, &qbuf[0], 32);
    if (ret) { goto cleanup; }
    ret = mbedtls_mpi_read_binary(&Q->Y, &qbuf[32], 32);
    if (ret) { goto cleanup; }
    ret = mbedtls_mpi_lset(&Q->Z, 1);
    if (ret) { goto cleanup; }
    ret = mbedtls_mpi_read_binary(d, dbuf, 32);
    if (ret) { goto cleanup; }
    ret = axiocrypto_free_slot(handle, ASYM_ECDH_P256);
    if (ret) { goto cleanup; }
cleanup:
    memset(dbuf, 0, 32);
    memset(qbuf, 0, 64);
    return ret;
}

int mbedtls_ecdh_compute_shared( mbedtls_ecp_group *grp, mbedtls_mpi *z,
                         const mbedtls_ecp_point *Q, const mbedtls_mpi *d,
                         int (*f_rng)(void *, unsigned char *, size_t),
                         void *p_rng )
{
    CRYPTO_STATUS ret;
    ctx_handle_t handle;
    uint8_t dbuf[32];
    uint8_t qbuf[64];
    uint8_t zbuf[32];
    (void)grp;
    (void)f_rng;
    (void)p_rng;
    ret = axiocrypto_random(handle, sizeof(ctx_handle_t));
    if (ret) { goto cleanup; }
    ret = axiocrypto_allocate_slot(handle, ASYM_ECDH_P256, 0);
    if (ret) { goto cleanup; }
    ret = mbedtls_mpi_write_binary(d, dbuf, 32);
    if (ret) { goto cleanup; }
    ret = axiocrypto_asym_putkey(handle, ASYM_ECDH_P256, dbuf, 32, axiocrypto_crc(dbuf, 32), NULL, 0, 0, 0);
    if (ret) { goto cleanup; }
    ret = axiocrypto_asym_getkey(handle, ASYM_ECDH_P256, qbuf, 64);
    if (ret) { goto cleanup; }
    ret = mbedtls_mpi_write_binary(&Q->X, &qbuf[0], 32);
    if (ret) { goto cleanup; }
    ret = mbedtls_mpi_write_binary(&Q->Y, &qbuf[32], 32);
    if (ret) { goto cleanup; }
    ret = axiocrypto_ecdh_computekey(handle, qbuf, 64, zbuf, 32);
    if (ret) { goto cleanup; }
    ret = mbedtls_mpi_read_binary(z, zbuf, 32);
    if (ret) { goto cleanup; }
    ret = axiocrypto_free_slot(handle, ASYM_ECDH_P256);
    if (ret) { goto cleanup; }
cleanup:
    memset(dbuf, 0, 32);
    memset(qbuf, 0, 64);
    return ret;
}
