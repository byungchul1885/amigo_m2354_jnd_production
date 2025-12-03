/**
 * @file DLMS_HLS_ECDSA.h
 */
#ifndef __HLS_H__
#define __HLS_H__

#include "axiocrypto.h"

#define SECURITY_SUITE_ALG_OID_LEN              7
#define SYSTEM_TITLE_LEN                        8
#define AUTH_KEY_LEN                            16
#define CRYPT_KEY_LEN                           16
#define ADD_LEN                                 17
#define RANDOM_LEN                              32
#define SHA256_LEN                              32
#define KEY_DERIVATION_LEN                      32
#define COMMON_NAME_SIZE                        13
#define VENDOR_ID_SIZE                          3
#define HLS_ECDSA_AUTHENTICATION_DATA_LEN       80
#define OTHER_INFO_LEN                          (SECURITY_SUITE_ALG_OID_LEN + SYSTEM_TITLE_LEN + SYSTEM_TITLE_LEN)
#define INITIAL_VECTOR_LEN                      12
#define AUTH_TAG_LEN                            12
#define INVOCATION_COUNTER_LEN                  4
#define GCM_KEY_BIT_SIZE                        128
#define SIGNATURE_R_S_LEN                       32
#define KDF_LEN                                 32
#define ZKEY_LEN                                32
#define HLS_PUBKEY_LEN                          64
#define HLS_PRIKEY_LEN                          32
/**
 * @file hls.h
 *
 * @brief API functions in HLS
 */

/** @defgroup hls
 * @brief DLMS HLS library
 * @{
 */


/**
 * @brief   HLS 암호 알고리즘 타입
 */
typedef enum {
    DLMS_CRYPT_NOT_SUPPORT = 0,   
    DLMS_CRYPT_AES = 1,     /**< AES */ 
    DLMS_CRYPT_ARIA = 15,   /**< ARIA */ 
}hls_crypto_algo_t;

/**
 * @brief   HLS 암호화 타입
 */
typedef enum {
    DLMS_CRYPT_AUTH_NONE = 0,
    DLMS_ONLY_AUTH       = 1,   /**< 인증만 진행*/ 
    DLMS_ONLY_CRYPT      = 2,   /**< 암호화만  진행*/ 
    DLMS_CRYPT_AUTH      = 3,   /**< 인증과 암호화 모두 진행 */ 
 }hls_crypto_auth_t;

/**
 * @brief   HLS 운용 모드
 */
typedef enum{
    HLS_OPMODE_NONE   = 0,  
    HLS_OPMODE_CLIENT = 1,  /**< 클라이언트에서 사용하는 경우 */
    HLS_OPMODE_SERVER = 2,  /**< 서버에서 사용하는 경우 */
}hls_opmode_t;

/**
 * @brief   HLS 암복호화/인증 사용을 위한 설정 정보 구조체
 */
typedef struct {
    ctx_handle_t ecdsa_key;                             /**< 자신의 ECDSA용 AxioCrypto 핸들 */
    unsigned char other_pubkey[HLS_PUBKEY_LEN];         /**< 상대방의 ECDSA용 공개 키*/

    
    ctx_handle_t ecdh_key;                              /**< 자신의 ECDH용 AxioCrypto 핸들 */
    unsigned char ecdh_other_pubkey[HLS_PUBKEY_LEN];    /**< 상대방의 ECDH용 공개 키*/

    ctx_handle_t temp_key;                              /**< 임시 AxioCrypto 핸들*/

    unsigned char rand[RANDOM_LEN];                     /**< 자신의 난수 <32 BYTE> */
    unsigned char other_rand[RANDOM_LEN];               /**< 상대방의 난수 <32 BYTE> */

    unsigned char system_title[SYSTEM_TITLE_LEN];       /**< 자신의 시스템 타이틀 데이터 <8 BYTE> */
    unsigned char other_system_title[SYSTEM_TITLE_LEN]; /**< 상대방의 시스템 타이틀 데이터 <8 BYTE> */

}hls_info_t;

/**
 * @brief   HLS 사용을 위한 설정 정보 구조체
 */
typedef struct{
    hls_info_t *hls_info;                   /**< HLS 정보 구조체 포인터 */

    hls_opmode_t opmode;                    /**< 운용모드 */
    hls_crypto_algo_t crypto_algo;          /**< HLS 알고리즘 종류 */
    hls_crypto_auth_t crypto_auth;          /**< HLS 인증 방식 */
    unsigned char z_key[ZKEY_LEN];          /**< ECDHE 공유 키 */
}hls_context_t;


/**
 * @brief   HLS를 초기화 한다.
 * @param[in]   opmode HLS 운용 모드
 * @return 성공시 hls_context_t형 포인터, 실패시 NULL
 */
hls_context_t *hls_init(hls_opmode_t opmode);

