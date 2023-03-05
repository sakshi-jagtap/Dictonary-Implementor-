#include<stdio.h> 
#include <stdlib.h>
#include <string.h>
#include "libDict.h"

#define DEBUG
#define DEBUG_LEVEL 3

#ifdef DEBUG
# define DEBUG_PRINT(x) printf x
#else
# define DEBUG_PRINT(x) do {} while (0)
#endif

#define DICT_INIT_ROWS 1024 
#define DICT_GROW_FACTOR 2
#define ROW_INIT_ENTRIES 8
#define ROW_GROW_FACTOR 2 

#define PRIME1 77933 // a large prime
#define PRIME2 119557 // a large prime

/**
 * hash *c as a sequence of bytes mod m
 */
int dictHash(char *c, int m){
	int sum=0;
	while(*c!='\0'){
		int num = *c; 
		sum+= PRIME1*num+PRIME2*sum;
		c++;
	}
	if(sum<0)sum=-sum;
	sum = sum%m;
	return sum;
}

/**
 * Print the dictionary, 
 * level==0, dict header
 * level==1, dict header, rows headers
 * level==2, dict header, rows headers, and keys
 */
void dictPrint(Dict *d, int level){
	if(d==NULL){
		printf("\tDict==NULL\n");
		return;
	}
	printf("Dict\n");
	printf("\tnumRows=%d\n",d->numRows);
	if(level<1)return;

	for(int i=0;i<d->numRows;i++){
		printf("\tDictRow[%d]: numEntries=%d capacity=%d keys=[", i, d->rows[i].numEntries, d->rows[i].capacity);
		if(level>=2){
			for(int j=0;j<d->rows[i].numEntries;j++){
				printf("%s, ",d->rows[i].entries[j].key);
			}
		}
		printf("]\n");
	}
}

/**
 * Return the DictEntry for the given key, NULL if not found.
 * This is so we can store NULL as a value.
 */
