/* Query - query parsing and evaluation
**
** Initial version by Axel Dörfler, axeld@pinc-software.de
** This file may be used under the terms of the OpenBeOS License.
*/


#include "Query.h"
#include "cpp.h"
#include "bfs.h"
#include "Debug.h"
#include "Stack.h"
#include "Volume.h"
#include "Inode.h"
#include "BPlusTree.h"
#include "Index.h"

#include <SupportDefs.h>
#include <TypeConstants.h>

#include <malloc.h>
#include <stdio.h>
#include <string.h>


// I have decided to break our rules for the kernel, and use virtuals in
// the query code, virtuals aren't that bad anyway, and the query code
// shouldn't be in the kernel.
//
// It now parses the whole query string correctly. What's still missing is
// the handling of "!" (not), but I remember to have read something about
// it in Dominic's book.
//
// The parser has a very static design, but it will do what is required - if we'd
// have that functionality in user-land, we could solve it a bit nicer (e.g. no hard-
// coded operator precedence) - but that doesn't matter too much.
//
// ParseOr(), ParseAnd(), ParseEquation() are guarantying the operator
// precedence, that is =,!=,>,<,>=,<= .. && .. ||.
// Apparently, the "!" (not) can only be used with brackets.


// some of this will be moved to the header, when it's ready:

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


class Term {
	public:
		Term(int8 op) : fOp(op), fParent(NULL) {}

		int8		Op() const { return fOp; }

		void		SetParent(Term *parent) { fParent = parent; }
		Term		*Parent() const { return fParent; }

		virtual status_t Match(Inode *inode) = 0;

#ifdef DEBUG
		virtual void	PrintToStream() = 0;
#endif

	protected:
		int8	fOp;
		Term	*fParent;
};

class Equation : public Term {
	public:
		Equation(char **expr);
		~Equation();

		status_t	InitCheck();
		status_t	ParseQuotedString(char **_start,char **_end);
		char		*CopyString(char *start, char *end);

		virtual status_t Match(Inode *inode);
		status_t	RunQuery(Volume *volume);

		void		SetScore(int32 score) { fScore = score; }
		int32		Score() const { return fScore; }

#ifdef DEBUG
		virtual void PrintToStream();
#endif

	private:
		status_t ConvertValue(type_code type);

		char		*fAttribute;
		char		*fString;
		union {
			int64	Int64;
			uint64	Uint64;
			int32	Int32;
			uint32	Uint32;
			float	Float;
			double	Double;
			char	*String;
		} fValue;
		type_code	fType;
		bool		fIsRegExp;
		
		int32		fScore;
		bool		fRunsQuery;
};

class Operator : public Term {
	public:
		Operator(Term *,int8,Term *);
		~Operator();

		Term		*Left() const { return fLeft; }
		Term		*Right() const { return fRight; }

		//Term		*Copy() const;
#ifdef DEBUG
		virtual void PrintToStream();
#endif

	protected:
		Term		*fLeft,*fRight;
};

class AndOperator : public Operator {
	public:
		AndOperator(Term *,Term *);
		~AndOperator();

		virtual status_t Match(Inode *inode);
};

class OrOperator : public Operator {
	public:
		OrOperator(Term *,Term *);
		~OrOperator();

		virtual status_t Match(Inode *inode);
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
	: Term(OP_EQUATION),
	fAttribute(NULL),
	fString(NULL),
	fType(0),
	fRunsQuery(false)
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
		if (*string != '=' && *string != '<' && *string != '>' && *string != '!') {
			*expr = string;
			return;
		}
	} else {
		// search the (in)equation for the actual equation symbol (and for other operators
		// in case the equation is malformed)
		while (*string && *string != '=' && *string != '<' && *string != '>' && *string != '!'
			&& *string != '&' && *string != '|')
			string++;

		// get the attribute string	(and trim whitespace), in case
		// the string was not quoted
		end = string - 1;
		skipWhitespaceReverse(&end,start);
	}

	// attribute string is empty (which is not allowed)
	if (start > end)
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
			*expr = string;
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

	fString = CopyString(start,end);
	if (fString == NULL)
		return;

	// this isn't checked for yet
	fIsRegExp = false;

	*expr = string;
}


Equation::~Equation()
{
	if (fAttribute != NULL)
		free(fAttribute);
	if (fString != NULL)
		free(fString);
}


