/** \file
 * \brief iupTable functions.
 * Implementation by Danny Reinhold and Antonio Scuri. 
 *
 * See Copyright Notice in iup.h
 *  */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "itable.h"
#include "istrutil.h"

/* #define DEBUGTABLE 1 */

/* Adjust these parameters for optimal performance and memory usage */
static const unsigned int itable_maxTableSizeIndex = 8;
static const unsigned int itable_hashTableSize[] = { 31, 101, 401, 1601, 4001, 8009, 16001, 32003, 64007 };
static const float itable_resizeLimit = 2;
static const unsigned int itable_itemGrow = 5;

/* Iteration context.
 */
typedef struct ItableContext
{
  unsigned int entryIndex;  /* index at the Itable::entries array */
  unsigned int itemIndex;   /* index at the ItableEntry::items array */
} ItableContext;

/* A key of an item.
 * To avoid lots of string comparisions we store
 * a keyindex as an integer.
 * To find a key in an item list we only have to
 * do integer comparisions.
 * Additionally the key itself is stored in
 * keyStr. In a string indexed hashtable this is
 * a duplicated string, in a pointer indexed hash table
 * this is simply the pointer (in this case keyIndex
 * and keyStr are equal).
*/
typedef struct ItableKey
{
  unsigned long keyIndex;  /* the secondary hash number */
  const char   *keyStr;
}
ItableKey;

/* An item in the hash table.
 * Such an item is stored in the item list of
 * an entry.
 */
typedef struct ItableItem
{
  enum Itable_Types  itemType;
  ItableKey          key;
  void              *value;
}
ItableItem;

/* An entry in the hash table.
 * An entry is chosen by an index in the hash table
 * and contains a list of items.
 * The number of items in this list is stored
 * in nextItemIndex.
 * size is the current size of the items array.
 */
typedef struct ItableEntry
{
  unsigned int  nextItemIndex;
  unsigned int  size;
  ItableItem   *items;
}
ItableEntry;


/* A hash table.
 * indexType is the type of the index.
 * entries is an array of entries. Select an
 * entry by its index.
 * size is the number of entries in the hash table...
 */
struct Itable
{
  unsigned int            size;
  unsigned int            numberOfEntries;
  unsigned int            tableSizeIndex;  /* index into itable_hashTableSize array */
  enum Itable_IndexTypes  indexType;
  ItableEntry            *entries;
  ItableContext           context;
};


/* Prototypes of private functions */

static void itableFreeItemArray(enum Itable_IndexTypes indexType, unsigned int nextFreeIndex, 
                                                            ItableItem *items);
static unsigned int itableGetEntryIndex(Itable *it, const char *key, unsigned long *keyIndex);
static unsigned int itableFindItem(Itable *it, const char *key, ItableEntry **entry, 
                                                          unsigned int *itemIndex,
                                                          unsigned long *keyIndex);
static unsigned int itableResize(Itable *it);
static void itableAdd(Itable *it, ItableKey *key, void *value, enum Itable_Types itemType);
static void itableUpdateArraySize(ItableEntry *entry);   

#ifdef DEBUGTABLE
static void itableShowStatistics(Itable *it);
static void itableCheckDuplicated(ItableItem *item, unsigned int nextItemIndex, 
                                              const char *key, 
                                              unsigned long keyIndex);
#endif


Itable *iupTableCreate(enum Itable_IndexTypes indexType)
{
  return iupTableCreate2(indexType, 1);  /* 101 shows to be a better start for IUP */
}


Itable *iupTableCreate2(enum Itable_IndexTypes indexType, unsigned int initialSizeIndex)
{
  Itable *itable = (Itable *)malloc(sizeof(struct Itable));

  assert(itable);
  if(!itable)
    return 0;

  if(initialSizeIndex > itable_maxTableSizeIndex)
    initialSizeIndex = itable_maxTableSizeIndex;

  itable->size            = itable_hashTableSize[initialSizeIndex];
  itable->tableSizeIndex  = initialSizeIndex;
  itable->numberOfEntries = 0;
  itable->indexType       = indexType;

  itable->entries = (ItableEntry *)malloc(itable->size * sizeof(ItableEntry));
  assert(itable->entries);
  if(!itable->entries)
  {
    free(itable);
    return 0;
  }

  memset(itable->entries, 0, itable->size * sizeof(ItableEntry));

  itable->context.entryIndex = (unsigned int)-1;
  itable->context.itemIndex = (unsigned int)-1;

  return itable;
}


