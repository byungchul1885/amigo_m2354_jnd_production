/*
******************************************************************************
*    INCLUDE
******************************************************************************
*/
#include "main.h"
#include "os_list.h"

/*
******************************************************************************
*    Definition
******************************************************************************
*/
#define _D  "[OSL] "

/*
******************************************************************************
*    GLOBAL FUNCTION
******************************************************************************
*/

void os_list_init (OS_LIST *p_list, const char *p_name)
{
	p_list->list.head = NULL;
    p_list->list.tail = NULL;
	p_list->no_of_items = 0;
    p_list->p_name = p_name;
}


bool os_list_is_empty (OS_LIST *p_list)
{
    return p_list->list.head == NULL;
}

LIST_ITEM *os_list_get_head (OS_LIST *p_list)
{
    return p_list->list.head;
}

LIST_ITEM *os_list_get_tail (OS_LIST *p_list)
{
    return p_list->list.tail;
}

LIST_ITEM *os_list_get_head_next (OS_LIST *p_list)
{
    ASSERT (p_list->list.head->next);
    return ((LIST_ITEM *)p_list->list.head->next);
}

LIST_ITEM *os_list_get_tail_prev (OS_LIST *p_list)
{
    ASSERT (p_list->list.tail->next);
    return ((LIST_ITEM *)p_list->list.tail->next);
}

int os_list_get_no_of_items (OS_LIST *p_list)
{
    return p_list->no_of_items;
}

void os_list_insert_front (OS_LIST *p_list, LIST_ITEM *p_entry, LIST_ITEM *p_new_entry)
{
    p_list->no_of_items++;

	if (p_entry->prev != NULL)
	{
		p_entry->prev->next = p_new_entry;
	}
	else
	{
		p_list->list.head = p_new_entry;
	}

	p_new_entry->prev = p_entry->prev;

	p_new_entry->next = p_entry;
	p_entry->prev = p_new_entry;
}

void os_list_insert_head (OS_LIST *p_list, LIST_ITEM *p_entry)
{
	ASSERT(p_list->list.head != p_entry);

    p_list->no_of_items++;

    p_entry->prev = NULL;

	if (p_list->list.head != NULL)
	{
	    p_entry->next = p_list->list.head;
		p_list->list.head->prev = p_entry;
	}
	else
	{
		p_entry->next = NULL;
		p_list->list.tail = p_entry;
	}

	p_list->list.head = p_entry;

	if( p_list->list.tail != NULL )
	{
		ASSERT(p_list->list.tail->next == NULL);
	}
}

void os_list_insert_tail (OS_LIST *p_list, LIST_ITEM *p_entry)
{
	ASSERT(p_list->list.tail != p_entry);

    p_list->no_of_items++;

	p_entry->next = NULL;

	if (p_list->list.tail != NULL)
	{
	    p_entry->prev = p_list->list.tail;
		p_list->list.tail->next = p_entry;
	}
	else
	{
		p_entry->prev = NULL;
		p_list->list.head = p_entry;
	}

	p_list->list.tail = p_entry;

	if (p_list->list.tail != NULL )
	{
		ASSERT(p_list->list.tail->next == NULL);
	}
 }


LIST_ITEM *os_list_remove_head (OS_LIST *p_list)
{
	LIST_ITEM *p_entry;

	if( p_list->list.tail != NULL )
	{
		ASSERT(p_list->list.tail->next == NULL);
	}

    if (p_list->no_of_items == 0)
    {
        return NULL;
    }

    p_list->no_of_items--;

    p_entry = p_list->list.head;

	if (p_list->list.head)
	{
 		p_list->list.head = p_list->list.head->next;

		if (p_list->list.head == NULL)
		{
			p_list->list.tail = NULL;
            ASSERT(p_list->no_of_items==0);
		}
		else
		{
			p_list->list.head->prev = NULL;
		}

        p_entry->prev = NULL;
        p_entry->next = NULL;
	}

    return p_entry;
}

LIST_ITEM *os_list_remove_tail (OS_LIST *p_list)
{
	LIST_ITEM *p_entry;

    p_entry = p_list->list.tail;

    if (p_list->no_of_items == 0)
    {
        ASSERT (0);
    }

    p_list->no_of_items--;

    if(p_list->list.tail)
    {
    	p_list->list.tail = p_list->list.tail->prev;

    	if (p_list->list.tail == NULL)
    	{
    		p_list->list.head = NULL;
            ASSERT(p_list->no_of_items==0);
    	}
        else
        {
            p_list->list.tail->next = NULL;
        }
    }

	return p_entry;
}

void os_list_remove_entry (OS_LIST * p_list, LIST_ITEM *p_entry)
{
	PLIST_ITEM  prev;
	PLIST_ITEM	next;

    if (p_list->no_of_items == 0)
    {
        ASSERT (0);
    }

    p_list->no_of_items--;

	prev = p_entry->prev;
	next = p_entry->next;

	if (prev == NULL)
	{
		p_list->list.head = next;
	}
	else
	{
		prev->next = next;
	}

	if (next == NULL)
	{
		p_list->list.tail = prev;
	}
	else
	{
		next->prev	= prev;
	}
}

LIST_ITEM *os_list_is_item_exist (OS_LIST *p_list, LIST_ITEM *p_item)
{
    LIST        *p_lst = &p_list->list;
    LIST_ITEM   *p_i;

    p_i = p_lst->head;
    while (p_i)
    {
        if (p_item == p_i)
        {
            return p_i;
        }
        p_i = p_i->next;
    }

    return NULL;
}