status_t 
Equation::InitCheck()
{
	if (fAttribute == NULL
		|| fString == NULL
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


status_t 
Equation::ConvertValue(type_code type)
{
	// Has the type already been converted?
	if (type == fType)
		return B_OK;

	fType = type;
	char *string = fString;

	switch (type) {
		B_STRING_TYPE:
		B_MIME_STRING_TYPE:
			fValue.String = string;
			break;
		B_INT32_TYPE:
			fValue.Int32 = strtol(string,&string,0);
			break;
		B_UINT32_TYPE:
			fValue.Int32 = strtoul(string,&string,0);
			break;
		B_INT64_TYPE:
			fValue.Int64 = strtoll(string,&string,0);
			break;
		B_UINT64_TYPE:
			fValue.Uint64 = strtoull(string,&string,0);
			break;
		B_FLOAT_TYPE:
			fValue.Float = strtod(string,&string);
			break;
		B_DOUBLE_TYPE:
			fValue.Double = strtod(string,&string);
			break;
		default:
			FATAL(("query value conversion to %lx requested!\n",type));
			// should we fail here or just do a safety int32 conversion?
			return B_ERROR;
	}
	return B_OK;
}


/**	Matches the inode's attribute value with the equation.
 *	Returns 1 if it matches, 0 if not, < 0 if something went wrong
 */

status_t
Equation::Match(Inode *inode)
{
	// get a pointer to the attribute in question
	uint8 *buffer;
	int32 type;
	size_t size;
	bool ownBuffer = false;
	
	// first, check for "fake" attributes, "name", "size", "last_modified",
	if (!strcmp(fAttribute,"name")) {
		buffer = (uint8 *)inode->Name();
		if (buffer == NULL)
			return B_ERROR;

		type = B_STRING_TYPE;
		size = strlen((const char *)buffer);
	} else if (!strcmp(fAttribute,"size")) {
		buffer = (uint8 *)&inode->Node()->data.size;
		type = B_INT64_TYPE;
	} else if (!strcmp(fAttribute,"last_modified")) {
		buffer = (uint8 *)&inode->Node()->last_modified_time;
		type = B_INT64_TYPE;
	} else {
		// then for attributes in the small_data section, and finally for the
		// real attributes
		Inode *attribute;
		small_data *smallData;

		if (smallData != NULL) {
			buffer = smallData->Data();
			type = smallData->type;
			size = smallData->data_size;
		} else if ((attribute = inode->GetAttribute(fAttribute)) != NULL) {
			size = attribute->Size();
			if (size > INODE_FILE_NAME_LENGTH)
				size = INODE_FILE_NAME_LENGTH;
			buffer = (uint8 *)malloc(size);
			if (buffer == NULL) {
				inode->ReleaseAttribute(attribute);
				return B_NO_MEMORY;
			}

			if (attribute->ReadAt(0,buffer,&size) < B_OK) {
				inode->ReleaseAttribute(attribute);
				free(buffer);
				return B_IO_ERROR;
			}
			inode->ReleaseAttribute(attribute);
			ownBuffer = true;
		}
	}
	// prepare value for use, if it is possible to convert it
	status_t status = ConvertValue(type);
	if (status == B_OK) {
		status = true;
	}

	// compare the values here!

	if (ownBuffer)
		free(buffer);

	RETURN_ERROR(status);
}


status_t 
Equation::RunQuery(Volume *volume)
{
	fRunsQuery = true;

	// to be implemented, renamed, reimplemented, changed, etc.
	
	fRunsQuery = false;

	return B_OK;
}


//	#pragma mark -


Operator::Operator(Term *left, int8 op, Term *right)
	: Term(op),
	fLeft(left),
	fRight(right)
{
	if (left)
		left->SetParent(this);
	if (right)
		right->SetParent(this);
}


Operator::~Operator()
{
	delete fLeft;
	delete fRight;
}


#if 0
Term *
Operator::Copy() const
{
	if (fEquation != NULL) {
		Equation *equation = new Equation(*fEquation);
		if (equation == NULL)
			return NULL;

		Term *term = new Term(equation);
		if (term == NULL)
			delete equation;
		
		return term;
	}

	Term *left = NULL, *right = NULL;

	if (fLeft != NULL && (left = fLeft->Copy()) == NULL)
		return NULL;
	if (fRight != NULL && (right = fRight->Copy()) == NULL) {
		delete left;
		return NULL;
	}

	Term *term = new Term(left,fOp,right);
	if (term == NULL) {
		delete left;
		delete right;
		return NULL;
	}
	return term;
}
#endif


AndOperator::AndOperator(Term *left, Term *right)
	: Operator(left,OP_AND,right)
{
}


AndOperator::~AndOperator()
{
}


status_t 
AndOperator::Match(Inode *inode)
{
	if (fLeft != NULL) {
		status_t status = fLeft->Match(inode);
		if (status < true)
			return status;
	}
	if (fRight != NULL) {
		status_t status = fRight->Match(inode);
		if (status < B_OK)
			return status;
	}
	return false;
}


OrOperator::OrOperator(Term *left, Term *right)
	: Operator(left,OP_OR,right)
{
}


OrOperator::~OrOperator()
{
}

status_t 
OrOperator::Match(Inode *inode)
{
	// should choose the term with the better score here!
	if (fLeft != NULL) {
		status_t status = fLeft->Match(inode);
		if (status != false)
			return status;
	}
	if (fRight != NULL)
		return fRight->Match(inode);

	return false;
}


//	#pragma mark -

#ifdef DEBUG
void
Operator::PrintToStream()
{
	D(__out("( "));
	if (fLeft != NULL)
		fLeft->PrintToStream();
	
	char *op;
	switch (fOp) {
		case OP_OR: op = "OR"; break;
		case OP_AND: op = "AND"; break;
		default: op = "?"; break;
	}
	D(__out(" %s ",op));
	
	if (fRight != NULL)
		fRight->PrintToStream();

	D(__out(" )"));
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
	D(__out("[\"%s\" %s \"%s\"]",fAttribute,symbol,fString));
}

#endif	/* DEBUG */

//	#pragma mark -


Query::Query(char *expr)
{
	if (expr == NULL)
		return;
	
	fTerm = ParseOr(&expr);
	D(if (fTerm != NULL) {
		fTerm->PrintToStream();
		D(__out("\n"));
		if (*expr != '\0')
			PRINT(("Unexpected end of string: \"%s\"!\n",expr));
	} else
		PRINT(("Query not parsed, terminated at: \"%s\"!\n",expr));
	);
	fPosition = expr;
}


Query::~Query()
{
	delete fTerm;
}


Term *
Query::ParseEquation(char **expr)
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
	return equation;
}


Term *
Query::ParseAnd(char **expr)
{
	Term *left = ParseEquation(expr);
	if (left == NULL)
		return NULL;

	while (IsOperator(expr,'&')) {
		Term *right = ParseAnd(expr);
		Term *newParent = NULL;

		if (right == NULL || (newParent = new AndOperator(left,right)) == NULL) {
			delete left;
			delete right;

			return NULL;
		}
		left = newParent;
	}

	return left;
}


Term *
Query::ParseOr(char **expr)
{
	Term *left = ParseAnd(expr);
	if (left == NULL)
		return NULL;

	while (IsOperator(expr,'|')) {
		Term *right = ParseAnd(expr);
		Term *newParent = NULL;

		if (right == NULL || (newParent = new OrOperator(left,right)) == NULL) {
			delete left;
			delete right;

			return NULL;
		}
		left = newParent;
	}

	return left;
}


bool 
Query::IsOperator(char **expr, char op)
{
	char *string = *expr;
	
	if (*string == op && *(string + 1) == op) {
		*expr += 2;
		return true;
	}
	return false;
}


status_t 
Query::InitCheck()
{
	if (fTerm == NULL)
		return B_BAD_VALUE;

	return B_OK;
}


status_t 
Query::GetNext(Volume *volume, struct dirent *, size_t size)
{
	Index index(volume);
	
	Stack<Term *> stack;
	stack.Push(fTerm);

	Term *term;
	while (stack.Pop(&term)) {
		if (term->Op() == OP_OR) {
			stack.Push(((Operator *)term)->Left());
			stack.Push(((Operator *)term)->Right());
		} else if (term->Op() == OP_AND) {
			// here we should use a scoring system to decide which path to add
			// for now, we are just using the left
			stack.Push(((Operator *)term)->Left());
		} else if (term->Op() > OP_EQUATION)
			((Equation *)term)->RunQuery(volume);
		else
			FATAL(("Unknown term on stack"));
	}
}