void iupTableDestroy(Itable *it)
{
  unsigned int i;

  if(!it)
    return;

#ifdef DEBUGTABLE
   itableShowStatistics(it);
#endif

  for(i = 0; i < it->size; i++)
  {
    ItableEntry *entry = &(it->entries[i]);
    if(entry->items)
      itableFreeItemArray(it->indexType, entry->nextItemIndex, entry->items);
  }

  if(it->entries)
    free(it->entries);

  free(it);
}


void iupTableSet(Itable *it, const char *key, void *value, enum Itable_Types itemType)
{
  unsigned int  itemIndex,
                itemFound;
  unsigned long keyIndex;
  ItableEntry  *entry;
  ItableItem   *item;
  void         *v;

  assert(it && key);
  if(!it || !key || !value)
    return;

  itemFound = itableFindItem(it, key, &entry, &itemIndex, &keyIndex);

#ifdef DEBUGTABLE
  if(it->indexType == IUPTABLE_STRINGINDEXED)
    itableCheckDuplicated(&(entry->items[0]), entry->nextItemIndex, key, keyIndex);
#endif

  if(!itemFound)
  {
    /* create a new item */

    /* first check if the hash table has to be reorganized */
    if (itableResize(it))
    {
      /* We have to search for the entry again, since it may
       * have been moved by itableResize. */
      itableFindItem(it, key, &entry, &itemIndex, &keyIndex);
    }

    itableUpdateArraySize(entry);

    /* add the item at the end of the item array */
    if(itemType == IUP_STRING)
      v = iupStrDup(value);
    else
      v = value;

    item = &(entry->items[entry->nextItemIndex]);

    item->itemType     = itemType;
    item->key.keyIndex = keyIndex;
    item->key.keyStr   = it->indexType == IUPTABLE_STRINGINDEXED? iupStrDup(key) : key;
    item->value        = v;

    entry->nextItemIndex++;
    it->numberOfEntries++;
  }
  else
  {
    /* change an existing item */
    void *v;
    item = &(entry->items[itemIndex]);

    if (itemType == IUP_STRING && item->itemType == IUP_STRING)
    {
      /* this will avoid to free + alloc of a new pointer */
      if(iupStrEqual((char*)item->value, (char*)value))
        return;
    }

    if(itemType == IUP_STRING)
      v = iupStrDup(value);
    else
      v = value;

    if(item->itemType == IUP_STRING)
      free(item->value);

    item->value    = v;
    item->itemType = itemType;
  }
}


void iupTableRemove(Itable *it, const char *key)
{
  unsigned int itemFound,
               itemIndex;
  unsigned long keyIndex;
  ItableEntry  *entry;

  assert(it && key);
  if(!it || !key)
    return;

  itemFound = itableFindItem(it, key, &entry, &itemIndex, &keyIndex);
  if(itemFound)
  {
    ItableItem   *item;
    unsigned int  i;

    item = &(entry->items[itemIndex]);

    if(it->indexType == IUPTABLE_STRINGINDEXED)
      free((void *)(item->key.keyStr));

    if(item->itemType == IUP_STRING)
      free(item->value);

    for(i = itemIndex; i < entry->nextItemIndex-1; i++)
      entry->items[i] = entry->items[i+1];

    entry->nextItemIndex--;
    it->numberOfEntries--;
  }
}


void *iupTableGet(Itable *it, const char *key)
{
  unsigned int  itemFound,
                itemIndex;
  unsigned long keyIndex;
  ItableEntry  *entry;
  void         *value = 0;

  assert(it && key);
  if(!it || !key)
    return 0;

  itemFound = itableFindItem(it, key, &entry, &itemIndex, &keyIndex);
  if(itemFound)
    value = entry->items[itemIndex].value;

  return value;
}


void *iupTableGet2(Itable *it, const char *key, enum Itable_Types *itemType)
{
  unsigned int  itemFound,
                itemIndex;
  unsigned long keyIndex;
  ItableEntry  *entry;
  void         *value = 0;

  assert(it && key);
  if(!it || !key)
    return 0;

  itemFound = itableFindItem(it, key, &entry, &itemIndex, &keyIndex);
  if (itemFound)
  {
    value = entry->items[itemIndex].value;
    if (itemType) 
      *itemType = entry->items[itemIndex].itemType;
  }

  return value;
}


