#ifndef _PCI_DEF_H
#define _PCI_DEF_H 

/**
 * Macros definitions
 */

#define DEV_INFO(name) { name, 0x00 }

#define MAP_ITEM_DEF(KEY_TYPE, VALUE_TYPE, MAP_ITEM_NAME) \
typedef struct{ \
	KEY_TYPE key; \
	VALUE_TYPE value; \
} MAP_ITEM_NAME;

#define VARIABLE_NAME(NAME,PREFIX) PREFIX##NAME
#define VARIABLE_DEF(NAME,TYPE,PREFIX) static TYPE PREFIX##NAME = 

#define CLASSES_DEF(NAME,TYPE) VARIABLE_DEF(NAME,TYPE,classes_)
#define CLASSES_VAR_NAME(NAME) VARIABLE_NAME(NAME,classes_)

#define SEARCH_VALUE_BY_KEY(VAR, KEY, LAST_KEY) while( VAR->key != LAST_KEY && VAR->key != KEY) VAR++;

#define CLASSES_LAST_KEY 0xFFFF

#define CLASSES_SEARCH_VALUE_BY_KEY(VAR, KEY) SEARCH_VALUE_BY_KEY(VAR, KEY, CLASSES_LAST_KEY)

#define APPROP_BIT(offset)	1 << offset
#define APPROP_MASK(mask,offset) mask << offset

#endif