/**
 * @brief HLS를 종료한다.
 * 
 * @param[in] ctx  hls_context_t 형 포인터
 */
void hls_deinit(hls_context_t *ctx);


/**
 * @brief 자신의 시스템타이틀을 설정한다.
 * 
 * @param[in] ctx hls_context_t 포인터
 * @param[in] sys_title     시스템타이틀
 * @param[in] sys_title_len  시스템타이틀 길이
 * @return int 성공시 HLS_SUCCESS, 실패시 HLS error code 참고
 */
int hls_set_system_title(hls_context_t *ctx, unsigned char *sys_title, int sys_title_len);

/**
 * @brief 상대방의 시스템타이틀을 설정한다.
 * 
 * @param[in] ctx hls_context_t 포인터
 * @param[in] sys_title     시스템타이틀
 * @param[in] sys_title_len  시스템타이틀 길이
 * @return int 성공시 HLS_SUCCESS, 실패시 HLS error code 참고
 */
int hls_set_other_system_title(hls_context_t *ctx, unsigned char *sys_title, int sys_title_len);

/**
 * @brief 자신의 시스템타이틀을 얻어온다.
 * 
 * @param[in] ctx hls_context_t 포인터
 * @param[out] sys_title     시스템타이틀
 * @param[out] sys_title_len  시스템타이틀 길이
 * @return int 성공시 HLS_SUCCESS, 실패시 HLS error code 참고
 */
int hls_get_system_title(hls_context_t *ctx, unsigned char *sys_title, int *sys_title_len);

/**
 * @brief 상대방의 시스템타이틀을 얻어온다.
 * 
 * @param[in] ctx hls_context_t 포인터
 * @param[out] sys_title     시스템타이틀
 * @param[out] sys_title_len  시스템타이틀 길이
 * @return int 성공시 HLS_SUCCESS, 실패시 HLS error code 참고
 */
int hls_get_other_system_titile(hls_context_t *ctx, unsigned char *sys_title, int *sys_title_len);

/**
 * @brief 자신의 ECDSA용 키를 설정한다.
 * 
 * @param[in] ctx           hls_context_t 포인터
 * @param[in] prikey        ECDSA용 개인키
 * @param[in] prikey_len    ECDSA용 개인키 길이
 * @param[in] pubkey        ECDSA용 공개키
 * @param[in] pubkey_len    ECDSA용 공개키 길이
 * @return int 성공시 HLS_SUCCESS, 실패시 HLS error code 참고
 */
int hls_set_ecdsa_key(hls_context_t *ctx, unsigned char *prikey, int prikey_len, unsigned char *pubkey, int pubkey_len);

/**
 * @brief 상대방의 ECDSA용 키를 설정한다.
 * 
 * @param[in] ctx           hls_context_t 포인터
 * @param[in] pubkey        ECDSA용 공개키
 * @param[in] pubkey_len    ECDSA용 공개키 길이
 * @return                  성공시 HLS_SUCCESS, 실패시 HLS error code 참고
 */
int hls_set_other_ecdsa_pub_key(hls_context_t *ctx, unsigned char *pubkey, int pubkey_len);

/**
 * @brief 자신의 ECDSA용 공개키를 얻어온다.
 * 
 * @param[in] ctx               hls_context_t 포인터
 * @param[out] pubkey            ECDSA 공개키
 * @param[out] pubkey_len        ECDSA 공개키 길이
 * @return                  성공시 HLS_SUCCESS, 실패시 HLS error code 참고
 */
int hls_get_ecdsa_key(hls_context_t *ctx, unsigned char *pubkey, int *pubkey_len);

/**
 * @brief 상대방의 ECDSA 공개키를 얻어온다.
 * 
 * @param[in] ctx               hls_context_t 포인터
 * @param[out] pubkey            ECDSA 공개키
 * @param[out] pubkey_len        ECDSA 공개키 길이
 * @return 성공시 HLS_SUCCESS, 실패시 HLS error code 참고
 */
int hls_get_other_ecdsa_key(hls_context_t *ctx, unsigned char *pubkey, int *pubkey_len);

// ECDH keys
/**
 * @brief ECDH용 키를 생성한다.
 * 
 * @param[in] ctx               hls_context_t 포인터
 * @return 성공시 HLS_SUCCESS, 실패시 HLS error code 참고
 */
int hls_gen_ecdh_key(hls_context_t *ctx);

/**
 * @brief 생성된 ECDH 공개키를 얻어온다.
 * 
 * @param[in] ctx               hls_context_t 포인터
 * @param[out] pubkey            ECDH 공개키
 * @param[out] pubkey_len        ECDH 공개키 길이
 * @return 성공시 HLS_SUCCESS, 실패시 HLS error code 참고 
 */
int hls_get_ecdh_pubkey(hls_context_t *ctx, unsigned char *pubkey, int *pubkey_len);