void *iupTableGetCurr(Itable *it)
{
  assert(it);
  if(!it || it->context.entryIndex == (unsigned int)-1
         || it->context.itemIndex == (unsigned int)-1)
    return 0;

  return it->entries[it->context.entryIndex].items[it->context.itemIndex].value;
}


char *iupTableFirst(Itable *it)
{
  unsigned int entryIndex;

  assert(it);
  if(!it)
    return 0;

  it->context.entryIndex = (unsigned int)-1;
  it->context.itemIndex = (unsigned int)-1;

  /* find the first used entry */
  for(entryIndex = 0; entryIndex < it->size; entryIndex++)
  {
    if (it->entries[entryIndex].nextItemIndex > 0)
    {
      it->context.entryIndex = entryIndex;
      it->context.itemIndex = 0;
      return (char*)it->entries[entryIndex].items[0].key.keyStr;
    }
  }

  return 0;
}


char *iupTableNext(Itable *it)
{
  unsigned int entryIndex;

  assert(it);
  if(!it || it->context.entryIndex == (unsigned int)-1
         || it->context.itemIndex == (unsigned int)-1)
    return 0;

  if(it->context.itemIndex + 1 < it->entries[it->context.entryIndex].nextItemIndex)
  {
    /* key in the current entry */
    it->context.itemIndex++;
    return (char*)it->entries[it->context.entryIndex].items[it->context.itemIndex].key.keyStr;
  }
  else
  {
    /* find the next used entry */
    for(entryIndex = it->context.entryIndex+1; entryIndex < it->size; entryIndex++)
    {
      if (it->entries[entryIndex].nextItemIndex > 0)
      {
        it->context.entryIndex = entryIndex;
        it->context.itemIndex = 0;
        return (char*)it->entries[entryIndex].items[0].key.keyStr;
      }
    }
  }

  return 0;
}


/********************************************/
/*           Private functions              */
/********************************************/


static void itableFreeItemArray(enum Itable_IndexTypes indexType, unsigned int nextFreeIndex, 
                                                            ItableItem *items)
{
  unsigned int i;

  assert(items);
  if(!items)
    return;

  if(indexType == IUPTABLE_STRINGINDEXED)
  {
    for(i = 0; i < nextFreeIndex; i++)
      free((void *)(items[i].key.keyStr));
  }

  for(i = 0; i < nextFreeIndex; i++)
  {
    if(items[i].itemType == IUP_STRING)
      free(items[i].value);
  }

  free(items);
}


static unsigned int itableGetEntryIndex(Itable *it, const char *key, unsigned long *keyIndex)
{
  if(it->indexType == IUPTABLE_STRINGINDEXED)
  {
    unsigned int i, checksum = 0;

    for(i = 0; key[i]; i++)
      checksum = checksum*31 + key[i]; 

    *keyIndex = checksum;            /* this could NOT be dependent from table size */
  }
  else
  {
    /* Pointer indexed */
    *keyIndex = (unsigned long)key;   /* this could NOT be dependent from table size */
  }

  return (unsigned int)((*keyIndex) % it->size);
}

#ifdef DEBUGTABLE
static void itableCheckDuplicated(ItableItem *item, unsigned int nextItemIndex, const char *key, 
                                                                          unsigned long keyIndex)
{
  unsigned int i;
  for(i = 0; i < nextItemIndex; i++, item++)
  {
    if (!iupStrEqual((char*)item->key.keyStr, (char*)key) && 
        item->key.keyIndex == keyIndex)
    {
      fprintf(stderr, "#ERROR# Duplicated key index (%ld): %s %s \n", keyIndex, 
                                                                     (char*)item->key.keyStr, 
                                                                     (char*)key);
    }
  }
}
#endif

static unsigned int itableFindItem(Itable *it, const char *key, ItableEntry **entry, 
                                                          unsigned int *itemIndex,
                                                          unsigned long *keyIndex)
{
  unsigned int entryIndex,
               itemFound,
               i;
  ItableItem  *item;

  entryIndex = itableGetEntryIndex(it, key, keyIndex);

  *entry = &(it->entries[entryIndex]);

  item = &((*entry)->items[0]);
  for(i = 0; i < (*entry)->nextItemIndex; i++, item++)
  {
    if(it->indexType == IUPTABLE_STRINGINDEXED)
      itemFound = item->key.keyIndex == *keyIndex; 
/*    itemFound = iupStrEqual(item->key.keyStr, key);  This is the original safe version */
    else
      itemFound = item->key.keyStr == key;

    if(itemFound)
    {
      *itemIndex = i;
      return 1;
    }
  }

  /* if not found "entry", "itemIndex" and "keyIndex" will have the new insert position. */

  *itemIndex = i;
  return 0;
}

