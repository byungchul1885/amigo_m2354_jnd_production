#ifndef __KEPCO_CERT_H__
#define __KEPCO_CERT_H__

#include <arm_cmse.h>
#include <stdio.h>

#define KEPCO_CERT_STORAGE_MAGIC 0x4B43534D
#define KEPCO_CERT_ADDRESS 0x2F000

typedef struct
{
    uint8_t systemtitle[16];
    uint32_t systemtitle_len;

    uint8_t sign_prikey[32];
    uint8_t sign_cert[768];
    uint32_t sign_cert_len;

    uint8_t km_prikey[32];
    uint8_t km_cert[768];
    uint32_t km_cert_len;

} kepco_cert_t;

typedef struct
{
    uint32_t magic;
    uint8_t hash[32];
    kepco_cert_t kepco_cert;
} kepco_cert_storage_t;

int get_kepco_certs(kepco_cert_t *kepco_cert);

#endif