/**
 * @brief 상대방의 ECDH 공개키를 설정한다.
 * 
 * @param[in] ctx               hls_context_t 포인터
 * @param[in] pubkey            ECDH 공개키
 * @param[in] pubkey_len        ECDH 공개키 길이
 * @return 성공시 HLS_SUCCESS, 실패시 HLS error code 참고 
 */
int hls_set_ecdh_other_pubkey(hls_context_t *ctx, unsigned char *pubkey, int pubkey_len);

/**
 * @brief 상대방의 ECH 공개키를 얻어온다.
 * 
 * @param[in]ctx               hls_context_t 포인터
 * @param[out] pubkey            ECDH 공개키
 * @param[out] pubkey_len        ECDH 공개키 길이
 * @return 성공시 HLS_SUCCESS, 실패시 HLS error code 참고 
 */
int hls_get_ecdh_other_pubkey(hls_context_t *ctx, unsigned char *pubkey, int *pubkey_len);

/**
 * @brief ECDH 공유키를 생성한다.
 * 
 * @param[in] ctx               hls_context_t 포인터
 * @return 성공시 HLS_SUCCESS, 실패시 HLS error code 참고 
 */
int hls_gen_share_key(hls_context_t *ctx);

// rand
/**
 * @brief 자신의 난수정보를 설정한다.
 * 
 * @param[in] ctx               hls_context_t 포인터
 * @param[in] rand              난수정보           
 * @param[in] rand_len          난수정보 길이
 * @return 성공시 HLS_SUCCESS, 실패시 HLS error code 참고 
 */
int hls_set_rand(hls_context_t *ctx, unsigned char *rand, int rand_len);

/**
 * @brief 자신의 난수정보를 얻어온다.
 * 
 * @param[in] ctx               hls_context_t 포인터
 * @param[out] rand              난수정보           
 * @param[out] rand_len          난수정보 길이
 * @return 성공시 HLS_SUCCESS, 실패시 HLS error code 참고  
 */
int hls_get_rand(hls_context_t *ctx, unsigned char *rand, int *rand_len);

/**
 * @brief 상대방의 난수정보를 설정한다.
 * 
 * @param[in] ctx               hls_context_t 포인터
 * @param[in] rand              난수정보           
 * @param[in] rand_len          난수정보 길이
 * @return 성공시 HLS_SUCCESS, 실패시 HLS error code 참고 
 */
int hls_set_other_rand(hls_context_t *ctx, unsigned char *rand, int rand_len);

/**
 * @brief 상대방의 난수정보를 얻어온다.
 * 
 * @param[in] ctx               hls_context_t 포인터
 * @param[out] rand              난수정보           
 * @param[out] rand_len          난수정보 길이
 * @return 성공시 HLS_SUCCESS, 실패시 HLS error code 참고 
 */
int hls_get_other_rand(hls_context_t *ctx, unsigned char *rand, int *rand_len);

/**
 * @brief 자신의 난수정보를 생성한다.
 * 
 * @param[in] ctx               hls_context_t 포인터
 * @return 성공시 HLS_SUCCESS, 실패시 HLS error code 참고 
 */
int hls_gen_rand(hls_context_t *ctx);

//generate kdf, authdata, otherinfo
/**
 * @brief Z 키를 생성한다.
 * 
 * @param[in] info              otherinfo 정보
 * @param[in] info_len          otherinfo 정보 길이
 * @param[in] key               Z키 
 * @param[in] key_len           Z키 길이
 * @param[out] out               KDF 키
 * @param[out] out_len           KDF 키 길이
 * @return 성공시 HLS_SUCCESS, 실패시 HLS error code 참고 
 */
int hls_key_derivation(unsigned char *info, int info_len, unsigned char *key, int key_len, unsigned char *out, int *out_len);

/**
 * @brief ECDSA 인증을 위한 데이터를 생성한다.
 * 
 * @param[in] ctx               hls_context_t 포인터
 * @param[out] auth              인증데이터
 * @param[in] is_server         서버이면 1, 아니면 0
 * @return 성공시 HLS_SUCCESS, 실패시 HLS error code 참고 
 */
int hls_make_authentication_data(hls_context_t *ctx, unsigned char auth[HLS_ECDSA_AUTHENTICATION_DATA_LEN], int is_server);

/**
 * @brief KDF생성에 필요한 other info 정보를 생성한다.
 * 
 * @param[in] ctx               hls_context_t 포인터
 * @param[out] out               other info 정보
 * @param[out] out_len           ohter info 정보 길이.
 * @return 성공시 HLS_SUCCESS, 실패시 HLS error code 참고 
 */
int hls_make_other_info_data(hls_context_t *ctx, unsigned char *out, int *out_len);

