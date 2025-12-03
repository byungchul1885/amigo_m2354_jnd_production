#ifndef __OS_MEMORY_H__
#define __OS_MEMORY_H__
/*
******************************************************************************
*    INCLUDE
******************************************************************************
*/
/*
******************************************************************************
*    DATA TYPE
******************************************************************************
*/
#define FEATURE_OS_MEM_DEBUG_ENABLE     1

/*
******************************************************************************
*    LOCAL VARIABLES
******************************************************************************
*/

/*
******************************************************************************
*    MACRO
******************************************************************************
*/
#if FEATURE_OS_MEM_DEBUG_ENABLE == 0
#define os_mem_init()
#define os_mem_close()
#ifdef FreeRTOS
#define os_mem_alloc(size, p_tag_name)  pvPortMalloc(size)
#define os_mem_free(p_mem)              vPortFree(p_mem)
#else
#define os_mem_alloc(size, p_tag_name)  malloc(size)
#define os_mem_free(p_mem)              free(p_mem)
#endif

#define os_mem_get_totoal_alloc_size()  1
#define os_mem_disp_list_info()         DPRINTF (DBG_ERR, "No Memory Debug Mode!\r\n")
#endif

#define os_memset(src, val, len)        memset(src, val, len)
#define os_memcpy(src, dst, len)        memcpy(src, dst, len)
#define os_memcmp(src, dst, len)        memcmp(src, dst, len)

/*
******************************************************************************
*    GLOBAL FUNCTIONS
******************************************************************************
*/

#if FEATURE_OS_MEM_DEBUG_ENABLE == 1

#ifndef __OS__
#define malloc      "must use os_mem_alloc instead of malloc"
#define free        "must use os_mem_free instead of free"
#endif
void   os_mem_init (void);
void   os_mem_close (void);
void  *os_mem_alloc (UINT32 size, const char *p_tag_name);
void  *os_mem_zalloc (UINT32 size, const char *p_tag_name);
void   os_mem_free (void *p_mem);
UINT32 os_mem_get_totoal_alloc_size (void);
void   os_mem_disp_list_info (void);
#endif

#endif

