/* Utility - some helper classes
**
** Initial version by Axel Dörfler, axeld@pinc-software.de
** This file may be used under the terms of the OpenBeOS License.
*/


#include "Utility.h"
#include "Debug.h"

#include <stdlib.h>
#include <string.h>


int32
sorted_array::Find(off_t value)
{
	int32 size = count,first = 0;
	while (size > 0) {
		int32 i = size >> 1;
		off_t cmp = value - values[first + i];

		if (cmp == 0)
			return first + i;
		if (cmp > 0) {
			first += i + 1;
			size = (size-1) >> 1;
		} else
			size = i;
	}
	return -1;
}


void 
sorted_array::Insert(off_t value)
{
	// ToDo: if the value count is greater 8, it should do a binary
	// search to get the correct insertion point
	int32 i = 0,size = count,first = 0;
	while (size > 0) {
		/*int32*/ i = size >> 1;
		off_t cmp = value - values[first + i];

		if (cmp == 0) {
			i = first + i;
			break; // i;
		}
		if (cmp > 0) {
			first += i + 1;
			size = (size-1) >> 1;
		} else
			size = i;
	}
	if (size == 0)
		i += first;
	else
		__out("already in list!\n");
	int32 j = 0;
	for (;j < count;j++)
		if (values[j] > value)
			break;
if (j != i-1 && i != j)
	PRINT(("insert: bin says: %ld, correct is %ld\n",i,j));
	i = j;

	memmove(&values[i+1],&values[i],(count - i) * sizeof(off_t));
	values[i] = value;
	count++;
}


bool 
sorted_array::Remove(off_t value)
{
	int32 index = Find(value);
	if (index == -1)
		return false;

	memmove(&values[index],&values[index + 1],(count - index) * sizeof(off_t));
	count--;

	return true;
}


//	#pragma mark -


BlockArray::BlockArray(int32 blockSize)
	:
	fArray(NULL),
	fSize(0),
	fBlockSize(blockSize)
{
}


BlockArray::~BlockArray()
{
	if (fArray)
		free(fArray);
}


int32
BlockArray::Find(off_t value)
{
	if (fArray == NULL)
		return -1;
	
	return fArray->Find(value);
}


status_t
BlockArray::Insert(off_t value)
{
	if (fArray == NULL || fArray->count + 1 > fMaxBlocks) {
		sorted_array *array = (sorted_array *)realloc(fArray,fSize + fBlockSize);
		if (array == NULL)
			return B_NO_MEMORY;
		
		if (fArray == NULL)
			array->count = 0;

		fArray = array;
		fSize += fBlockSize;
		fMaxBlocks = fSize / sizeof(off_t) - 1;
	}
//PRINT(("blockSize = %ld, size = %ld, max blocks = %ld\n",fBlockSize,fSize,fMaxBlocks));
//PRINT(("array->count == %Ld (before insertion)\n",fArray->count));

	fArray->Insert(value);
	return B_OK;
}


status_t
BlockArray::Remove(off_t value)
{
	if (fArray == NULL)
		return B_ENTRY_NOT_FOUND;

	return fArray->Remove(value) ? B_OK : B_ENTRY_NOT_FOUND;
}


void 
BlockArray::MakeEmpty()
{
	fArray->count = 0;
}

