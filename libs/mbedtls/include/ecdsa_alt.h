#if 0
#pragma once

#include "axiocrypto.h"
#include "mbedtls/ecp.h"

typedef struct mbedtls_ecdsa_context {
    mbedtls_ecp_group grp;      /*!<  Elliptic curve and base point     */
    mbedtls_ecp_point Q;        /*!<  our public value                  */
    mbedtls_mpi d;        /*!<  our public value                  */

    ctx_handle_t handle;
}
mbedtls_ecdsa_context;
#endif
