/* Query - query parsing and evaluation
**
** Initial version by Axel Dörfler, axeld@pinc-software.de
** This file may be used under the terms of the OpenBeOS License.
*/


#include "bfs.h"

#include <SupportDefs.h>

#include <malloc.h>
#include <stdio.h>


// that's just a stand-alone testing version, just enter:
//		gcc -g Query.cpp
// to compile it.
//
// It now parses the whole query string correctly. The only thing that should
// be fixed is that poor error handling - this should definitely be improved,
// especially if this is going to live in user-space (where we could report
// detailed error messages).
//
// It puts out the infix notation of the parsed tree - including brackets
// to make clear how the operator precedence was handled.
//
// It's a very static design, but it will do what is required - if we'd have
// that functionality in user-land, we could solve it a bit nicer (e.g. no hard-
// coded operator precedence, the Term class could have subclasses And, ... and
// so on) - but that doesn't matter too much.
//
// ParseOr(), ParseAnd(), ParseEquation() are guarantying the operator
// precedence, that is =,!=,>,<,>=,<= .. && .. ||.
// Apparently, the "!" (not) can only be used with brackets.


// this will be moved to the header, when it's ready:

enum ops {
	OP_NONE,

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

		status_t	InitCheck();
		status_t	ParseQuotedString(char **_start,char **_end);
		char		*CopyString(char *start, char *end);

		void		PrintToStream();

	private:
		int8		fOp;
		char		*fAttribute;
		char		*fValue;
		bool		fIsRegExp;
};	

class Term {
	public:
		Term(Equation *);
		Term(Term *,int8,Term *);
		~Term();
	
		void		PrintToStream();

	private:
		int8		fOp;
		Term		*fLeft,*fRight;
		Equation	*fEquation;
};

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


void 
skipWhitespace(char **expr)
{
	char *string = *expr;
	while (*string == ' ' || *string == '\t') string++;
	*expr = string;
}


void 
skipWhitespaceReverse(char **expr,char *stop)
{
	char *string = *expr;
	while (string > stop && (*string == ' ' || *string == '\t')) string--;
	*expr = string;
}


//	#pragma mark -


Equation::Equation(char **expr)
	:
	fOp(OP_NONE),
	fAttribute(NULL),
	fValue(NULL)
{
	char *string = *expr;
	char *start = string;
	char *end = NULL;

	// Since the equation is the integral part of any query, we're just parsing
	// the whole thing here.
	// The whitespace at the start is already removed in Expression::ParseEquation()

	if (*start == '"' || *start == '\'') {
		// string is quoted (start has to be on the beginning of a string)
		if (ParseQuotedString(&start,&end) < B_OK)
			return;

		// set string to a valid start of the equation symbol
		string = end + 2;
		skipWhitespace(&string);
		if (*string != '=' && *string != '<' && *string != '>' && *string != '!')
			return;
	} else {
		// search the (in)equation for the actual equation symbol
		while (*string && *string != '=' && *string != '<' && *string != '>' && *string != '!')
			string++;

		// get the attribute string	(and trim whitespace), in case
		// the string was not quoted
		end = string - 1;
		skipWhitespaceReverse(&end,start);
	}

	// attribute string is empty (which is not allowed)
	if (start == end)
		return;
		
	// at this point, "start" points to the beginning of the string, "end" points
	// to the last character of the string, and "string" points to the first
	// character of the equation symbol

	// test for the right symbol (as this doesn't need any memory)
	switch (*string) {
		case '=':
			fOp = OP_EQUAL;
			break;
		case '>':
			fOp = *(string + 1) == '=' ? OP_GREATER_OR_EQUAL : OP_GREATER;
			break;
		case '<':
			fOp = *(string + 1) == '=' ? OP_LESSER_OR_EQUAL : OP_LESSER;
			break;
		case '!':
			if (*(string + 1) != '=')
				return;
			fOp = OP_UNEQUAL;
			break;
		
		// any invalid characters will be rejected
		default:
			return;
	}
	// lets change "start" to point to the first character after the symbol
	if (*(string + 1) == '=')
		string++;
	string++;
	skipWhitespace(&string);

	// allocate & copy the attribute string

	fAttribute = CopyString(start,end);
	if (fAttribute == NULL)
		return;

	start = string;
	if (*start == '"' || *start == '\'') {
		// string is quoted (start has to be on the beginning of a string)
		if (ParseQuotedString(&start,&end) < B_OK)
			return;
		
		string = end + 2;
		skipWhitespace(&string);
	} else {
		while (*string && *string != '&' && *string != '|' && *string != ')')
			string++;

		end = string - 1;
		skipWhitespaceReverse(&end,start);
	}
	
	// at this point, "start" will point to the first character of the value,
	// "end" will point to its last character, and "start" to the first non-
	// whitespace character after the value string

	fValue = CopyString(start,end);
	if (fValue == NULL)
		return;

	// this isn't checked for yet
	fIsRegExp = false;

	*expr = string;
}


