/**
 * @file HLS_error.h
 */


/** @defgroup hls
 * @brief DLMS HLS library
 * @{
 */

#ifndef __HLS_ERROR_H__
#define __HLS_ERROR_H__

#define HLS_SUCCESS                             0       /**< 성공 */ 
#define HLS_FAILED                              -1      /**< 실패 */ 

#define HLS_ERR_MEMORY_ALLOC                0x00000101  /**< 메모리 할당 오류 */
#define HLS_ERR_INPUT_PARAMETER             0x00000102  /**< 파라미터 오류*/
#define HLS_ERR_NOT_SUPPORT                 0x00000103  /**< 해당 기능을 지원하지 않음 */

#define HLS_NOT_FOUND_FILE                  0x00000200  /**<  파일찾기 오류 */
#define HLS_NOT_FOUND_SYSTEM_TITLE          0x00000201  /**<  시스템 타이틀 없음*/
#define HLS_NOT_FOUND_CERT                  0x00000202  /**<  인증서 없음*/
#define HLS_NOT_FOUND_CtoS                  0x00000203  /**<  CtoS 없음*/
#define HLS_NOT_FOUND_PUBKEY                0x00000204  /**<  공개키 없음*/
#define HLS_NOT_FOUND_PRIKEY                0x00000205  /**<  개인키 없음*/
#define HLS_NOT_FOUND_ECC_PUBKEY            0x00000206  /**<  ECC 공개키 없음*/
#define HLS_NOT_FOUND_ECC_PRIKEY            0x00000207  /**<  ECC 개인키 없음*/
#define HLS_NOT_FOUND_SHARE_KEY             0x00000208  /**<  공유키 없음*/

#define HLS_ERR_CERT_PARSE                  0x00000301  /**<  인증서 파싱 오류*/
#define HLS_ERR_INVALID_COMMON_NAME         0x00000302  /**<  COMMON NAME 오류*/
#define HLS_NOT_FOUND_SUJECT_COMMON_NAME    0x00000303  /**<  SUJECT COMMON NAME 없음*/
#define HLS_ERR_GEN_RANDOM                  0x00000304  /**<  난수정보 생성 오류*/
#define HLS_ERR_HLS_AUTH_DATA               0x00000305  /**<  인증 정보 오류*/
#define HLS_ERR_HLS_AUTH_SERVER_DATA        0x00000306  /**<  인증 정보(서버) 오류*/
#define HLS_ERR_ECDSA_SIGN                  0x00000307  /**<  ECDSA 전자서명 오류*/
#define HLS_ERR_ECDSA_VERIFY                0x00000308  /**<  ECDSA 전자서명 검증 오류*/
#define HLS_ERR_SHA256                      0x00000309  /**<  SHA256 오류*/
#define HLS_NOT_SUPPORT_EC_KEY              0x0000030A  /**<  지원하지 않는 ECC KEY*/
#define HLS_ERR_ECP_GROUP_LOAD              0x0000030B  /**<  ECC 키 로드 오류*/
#define HLS_ERR_ECDH_GEN                    0x0000030C  /**<  ECDH용 키 생성 오류*/
#define HLS_ERR_WRITE_ECDH_PUBLIC_KEY       0x0000030D  /**<  ECDH 공개키 저장 오류*/
#define HLS_ERR_WRITE_ECDH_PRIVATE_KEY      0x0000030E  /**<  ECDH 개인키 저장 오류*/
#define HLS_ERR_READ_ECDH_PUBLIC_KEY        0x0000030F  /**<  ECDH 공개키 읽기 오류*/
#define HLS_ERR_READ_ECDH_PRIVATE_KEY       0x00000310  /**<  ECDH 개인키 읽기 오류*/
#define HLS_ERR_ECDH_COMPUTE_SHARED         0x00000311  /**<  ECDH 공유키 생성 오류*/
#define HLS_ERR_SET_ECDH_SHARED_KEY         0x00000312  /**<  ECDH 공유키 설정 오류*/
#define HLS_ERR_HMAC_SETUP                  0x00000313  /**<  HMAC 오류*/
#define HLS_ERR_HMAC_STARTS                 0x00000314  /**<  HMAC 오류*/
#define HLS_ERR_HMAC_UPDATE                 0x00000315  /**<  HMAC 오류 */
#define HLS_ERR_HMAC_FINISH                 0x00000316  /**<  HMAC 오류*/
#define HLS_ERR_NOT_SUPPORT_ALG             0x00000317  /**<  미지원 알고리즘*/
#define HLS_ERR_NOT_SUPPORT_CRYPT_AUTH      0x00000318  /**<  미지원 암호 인증*/
#define HLS_ERR_MAKE_OTHER_INFO             0x00000319  /**<  OTHER INFO 생성 오류*/
#define HLS_ERR_MAKE_OTHER_INFO_LEN         0x0000031A  /**<  OTHER INFO 길이 오류*/
#define HLS_ERR_CRYPT_TAG_BYTE              0x0000031B  /**<  암호 TAG 오류*/
#define HLS_ERR_GCM_SET_KEY                 0x0000031C  /**<  GCM키 설정 오류*/
#define HLS_ERR_GCM_CRYPT_AND_TAG           0x0000031D  /**<  GCM 암호화 AUTH TAG 오류*/
#define HLS_ERR_GCM_CRYPT                   0x0000031E  /**<  GCM 암호화 오류*/
#define HLS_ERR_GCM_AUTH                    0x0000031F  /**<  GCM AUTH 오류*/
#define HLS_ERR_AUTH_DATA                   0x00000320  /**<  AUTH DATA 오류*/
#define HLS_ERR_GET_ASN1_TAG                0x00000321  /**<  ASN1 TAG 파싱 오류*/
#define HLS_ERR_MISMATCH_ASN1_SIGN_LENGTH   0x00000322  /**<  ASN1 SIGN 길이 오류*/
#define HLS_ERR_GET_MPI                     0x00000323  /**<  MPI 오류*/
#define HLS_ERR_MISMATCH_ECDSA_SIGN_LEN     0x00000324  /**<  ECDSA 전자서명 길이 오류*/
#define HLS_ERR_CONVERT_SIGN_ASN1_TO_BINARY 0x00000325  /**<  ASN1 변환 오류*/
#define HLS_ERR_CONVERT_BINARY_TO_SIGN_ASN1 0x00000326  /**<  ASN1 변환 오류*/
#define HLS_ERR_WRITE_ECDSA_ECC_PUBKEY      0x00000327  /**<  ECDSA 공개키 저장 오류*/
#define HLS_ERR_GCM_STARTS                  0x00000328  /**<  GCM 오류*/
#define HLS_ERR_GCM_UPDATE                  0x00000329  /**<  GCM UPDATE 오류*/
#define HLS_ERR_SET_KEY                     0x0000032A  /**<  키 설정 오류*/
#define HLS_ERR_GET_KEY                     0x0000032B  /**<  키 반환 오류*/

#define HLS_LICENSE_EXPIRATION              0x00000901    /**<  라이센스 만료*/
#define HLS_LICENSE_NOT_SUPPORT_CERT        0x00000902    /**<  미지원 인증서 라이센스*/

#endif   /* __HLS_ERROR_H__ */
/**
 * @}
 */