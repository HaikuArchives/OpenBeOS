#ifndef QUERY_H
#define QUERY_H
/* Query - query parsing and evaluation
**
** Initial version by Axel Dörfler, axeld@pinc-software.de
** This file may be used under the terms of the OpenBeOS License.
*/


#include <SupportDefs.h>

#include "Index.h"
#include "Stack.h"

class Volume;
class Term;
class Equation;
class TreeIterator;
class QueryFetcher;


class Query {
	public:
		Query(char *expr);
		~Query();

		status_t InitCheck();
		const char *Position() const { return fPosition; }

	private:
		Term *ParseOr(char **expr);
		Term *ParseAnd(char **expr);
		Term *ParseEquation(char **expr);

		bool IsOperator(char **expr,char op);

		char *fPosition;
		
		friend QueryFetcher;
		Term *fTerm;
};

class QueryFetcher {
	public:
		QueryFetcher(Volume *volume,Query *query);
		~QueryFetcher();

		status_t GetNextEntry(struct dirent *,size_t size);

		Query *GetQuery() const { return fQuery; }

	private:
		Volume			*fVolume;
		Query			*fQuery;
		Equation		*fCurrent;
		TreeIterator	*fIterator;
		Index			fIndex;
		Stack<Equation *> fStack;
};

#endif	/* QUERY_H */