// mutual authenticate
/**
 * @brief 상호인증을 위한 전자서명 정보을 생성한다.
 * 
 * @param[in] ctx               hls_context_t 포인터
 * @param[out] signature         전자서명
 * @param[out] signature_len     전자서명 길이
 * @return 성공시 HLS_SUCCESS, 실패시 HLS error code 참고  
 */
int hls_sign_mutual_auth(hls_context_t *ctx, unsigned char *signature, int *signature_len);

/**
 * @brief 상호인증을 위한 전자서명 검증을 한다.
 * 
 * @param[in] ctx               hls_context_t 포인터
 * @param[in] signature         전자서명
 * @param[in] signature_len     전자서명 길이
 * @return 성공시 HLS_SUCCESS, 실패시 HLS error code 참고  
 */
int hls_verify_mutual_auth(hls_context_t *ctx, unsigned char *signature, int signature_len);

// key agreement
/**
 * @brief 키교환을 위한 전자서명 정보를 생성한다.
 * 
 * @param[in] ctx               hls_context_t 포인터
 * @param[out] signature         전자서명
 * @param[out] signature_len     전자서명 길이
 * @return 성공시 HLS_SUCCESS, 실패시 HLS error code 참고  
 */
int hls_sign_key_agreement(hls_context_t *ctx, unsigned char *signature, int *signature_len);

/**
 * @brief 키교환을 위한 전자서명 검증을 한다.
 * 
 * @param[in] ctx               hls_context_t 포인터
 * @param[in] signature         전자서명
 * @param[in] signature_len     전자서명 길이
 * @return 성공시 HLS_SUCCESS, 실패시 HLS error code 참고  
 */
int hls_verify_key_agreement(hls_context_t *ctx, unsigned char *signature, int signature_len);

// encrypt and decrypt message
/**
 * @brief 평문메시지를 암호화 한다.
 * 
 * @param[in] ctx               hls_context_t 포인터
 * @param[in] IC                Invocation Counter
 * @param[in] in                평문메시지
 * @param[in] in_len            평문메시지 길이
 * @param[out] out               암호메시지
 * @param[out] out_len           암호메시지 길이
 * @return 성공시 HLS_SUCCESS, 실패시 HLS error code 참고  
 */
int hls_encrypt_message(hls_context_t *ctx, unsigned char IC[4], unsigned char *in, int in_len, unsigned char *out, int *out_len);

/**
 * @brief 암호메시지를 복호화 한다.
 * 
 * @param[in] ctx               hls_context_t 포인터
 * @param[in] IC                Invocation Counter
 * @param[in] SC                Security Control Byte
 * @param[in] in                암호메시지
 * @param[in] in_len            암호메시지 길이
 * @param[out] out               평문메시지
 * @param[out] out_len           평문메시지 길이
 * @return 성공시 HLS_SUCCESS, 실패시 HLS error code 참고  
 */
int hls_decrypt_message(hls_context_t *ctx, unsigned char IC[4], unsigned char SC, unsigned char *in, int in_len, unsigned char *out, int *out_len);

/**
 * @brief 암호화시 암호메시지의 길이를 반환한다.
 * 
 * @param[in] SC                Security Control Byte
 * @param[in] msg_len           평문 메시지 길이.
 * @return 성공시 HLS_SUCCESS, 실패시 HLS error code 참고  
 */
int hls_get_encrypt_len(unsigned char SC, int msg_len);

/**
 * @brief 복호화시 평문메시지의 길이를 반환한다.
 * 
 * @param[in] SC                Security Control Byte
 * @param[in] msg_len           암호메시지 길이
 * @return 성공시 HLS_SUCCESS, 실패시 HLS error code 참고  
 */
int hls_get_decrypt_len(unsigned char SC, int msg_len);

/**
 * @brief 인증서로부터 공개키를 추출한다.
 * 
 * @param[in] cert              인증서
 * @param[in] cert_len          인증서 길이
 * @param[out] pubkey            공개키 
 * @param[out] pubkey_len        공개키 길이
 * @return 성공시 HLS_SUCCESS, 실패시 HLS error code 참고  
 */
int hls_get_pubkey_from_cert(uint8_t *cert, int cert_len, uint8_t *pubkey, int *pubkey_len);

/**
 * @brief 현재 설정된 HLS 정보에서 Security Control Byte 정보를 반환한다.
 * 
 * @param[in] ctx               hls_context_t 포인터
 * @return unsigned char Security Control Byte
 */
unsigned char hls_get_sc(hls_context_t *ctx);

/**
 * @brief HLS 정보를 출력한다(개발용)
 * 
 * @param[in] ctx               hls_context_t 포인터
 * @return 성공시 HLS_SUCCESS, 실패시 HLS error code 참고  
 */
int hls_dump_context(hls_context_t *ctx);


#endif   /* __DLMS_HLS_ECDSA_H__ */
/**
 * @}
 */