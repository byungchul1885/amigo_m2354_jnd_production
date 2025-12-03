#if 0 /* bccho, UNUSED, 2023-07-15 */
#if !defined(__AMG_PARTITION_MEMORY_H__)
#define __AMG_PARTITION_MEMORY_H__

#if defined (FEATURE_USE_MEMORY_POOL)

/*
******************************************************************************
*	INCLUDE
******************************************************************************
*/

/*
******************************************************************************
*	Definition
******************************************************************************
*/

/*
******************************************************************************
*	MACRO
******************************************************************************
*/

/*
******************************************************************************
*	DATA TYPE
******************************************************************************
*/

/*
******************************************************************************
*	GLOBAL VARIABLE
******************************************************************************
*/

/*
******************************************************************************
*	FUNCTIONS
******************************************************************************
*/

extern void   partition_memory_initialize(void);
extern INT32  partition_memory_allocate( UINT16 size, void ** pp_buffer);
extern INT32  partition_memory_free( UINT16 size, void * p_buffer);
extern INT32  partition_memory_find( void * p_buffer);

extern void   partition_memory_dump( UINT32 from_dm);
extern void   partition_memory_trace( UINT32 from_dm);

extern UINT32 dsm_mac_find(void *ptr);
extern void   dsm_mac_free(void *ptr);
extern void  *dsm_mac_malloc(UINT32 size);
extern void  *dsm_mac_realloc(void *ptr, UINT32 size);
extern void  *dsm_mac_zalloc(UINT32 size);

extern void   dsm_mem_free(void **ptr, UINT32 *size);
extern UINT32 dsm_mem_malloc(void **ptr, UINT32 size, void *buff, UINT32 len);

extern UINT32 dsm_mac_partition_memory_usage(UINT32 byte_count);
extern UINT32 dsm_mac_partition_memory_is_malloc_range( UINT32 address);
extern UINT32 dsm_mac_partition_memory_get_free_size (void);
#endif /* FEATURE_USE_MEMORY_POOL */

#endif /*__AMG_PARTITION_MEMORY_H__*/

#endif /* bccho */