/** \file
 * \brief Simple hash table C API.
 * Does not allow 0 values for items...
 *
 * See Copyright Notice in iup.h
 *  */
 
#ifndef __ITABLE_H 
#define __ITABLE_H

#ifdef __cplusplus
extern "C"
{
#endif

/** How the table key is interpreted. */
enum Itable_IndexTypes
{
  IUPTABLE_POINTERINDEXED = 10,
  IUPTABLE_STRINGINDEXED
};

/** How the value is interpreted. */
enum Itable_Types
{
  IUP_POINTER,     /** regular pointer for strings and other pointers */
  IUP_STRING,      /** string duplicated internally */
  IUP_RESPOINTER   /** also regular pointer, but with different identifier. */
};


struct Itable;
typedef struct Itable Itable;


/** Creates a hash table with an initial default size.
 * This function is equivalent to iupTableCreate2(0);
 */
Itable *iupTableCreate(enum Itable_IndexTypes indexType);

/** Creates a hash table with the specified initial size.
 * Use this function if you expect the table to become very large.
 * initialSizeIndex is an array into the (internal) list of
 * possible hash table sizes. Currently only indexes from 0 to 8
 * are supported. If you specify a higher value here, the maximum
 * allowed value will be used.
 */
Itable *iupTableCreate2(enum Itable_IndexTypes indexType, unsigned int initialSizeIndex);

/** Destroys the Itable.
 * This function does also free the memory of strings contained in the table!!!!
 */
void iupTableDestroy(Itable *n);

/** Store an element in the table.
*/
void iupTableSet(Itable *n, const char *key, void *value, enum Itable_Types itemType);

/** Retreives an element from the table.
*/
void *iupTableGet(Itable *n, const char *key);

/** Retreives an element from the table and its type.
*/
void *iupTableGet2(Itable *n, const char *key, enum Itable_Types *itemType);

/** Returns the value at the current position.
 * The current context is an iterator
 * that is filled by iupTableNext()...
 * iupTableGetCur() is faster then iupTableGet(),
 * so when you want to access an item stored
 * at a key returned by iupTableNext(),
 * use this function instead of iupTableGet()...
 */
void *iupTableGetCurr(Itable *it);

/** Removes the entry at the specified key from the
 * hash table and frees the memory used by it if
 * it is a string...
 */
void iupTableRemove(Itable *n, const char *key);

/** Key iteration function. Returns a key.
 * To iterate over all keys call iupTableFirst at the first
 * and call iupTableNext in a loop
 * until 0 is returned...
 * Do NOT change the content of the hash table during iteration.
 * During an iteration you can use context with
 * iupTableGetCurr() to access the value of the key
 * very fast.
 */
char *iupTableFirst(Itable *it);

/** Key iteration function. See \ref iupTableNext.
 */
char *iupTableNext(Itable *it);


#ifdef __cplusplus
}
#endif

#endif

