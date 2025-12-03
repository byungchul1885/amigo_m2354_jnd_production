#ifndef __OS_LIST_H__
#define __OS_LIST_H__
/*
******************************************************************************
*    INCLUDE
******************************************************************************
*/

/*
******************************************************************************
*    Definition
******************************************************************************
*/

/*
******************************************************************************
*    MACRO
******************************************************************************
*/

/*
******************************************************************************
*    DATA TYPE
******************************************************************************
*/
typedef struct _LIST_ITEM
{
	struct _LIST_ITEM   *prev;
	struct _LIST_ITEM   *next;
}
LIST_ITEM, *PLIST_ITEM;

typedef struct _LIST_TAG
{
	PLIST_ITEM      head;
	PLIST_ITEM      tail;
}
LIST, *PLIST;

typedef struct
{
    LIST_ITEM       link;
    LIST            list;
    uint32_t          no_of_items;
    const char      *p_name;
}
OS_LIST;

/*
******************************************************************************
*    GLOBAL VARIABLE
******************************************************************************
*/

/*
******************************************************************************
*    GLOBAL FUNCTION
******************************************************************************
*/

void os_list_init (OS_LIST *p_list, const char *p_name);
void os_list_close (OS_LIST *p_list);
void os_list_lock (OS_LIST *p_list);
void os_list_unlock (OS_LIST *p_list);
bool os_list_is_empty (OS_LIST *p_list);
LIST_ITEM *os_list_is_item_exist (OS_LIST *p_list, LIST_ITEM *p_item);
LIST_ITEM *os_list_get_head (OS_LIST *p_list);
LIST_ITEM *os_list_get_tail (OS_LIST *p_list);
LIST_ITEM *os_list_get_head_next (OS_LIST *p_list);
LIST_ITEM *os_list_get_tail_prev (OS_LIST *p_list);
void os_list_insert_front (OS_LIST *p_list, LIST_ITEM *p_entry, LIST_ITEM *p_new_entry);
int os_list_get_no_of_items (OS_LIST *p_list);
void os_list_insert_head (OS_LIST *p_list, LIST_ITEM *p_entry);
void os_list_insert_tail (OS_LIST *p_list, LIST_ITEM *p_entry);
LIST_ITEM *os_list_remove_head (OS_LIST *p_list);
LIST_ITEM *os_list_remove_tail (OS_LIST *p_list);
void os_list_remove_entry (OS_LIST * p_list, LIST_ITEM *p_entry);

#endif