static void itableUpdateArraySize(ItableEntry *entry)
{
  if(entry->nextItemIndex >= entry->size)
  {
    /* we have to expand the item array */
    unsigned int newSize;

    newSize = entry->size + itable_itemGrow;

    entry->items = (ItableItem *)realloc(entry->items, newSize * sizeof(ItableItem));
    assert(entry->items);
    if(!entry->items)
      return;

    entry->size = newSize;
  }
}


static void itableAdd(Itable *it, ItableKey *key, void *value, enum Itable_Types itemType)
{
  unsigned int entryIndex;
  unsigned long keyIndex;
  ItableEntry *entry;
  ItableItem* item;

  entryIndex = itableGetEntryIndex(it, key->keyStr, &keyIndex);

  entry = &(it->entries[entryIndex]);
  itableUpdateArraySize(entry);

  /* add a new item at the end of the item array without duplicating memory. */
  item = &(entry->items[entry->nextItemIndex]);
  item->itemType     = itemType;
  item->key.keyIndex = keyIndex;
  item->key.keyStr   = key->keyStr;
  item->value        = value;

  entry->nextItemIndex++;
  it->numberOfEntries++;
}

static unsigned int itableResize(Itable *it)
{
  unsigned int   newSizeIndex,
                 entryIndex,
                 i;
  Itable        *newTable;
  ItableEntry   *entry;
  ItableItem    *item;

  /* check if we do not need to resize the hash table */
  if(it->numberOfEntries == 0 ||
     it->tableSizeIndex >= itable_maxTableSizeIndex ||
     it->size / it->numberOfEntries >= itable_resizeLimit)
    return 0;

  /* create a new hash table and copy the contents of
   * the current table into the new one
   */
  newSizeIndex = it->tableSizeIndex + 1;
  newTable = iupTableCreate2(it->indexType, newSizeIndex);

  for(entryIndex = 0; entryIndex < it->size; entryIndex++)
  {
    entry = &(it->entries[entryIndex]);

    if (entry->items)
    {
      item = &(entry->items[0]);

      for(i = 0; i < entry->nextItemIndex; i++, item++)
      {
        itableAdd(newTable, &(item->key), item->value, item->itemType);
      }     

      free(entry->items);
    }
  }

  free(it->entries);

  it->size            = newTable->size;
  it->tableSizeIndex  = newTable->tableSizeIndex;
  it->numberOfEntries = newTable->numberOfEntries;
  it->entries         = newTable->entries;

  free(newTable);

  return 1;
}

#ifdef DEBUGTABLE
static void itableShowStatistics(Itable *it)
{
  unsigned int nofSlots        = 0;
  unsigned int nofKeys         = 0;
  double optimalNofKeysPerSlot = 0.0;
  unsigned int nofSlotsWithMoreKeys = 0;
  unsigned int nofSlotsWithLessKeys = 0;

  unsigned int entryIndex;
  fprintf(stderr, "\n--- HASH TABLE STATISTICS ---\n");
  if(!it)
  {
    fprintf(stderr, "no hash table...\n");
    return;
  }

  nofSlots = it->size;
  nofKeys  = it->numberOfEntries;
  optimalNofKeysPerSlot = (double)nofKeys / (double)nofSlots;

  for(entryIndex = 0; entryIndex < it->size; entryIndex++)
  {
    ItableEntry *entry = &(it->entries[entryIndex]);

    if(entry->nextItemIndex > optimalNofKeysPerSlot + 3)
      nofSlotsWithMoreKeys++;
    else if(entry->nextItemIndex < optimalNofKeysPerSlot - 3)
      nofSlotsWithLessKeys++;
  }

  fprintf(stderr, "Number of slots: %d\n", nofSlots);
  fprintf(stderr, "Number of keys: %d\n", nofKeys);
  fprintf(stderr, "Optimal number of keys per slot: %f\n", optimalNofKeysPerSlot);
  fprintf(stderr, "Number of slots with much more keys: %d\n", nofSlotsWithMoreKeys);
  fprintf(stderr, "Number of slots with far less keys: %d\n", nofSlotsWithLessKeys);
  fprintf(stderr, "\n");
}
#endif
