/* Query - query parsing and evaluation
**
** Initial version by Axel Dörfler, axeld@pinc-software.de
** This file may be used under the terms of the OpenBeOS License.
*/


#include <SupportDefs.h>

#include <malloc.h>
#include <stdio.h>


// that's just a stand-alone testing version, just enter:
//		gcc -g Query.cpp
// to compile it.
//
// Currently, it just understands || and &&, no brackets, etc., e.g.
// something like "a||b&&c||d" will fit - whitespace is currently regarded
// as part of the equation - thanks to !, and the brackets, we will have
// to filter them out earlier.
// It puts out the infix notation of the parsed tree - including brackets
// to make clear how the operator precedence was handled.
// The whole equation is currently left unparsed in Equation::fAttribute.
// Error checking is not done right now either, so it may loose some
// memory on errors (especially in cases when malloc() fails).
//
// It's a very static design, but it will do what is required - if we'd have
// that functionality in user-land, we could solve it a bit nicer (e.g. no hard-
// coded operator precedence, the Term class could have subclasses And, ... and
// so on) - but that doesn't matter too much.
//
// ParseOr(), ParseAnd(), ParseEquation() are guarantying the operator
// precedence, that is =,!=,>,<,>=,<= .. && .. ||.
// Apparently, the "!" (not) can only be used with brackets.


enum ops {
	OP_AND,
	OP_OR,
	OP_NOT,

	OP_EQUATION,

	OP_EQUAL,
	OP_UNEQUAL,
	OP_GREATER,
	OP_LESSER,
	OP_GREATER_OR_EQUAL,
	OP_LESSER_OR_EQUAL,
};

class Equation {
	public:
		Equation(char **expr);
		~Equation();

		void PrintToStream();

	private:
		int8	fOp;
		char	*fAttribute;
		char	*fValue;
		bool	fIsRegExp;
};


Equation::Equation(char **expr)
{
	char *string = *expr;

	while (*string && *string != '&' && *string != '|')
		string++;
	
	int32 length = string - *expr + 1;
	fAttribute = (char *)malloc(length);
	memcpy(fAttribute,*expr,length);
	fAttribute[length - 1] = '\0';
	
	fValue = NULL;

	*expr = string;
	
	fOp = OP_EQUAL;
	fIsRegExp = false;
}


Equation::~Equation()
{
	if (fAttribute != NULL)
		free(fAttribute);
	if (fValue != NULL)
		free(fValue);
}


void 
Equation::PrintToStream()
{
	printf("\"%s\"",fAttribute);
}


class Term {
	public:
		Term(Equation *);
		Term(Term *,int8,Term *);
		~Term();
	
		void PrintToStream();

	private:
		int8	fOp;
		Term	*fLeft,*fRight;
		Equation *fEquation;
};


Term::Term(Equation *eq)
	:
	fOp(OP_EQUATION),
	fLeft(NULL),
	fRight(NULL),
	fEquation(eq)
{
}


Term::Term(Term *left, int8 op, Term *right)
	:
	fOp(op),
	fLeft(left),
	fRight(right),
	fEquation(NULL)
{
}


Term::~Term()
{
	delete fLeft;
	delete fRight;
	delete fEquation;
}


void 
Term::PrintToStream()
{
	if (fOp == OP_EQUATION) {
		fEquation->PrintToStream();
		return;
	}

	printf("( ");
	if (fLeft != NULL)
		fLeft->PrintToStream();
	
	char *op;
	switch (fOp) {
		case OP_OR: op = "OR"; break;
		case OP_AND: op = "AND"; break;
		default: op = "?"; break;
	}
	printf(" %s ",op);
	
	if (fRight != NULL)
		fRight->PrintToStream();

	printf(" )");
}


class Expression {
	public:
		Expression(char *expr);
		~Expression();
		
	//private:
		Term *ParseOr(char **expr);
		Term *ParseAnd(char **expr);
		Term *ParseEquation(char **expr);

		bool IsOr(char **expr);
		bool IsAnd(char **expr);

		Term *fTerm;
};


Expression::Expression(char *expr)
{
	if (expr == NULL)
		return;
	
	fTerm = ParseOr(&expr);
}


Expression::~Expression()
{
	delete fTerm;
}


Term *
Expression::ParseEquation(char **expr)
{
	//char *string = *expr;

	// skip whitespace	
	// handle brackets and "!" (not) here 

	return new Term(new Equation(expr));
}


Term *
Expression::ParseAnd(char **expr)
{
	Term *left = ParseEquation(expr);
	Term *right = NULL;

	while (IsAnd(expr)) {
		if (right != NULL)
			left = new Term(left,OP_AND,right);

		right = ParseAnd(expr);
	}
	if (right != NULL)
		return new Term(left,OP_AND,right);

	return left;
}


Term *
Expression::ParseOr(char **expr)
{
	Term *left = ParseAnd(expr);
	Term *right = NULL;

	while (IsOr(expr)) {
		if (right != NULL)
			left = new Term(left,OP_OR,right);

		right = ParseAnd(expr);
	}
	if (right != NULL)
		return new Term(left,OP_OR,right);

	return left;
}


bool 
Expression::IsOr(char **expr)
{
	char *string = *expr;
	
	if (*string == '|' && *(string + 1) == '|') {
		*expr += 2;
		return true;
	}
	return false;
}


bool 
Expression::IsAnd(char **expr)
{
	char *string = *expr;
	
	if (*string == '&' && *(string + 1) == '&') {
		*expr += 2;
		return true;
	}
	return false;
}


int main(int argc,char **argv)
{
	if (argv[1] == NULL) {
		printf("usage: Query expression\n");
		return -1;
	}

	Expression expr(argv[1]);
	if (expr.fTerm != NULL) {
		expr.fTerm->PrintToStream();
		printf("\n");
	} else
		printf("Query not parsed!\n");

	return 0;
}

