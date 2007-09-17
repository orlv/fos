#define FOS 1
#define JAR 1

#if FOS && JAR

#include "jar.h"
#include "alloc.h"
#include <string.h>
#include "zlib/zlib.h"

#define HASHTABSZE 1<<8

#define SIG_LEN     4

/* zlib window size */
#define MAX_WINDOW_BITS 15

/* End of central directory record */
#define END_CEN_SIG                0x06054b50
#define END_CEN_LEN                22
#define END_CEN_ENTRIES_OFFSET     8
#define END_CEN_DIR_START_OFFSET   16

/* Central directory file header */
#define CEN_FILE_HEADER_SIG        0x02014b50
#define CEN_FILE_HEADER_LEN        46
#define CEN_FILE_COMPMETH_OFFSET   10
#define CEN_FILE_COMPLEN_OFFSET    20
#define CEN_FILE_UNCOMPLEN_OFFSET  24
#define CEN_FILE_PATHLEN_OFFSET    28
#define CEN_FILE_EXTRALEN_OFFSET   30
#define CEN_FILE_COMMENTLEN_OFFSET 32
#define CEN_FILE_LOCALHDR_OFFSET   42

/* Local file header */
#define LOC_FILE_HEADER_SIG        0x04034b50
#define LOC_FILE_HEADER_LEN        30
#define LOC_FILE_EXTRA_OFFSET      28

/* Supported compression methods */
#define COMP_STORED                0
#define COMP_DEFLATED              8

/* Macros for reading non-aligned little-endian values from zip file */
#define READ_LE_INT(p)      ((p)[0]|((p)[1]<<8)|((p)[2]<<16)|((p)[3]<<24))
#define READ_LE_SHORT(p)    ((p)[0]|((p)[1]<<8))

