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
class Query;


class Expression {
	public:
		Expression(char *expr);
		~Expression();

		status_t InitCheck();
		const char *Position() const { return fPosition; }
		Term *Root() const { return fTerm; }

	protected:
		Term *ParseOr(char **expr);
		Term *ParseAnd(char **expr);
		Term *ParseEquation(char **expr);

		bool IsOperator(char **expr,char op);

	private:
		char *fPosition;
		Term *fTerm;
};

class Query {
	public:
		Query(Volume *volume,Expression *expression);
		~Query();

		status_t GetNextEntry(struct dirent *,size_t size);

		Expression *GetExpression() const { return fExpression; }

	private:
		Volume			*fVolume;
		Expression		*fExpression;
		Equation		*fCurrent;
		TreeIterator	*fIterator;
		Index			fIndex;
		Stack<Equation *> fStack;
};

#endif	/* QUERY_H */