Equation::~Equation()
{
	if (fAttribute != NULL)
		free(fAttribute);
	if (fValue != NULL)
		free(fValue);
}


status_t 
Equation::InitCheck()
{
	if (fAttribute == NULL
		|| fValue == NULL
		|| fOp == OP_NONE)
		return B_BAD_VALUE;

	return B_OK;
}


status_t 
Equation::ParseQuotedString(char **_start, char **_end)
{
	char *start = *_start;
	char quote = *start++;
	char *end = start;
	
	for (;*end && *end != quote;end++) {
		if (*end == '\\')
			end++;
	}
	if (*end == '\0')
		return B_BAD_VALUE;

	*_start = start;
	*_end = end - 1;

	return B_OK;
}


char *
Equation::CopyString(char *start,char *end)
{
	// end points to the last character of the string - and the length
	// also has to include the null-termination
	int32 length = end + 2 - start;
	// just to make sure; since that's the max. attribute name length and
	// the max. string in an index, it make sense to have it that way
	if (length > INODE_FILE_NAME_LENGTH || length <= 0)
		return NULL;

	char *copy = (char *)malloc(length);
	if (copy == NULL)
		return NULL;

	memcpy(copy,start,length - 1);
	copy[length - 1] = '\0';

	return copy;
}


void 
Equation::PrintToStream()
{
	char *symbol = "???";
	switch (fOp) {
		case OP_EQUAL: symbol = "=="; break;
		case OP_UNEQUAL: symbol = "!="; break;
		case OP_GREATER: symbol = ">"; break;
		case OP_GREATER_OR_EQUAL: symbol = ">="; break;
		case OP_LESSER: symbol = "<"; break;
		case OP_LESSER_OR_EQUAL: symbol = "<="; break;
	}
	printf("[\"%s\" %s \"%s\"]",fAttribute,symbol,fValue);
}


//	#pragma mark -


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


//	#pragma mark -


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
	skipWhitespace(expr);

	if (**expr == ')') {
		// shouldn't be handled here
		return NULL;
	} else if (**expr == '(') {
		(*expr)++;
		skipWhitespace(expr);
		
		Term *term = ParseOr(expr);
		
		skipWhitespace(expr);
		
		if (**expr != ')') {
			delete term;
			return NULL;
		}
		(*expr)++;
		
		skipWhitespace(expr);

		return term;
	}

	Equation *equation = new Equation(expr);
	if (equation == NULL || equation->InitCheck() < B_OK) {
		delete equation;
		return NULL;
	}
	return new Term(equation);
}


Term *
Expression::ParseAnd(char **expr)
{
	Term *left = ParseEquation(expr);

	while (IsAnd(expr)) {
		Term *right = ParseAnd(expr);

		if (right != NULL) {
			Term *newParent = new Term(left,OP_AND,right);
			if (newParent == NULL) {
				delete left;
				delete right;

				return NULL;
			}
			left = newParent;
		}
	}

	return left;
}


Term *
Expression::ParseOr(char **expr)
{
	Term *left = ParseAnd(expr);

	while (IsOr(expr)) {
		Term *right = ParseAnd(expr);

		if (right != NULL) {
			Term *newParent = new Term(left,OP_OR,right);
			if (newParent == NULL) {
				delete left;
				delete right;

				return NULL;
			}
			left = newParent;
		}
	}

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


//	#pragma mark -


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

