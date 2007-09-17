#include "alloc.h"
#include "hash.h"

#include <string.h>
#include <stdlib.h>

hash_table_t * hash_table_create(size_t size) {
    hash_table_t *ht = (hash_table_t *)jvm_malloc(sizeof(hash_table_t));
    if (ht == NULL)
		return NULL;
	
    ht->entries = (hash_entry_t *)jvm_malloc(sizeof(hash_entry_t) * size);
    if (ht->entries == NULL) {
		jvm_free(ht);
		return NULL;
    }
    
    ht->count = 0;
    ht->size = size;
    
    return ht;
}

void hash_table_free(hash_table_t *ht) {
    if (ht == NULL)
	return;
	
    if (ht->entries == NULL)
	jvm_free(ht->entries);
	
    jvm_free(ht);
}

hash_entry_t * hash_table_find(hash_table_t *ht, hash_t hash) {
    int i;
    
    if (ht == NULL || ht->entries == NULL)
		return NULL;

    synch_lock(&(ht->mutex));
    
    for	(i = 0; i < ht->count; i++) {
		if (ht->entries[i].hash == hash) {
	    	synch_unlock(&(ht->mutex));
	
	    	return &ht->entries[i];
		}
    }
    
    synch_unlock(&(ht->mutex));
    
    return NULL;
}

void hash_table_resize(hash_table_t *ht, size_t size) {
    synch_lock(&(ht->mutex));

    hash_entry_t *new_entries = (hash_entry_t*)jvm_malloc(sizeof(hash_entry_t) * size);
    hash_entry_t *old_entries = ht->entries;
    
    memcpy(new_entries, old_entries, sizeof(hash_entry_t) * ht->size);
    
    ht->entries = new_entries;
    ht->size = size;
    
    synch_unlock(&(ht->mutex));
    
    jvm_free(old_entries);
}

hash_entry_t * hash_table_add(hash_table_t *ht, hash_t hash, void *data) {
    int i;
    
    hash_entry_t *found_entry = hash_table_find(ht, hash);
    if (found_entry != NULL)
		return found_entry;
	
    synch_lock(&(ht->mutex));
	
    for (i = 0; i < ht->count; i++) {
		if (ht->entries[i].data == HASH_DELETED) {
		    found_entry = &ht->entries[i];
		    found_entry->hash = hash;
	    	found_entry->data = data;
	    
	    	synch_unlock(&(ht->mutex));
	    
	    	return found_entry;
		}
    }
    
    if (ht->count > ht->size) {
		hash_table_resize(ht, ht->size * 2);
    }
    
    found_entry = &ht->entries[ht->count];
    found_entry->hash = hash;
    found_entry->data = data;
    
    ht->count++;
    
    synch_unlock(&(ht->mutex));
    
    return found_entry;
}

void hash_table_delete(hash_table_t *ht, hash_t hash) {
    int i;
    
    synch_lock(&(ht->mutex));
    for (i = 0; i < ht->count; i++) {
		if (ht->entries[i].hash == hash) {
	    	ht->entries[i].data = HASH_DELETED;
	    	ht->entries[i].hash = 0;
	    	ht->count--;
	    
	    	synch_unlock(&(ht->mutex));
	    
	    	return;
		}	
    }
    
    synch_unlock(&(ht->mutex));
}

#define GET_UTF8_CHAR(ptr, c)                         \
{                                                     \
    int x = *ptr++;                                   \
    if(x & 0x80) {                                    \
        int y = *ptr++;                               \
        if(x & 0x20) {                                \
            int z = *ptr++;                           \
            c = ((x&0xf)<<12)+((y&0x3f)<<6)+(z&0x3f); \
        } else                                        \
            c = ((x&0x1f)<<6)+(y&0x3f);               \
    } else                                            \
        c = x;                                        \
}


hash_t utf8_hash(char *str) {	
	hash_t hash = 0;
	
	while (*str) {
		unsigned short c;

    	int x = *str++;          
    	                         
    	if(x & 0x80) {                                    
	        int y = *str++;
	                                       
        	if(x & 0x20) {                                
            	int z = *str++;                           
            	c = ((x&0xf)<<12)+((y&0x3f)<<6)+(z&0x3f); 
        	} else                                        
            	c = ((x&0x1f)<<6)+(y&0x3f);               
    	} else c = x;                                        
    	
		hash = hash * 37 + c;
	}
	
	return hash;
}