DictEntry *dictGet(Dict *d, char *key){

    // find row
    int h = dictHash(key, d->numRows);
    DictRow *r = &d->rows[h];

    // use binary search if the number of entries 
    int left = 0, right = r->numEntries - 1, mid;

    while (left <= right) {
        mid = (left + right) / 2;
        int cmp = strncmp(r->entries[mid].key, key, 1023);

        if (cmp == 0) {
            return &r->entries[mid];
        } else if (cmp < 0) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    return NULL;

}

/**
 * Delete key from dict if its found in the dictionary
 * Returns 1 if found and deleted
 * Returns 0 otherwise
 */
int dictDel(Dict *d, char *key){

    if (d == NULL) {
        return 0;
    }
    
    if (key == NULL ) {
        return 0;
    }

	int index = dictHash(key, d->numRows);
	// find row

	DictRow *r = &d->rows[index];


	#ifdef DEBUG
	printf("dictDel(d,%s) hash=%d\n",key, index);
	dictPrint(d,DEBUG_LEVEL);
	#endif
    

	// find key in row
    for (int i = 0; i < r->numEntries; i++) {
        if (strncmp(r->entries[i].key, key, 1023) == 0 ) {

            if (r->entries[i].key != NULL) {

                free(r->entries[i].key );

                r->entries[i].key = NULL;
                r->entries[i].value = NULL;
            }
            r->numEntries--;

            for (int j = i; j < r->numEntries; j++) {
                r->entries[j] = r->entries[j+1];
            }

            return 1; 
        }
    }
	#ifdef DEBUG 
	dictPrint(d,DEBUG_LEVEL);
	#endif
	return 0;
}

/**
 * put (key, value) in Dict
 * return 1 for success and 0 for failure
 */
/**
 * put (key, value) in Dict
 * return 1 for success and 0 for failure
 */
int dictPut(Dict *d, char *key, void *value) {

    int h = dictHash(key, d->numRows);

    #ifdef DEBUG
    printf("dictPut(d,%s) hash=%d\n", key, h);
    dictPrint(d, DEBUG_LEVEL);
    #endif

    // If key is already here, just replace value
    DictEntry *exitingenty = dictGet(d, key);

    if (exitingenty != NULL ) {

        free( exitingenty-> value );

        exitingenty->value = value;
        return 1;
    }

    #ifdef DEBUG
    dictPrint(d, DEBUG_LEVEL);
    #endif

    /**
     * else we need to place (key,value) as a new entry in this row
     * if there is no space, expand the row
     */

    DictRow *r = &d->rows[h];

    if (r->numEntries == r->capacity) {

        int cap = ROW_GROW_FACTOR * r->capacity;

        DictEntry *nwenty = realloc(r->entries, cap * sizeof(DictEntry));

        if (nwenty == NULL ) {

            return 0;
        }

        r->capacity = cap;
        r->entries = nwenty;
    }

    // find the infex to insert (key, value) based on the  order of

    int i;

    if ( r->numEntries > 0 ) {
        if (r->numEntries >= 20) {
           
            //  if bigger then 20, do binary search
            int left = 0, right = r->numEntries - 1, mid;

            while ( left <= right) {
                mid = (left + right) / 2;
                 
                int cmp = strcmp( r->entries[mid].key, key);
                if (cmp == 0) {
                    return 0; 

                } else if (cmp < 0) {
                    left = mid + 1;
                } else {
                    right = mid - 1;
                }
            }
            i = left;

        } else {
            // linear seach
            for (i = 0; i < r->numEntries; i++ ) {

                int cmp = strncmp(r->entries[i].key, key, 1024 );
                if (cmp == 0 ) {
                    return 0; 
                } else if ( cmp > 0) {
                    break;
                }
            }
        }
        
    } else {
        i = 0;
    }

    // shift existing entries to make space for new entry
    for (int j = r->numEntries; j > i; j--) {
        r->entries[j] = r->entries [j - 1];
    }

    // insert new entry
    r->entries[i].key = malloc(strlen(key) + 1);
    
    if (r->entries[i].key == NULL) {
        // memory allocation error
        return 0;
    }
    strcpy(r->entries[i].key, key);
    r->entries[i].value = value;
    r->numEntries++;

    #ifdef DEBUG
    dictPrint(d, DEBUG_LEVEL);
    #endif

    return 1;
}

/**
 * free all resources allocated for this Dict. Everything, and only those things
 * allocated by this code should be freed.
 */
void dictFree(Dict *d){

	    // go through each row
    for(int i = 0; i < d->numRows; i++ ) {

        DictRow *r = &d->rows[i];


        for(int j = 0; j < r->numEntries; j++) {
            DictEntry *entry = &r->entries[j];
            free(entry->key);
            free(entry->value);
        }
        // free the row's entries and the row itself
        free(r->entries);
        //row->entries = NULL;
    }
    // free the rows array and the dictionary itself
    free(d->rows);
    free(d);	
}

/**
 * Allocate and initialize a new Dict. Initially this dictionary will have initRows
 * hash slots. If initRows==0, then it defaults to DICT_INIT_ROWS
 * Returns the address of the new Dict on success
 * Returns NULL on failure
 */
Dict * dictNew(int initRows){
    // Allocate memory for the new dictionary
    Dict *d = malloc(sizeof(Dict));
    if (d == NULL) {
        return NULL;  // Failed to allocate memory
    }

    // Initialize the number of rows
    if (initRows <= 0) {
        initRows = DICT_INIT_ROWS;  // Default number of rows
    }
    d->numRows = initRows;

    // Allocate memory for the rows
    d->rows = malloc(initRows * sizeof(DictRow));
    if (d->rows == NULL) {
        free(d);  // Free previously allocated memory
        return NULL;  // Failed to allocate memory
    }

    // Initialize each row
    for (int i = 0; i < initRows; i++) {
        d->rows[i].numEntries = 0;
        d->rows[i].capacity = ROW_INIT_ENTRIES;
        d->rows[i].entries = malloc(d->rows[i].capacity * sizeof(DictEntry));
        if (d->rows[i].entries == NULL) {
            // Failed to allocate memory for the entries
            // Free previously allocated memory
            for (int j = 0; j < i; j++) {
                free(d->rows[j].entries);
            }
            free(d->rows);
            free(d);
            return NULL;  // Failed to allocate memory
        }
    }

    return d;
}