ZipFile_t * zip_process(unsigned char *data, unsigned int len) {
	#if VERBOSE
	printf("zip_process, len = %i\n", len);
	#endif
	
	/*
	unsigned char *magic;
	unsigned char *pntr;
	
	int entries, next_file_sig;
	hash_table_t *hash_table;
	ZipFile_t *zip;
	
	magic = data;
	if (READ_LE_INT(magic) != LOC_FILE_HEADER_SIG) {
		#if VERBOSE
		printf("READ_LE_INT(magic) != LOC_FILE_HEADER_SIG\n");
		printf("magic = 0x%x, LOC_FILE_HEADED_SIG = 0x%x\n", magic, LOC_FILE_HEADER_SIG);
		#endif
		return NULL;
	}
	
	if (len < END_CEN_LEN) {
		#if VERBOSE
		printf("len < END_CEN_LEN\n");
		#endif
		return NULL;
	}
	
	#if VERBOSE
	printf("loop1\n");
	#endif
	
	for (pntr = data + len - END_CEN_LEN; pntr >= data; ) {
		if (*pntr == (END_CEN_SIG & 0xFF)) {
			if (READ_LE_INT(pntr) == END_CEN_SIG) break;
			else pntr -= SIG_LEN;
		} else {
			pntr--;
		}
	}
	
	if (pntr < data) {
		#if VERBOSE
		printf("pntr < data\n");
		#endif
		//return NULL;
	}
	
	entries = READ_LE_SHORT(pntr + END_CEN_ENTRIES_OFFSET);
	
	hash_table = hash_table_create(HASHTABSZE);
	
	pntr = data + READ_LE_INT(pntr + END_CEN_DIR_START_OFFSET);
	
	if ((pntr + SIG_LEN) > (data + len)) {
		#if VERBOSE
		printf("(pntr + SIG_LEN) > (data + len)\n");
		#endif
		goto error2;
	}
	
	next_file_sig = READ_LE_INT(pntr);
	
	#if VERBOSE
	printf("loop2\n");
	#endif
	
	while (entries--) {
		char *found, *pathname, *term;
		int path_len, comment_len, extra_len;
		
		if ((pntr + CEN_FILE_HEADER_LEN) > (data + len)) {
			#if VERBOSE
			printf("(pntr + CEN_FILE_HEADER_LEN) > (data + len)\n");
			#endif
			goto error2;
		}
		
		if (next_file_sig != CEN_FILE_HEADER_SIG) {
			goto error2;
		}
		
		path_len = READ_LE_SHORT(pntr + CEN_FILE_PATHLEN_OFFSET);
		extra_len = READ_LE_SHORT(pntr + CEN_FILE_EXTRALEN_OFFSET);
		comment_len = READ_LE_SHORT(pntr + CEN_FILE_COMMENTLEN_OFFSET);
		
		pathname = (char*)(pntr += CEN_FILE_HEADER_LEN);
		term = (char*)(pntr += path_len);
		
		pntr += extra_len + comment_len;
		
		if ((pntr + SIG_LEN) > (data + len)) {
			goto error2;
		}
		
		next_file_sig = READ_LE_INT(pntr);
		*term = '\0';
		
		hash_entry_t *entry = hash_table_find(hash_table, utf8_hash(pathname));
		if (!entry) {
			hash_table_add(hash_table, utf8_hash(pathname), pathname);
		}
	}
	
	zip = (ZipFile_t*)jvm_malloc(sizeof(ZipFile_t));
	zip->data = data;
	zip->length = len;
	zip->dir = hash_table;
	
	return zip;
	
	error2:
	hash_table_free(hash_table);
	
	return NULL;
	*/
	
	unsigned char *magic;
    unsigned char *pntr;
    int entries, next_file_sig;
    hash_table_t *hash_table;
    ZipFile_t *zip;

    /* First 4 bytes must be a signature for the first local file header */
    magic = data;
    if ( READ_LE_INT(magic) != LOC_FILE_HEADER_SIG ) {
    	printf("READ_LE_INT(magic) != LOC_FILE_HEADER_SIG\n");
        return NULL;
    }

    /* Locate the end of central directory record by searching backwards for
       the record signature. */
    if ( len < END_CEN_LEN ) {
    	printf("len < END_CEN_LEN\n");
        return NULL;
    }

    for ( pntr = data + len - END_CEN_LEN; pntr >= data; )
        if ( *pntr == (END_CEN_SIG & 0xff) )
            if ( READ_LE_INT(pntr) == END_CEN_SIG )
                break;
            else
                pntr -= SIG_LEN;
        else
            pntr--;

    /* Check that we found it */
    if (pntr < data ) {
    	printf("pntr < data\n");
        return NULL;
    }

	entries = READ_LE_SHORT(pntr + END_CEN_ENTRIES_OFFSET);

    /* Create and initialise hash table to hold the directory.
       Do not create lock -- we're single threaded (bootstrapping)
       and once entries are added, table is read-only */
    hash_table = hash_table_create(HASHTABSZE);

    /* Get the offset from the start of the file of the first directory entry */
    pntr = data + READ_LE_INT(pntr + END_CEN_DIR_START_OFFSET);

    /* Scan the directory list and add the entries to the hash table */

    /* We terminate each pathname "in place" to save memory -- this may
       over-write the signature for the next entry.  We therefore
       speculatively load the next sig *before* we terminate the pathname */

    if ( (pntr + SIG_LEN) > (data + len) )
        goto error2;

    /* Speculatively read first sig */
    next_file_sig = READ_LE_INT(pntr);

    while ( entries-- ) {
        char *found, *pathname, *term;
        int path_len, comment_len, extra_len;

        if ( (pntr + CEN_FILE_HEADER_LEN) > (data + len) )
            goto error2;

        /* Check directory entry signature is present */
        if ( next_file_sig != CEN_FILE_HEADER_SIG )
            goto error2;

        /* Get the length of the pathname */
        path_len = READ_LE_SHORT(pntr + CEN_FILE_PATHLEN_OFFSET);

        /* Not interested in these fields but need length to skip */
        extra_len = READ_LE_SHORT(pntr + CEN_FILE_EXTRALEN_OFFSET);
        comment_len = READ_LE_SHORT(pntr + CEN_FILE_COMMENTLEN_OFFSET);

        /* The pathname starts after the fixed part of the dir entry */
        pathname = (char*)(pntr += CEN_FILE_HEADER_LEN);
	
		term = (char*)(pntr += path_len);

        /* Skip rest of variable fields -- should then be pointing to next
           sig.  If no extra or comment fields, pntr == term */
        pntr += extra_len + comment_len;

        /* Even if this is the last entry, a well-formed zip file will *always*
           have at least 4 extra bytes */
        if ( (pntr + SIG_LEN) > (data + len) )
            goto error2;

        /* Speculatively read next sig */
        next_file_sig = READ_LE_INT(pntr);

        /* Terminate the pathname */
        *term = '\0';

        /* Add if absent, no scavenge, not locked */
        hash_entry_t *ent = hash_table_find(hash_table, utf8_hash(pathname));
        if ( NULL == ent )
            hash_table_add(hash_table, utf8_hash(pathname), pathname);
    }

    zip = (ZipFile_t*) jvm_malloc(sizeof(ZipFile_t));

    zip->data = data;
    zip->length = len;
    zip->dir = hash_table;

    return zip;

error2:
    hash_table_free(hash_table);

error1:
    return NULL;
}

