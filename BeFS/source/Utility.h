#ifndef UTILITY_H
#define UTILITY_H
/* Utility - some helper classes
**
** Initial version by Axel Dörfler, axeld@pinc-software.de
** This file may be used under the terms of the OpenBeOS License.
*/


#include <SupportDefs.h>


struct sorted_array {
	off_t	count;
	off_t	values[0];

	int32 Find(off_t value);
	void Insert(off_t value);
	bool Remove(off_t value);
};


class BlockArray {
	public:
		BlockArray(int32 blockSize);
		~BlockArray();

		int32 Find(off_t value);
		status_t Insert(off_t value);
		status_t Remove(off_t value);

		void MakeEmpty();

		int32 CountItems() const { return fArray != NULL ? fArray->count : 0; }
		sorted_array *Array() const { return fArray; }
		int32 Size() const { return fSize; }

	private:
		sorted_array *fArray;
		int32	fBlockSize;
		int32	fSize;
		int32	fMaxBlocks;
};


// Doubly linked list

template<class Node> struct node {
	Node *next,*prev;

	void
	Remove()
	{
		prev->next = next;
		next->prev = prev;
	}

	Node *
	Next()
	{
		if (next->next != NULL)
			return next;

		return NULL;
	}
};

template<class Node> struct list {
	Node *head,*tail,*last;

	list()
	{
		head = (Node *)&tail;
		tail = NULL;
		last = (Node *)&head;
	}

	void
	Add(Node *entry)
	{
		entry->next = (Node *)&tail;
		entry->prev = last;
		last->next = entry;
		last = entry;
	}
};


#endif	/* UTILITY_H */