char * zip_find_dir_entry(char *pathname, ZipFile_t *zip) {
	hash_entry_t *entry = hash_table_find(zip->dir, utf8_hash(pathname));
	
	if (entry) {
		return entry->data;
	}
	
	return NULL;
}

char * zip_find_entry(char *pathname, ZipFile_t *zip, int *uncomp_len) {
	int offset, path_len, comp_len, extra_len, comp_method;
    unsigned char *decomp_buff, *comp_data;
    unsigned char *dir_entry;

    if((dir_entry = (unsigned char *)zip_find_dir_entry(pathname, zip)) == NULL)
        return NULL;

    /* Found the file -- the pathname points directly into the
       directory entry.  Read the values relative to it */

    /* Offset of the file entry relative to the start of the file */
    offset = READ_LE_INT(dir_entry + (CEN_FILE_LOCALHDR_OFFSET -
                                      CEN_FILE_HEADER_LEN));

    if((offset + LOC_FILE_HEADER_LEN) > zip->length)
        return NULL;
	
	
	/* Get the variable length part of the local file header */
    extra_len = READ_LE_SHORT(zip->data + offset + LOC_FILE_EXTRA_OFFSET);

    /* Get the path_len again -- faster than doing a strlen */
    path_len = READ_LE_SHORT(dir_entry + (CEN_FILE_PATHLEN_OFFSET - CEN_FILE_HEADER_LEN));

    /* The file's length when uncompressed -- this is passed out */
    *uncomp_len = READ_LE_INT(dir_entry + (CEN_FILE_UNCOMPLEN_OFFSET - CEN_FILE_HEADER_LEN));

    /* The compressed file's length, i.e. the data size in the file */
    comp_len = READ_LE_INT(dir_entry + (CEN_FILE_COMPLEN_OFFSET - CEN_FILE_HEADER_LEN));

    /* How the file is compressed */
    comp_method = READ_LE_SHORT(dir_entry + (CEN_FILE_COMPMETH_OFFSET - CEN_FILE_HEADER_LEN));

    /* Calculate the data start */
    offset += LOC_FILE_HEADER_LEN + path_len + extra_len;

    /* Make sure we're not reading outside the file */
    if((offset + comp_len) > zip->length)
        return NULL;

    comp_data = zip->data + offset;
    decomp_buff = jvm_malloc(*uncomp_len);
    
        switch(comp_method) {
        case COMP_STORED:
            /* Data isn't compressed, so just return it "as is" */
            memcpy(decomp_buff, comp_data, comp_len);
            return (char*)decomp_buff;

        case COMP_DEFLATED: {
            z_stream stream;
            int err;

            stream.next_in = comp_data;
            stream.avail_in = comp_len;
            stream.next_out = decomp_buff;
            stream.avail_out = *uncomp_len;

            stream.zalloc = Z_NULL;
            stream.zfree = Z_NULL;

            /* Use a negative windowBits value to stop the inflator looking
               for a header */
            if(inflateInit2(&stream, -MAX_WINDOW_BITS) != Z_OK)
                goto error;

            err = inflate(&stream, Z_SYNC_FLUSH);
            inflateEnd(&stream);

            if(err == Z_STREAM_END || (err == Z_OK && stream.avail_in == 0))
                return (char*)decomp_buff;
            break;
        }

        default:
            break;
    }

error:
    jvm_free(decomp_buff);
    return NULL;
    
}

#endif
