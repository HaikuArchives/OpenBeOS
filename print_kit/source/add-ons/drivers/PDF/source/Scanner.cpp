/*

Scanner.

Copyright (c) 2002 OpenBeOS. 

Author: 
	Michael Pfeiffer
	
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

#include "Scanner.h"
#include <ctype.h>

Scanner::Scanner(const char* name) {
	fFile = fopen(name, "r");
}

Scanner::~Scanner() {
	if (fFile) fclose(fFile);
}

status_t Scanner::InitCheck() {
	return fFile != NULL ? B_OK : B_ERROR;
}

bool Scanner::IsEOF() {
	return feof(fFile);
}

void Scanner::SkipSpaces() 
{
	int c = fgetc(fFile);
	for(;;) {
		while (!feof(fFile) && isspace(c)) c = fgetc(fFile);
		if (!feof(fFile) && c == '#') { // line comment; skip to end of line
			while (!feof(fFile) && c != '\n') c = fgetc(fFile);
		} else {
			ungetc(c, fFile); return;
		}
	}
}

bool Scanner::ReadName(BString *s) 
{
	SkipSpaces();
	*s = "";

	int c = fgetc(fFile);
	if (isalpha(c)) {
		do {
			s->Append((char)c, 1);
			c = fgetc(fFile);
		} while (!feof(fFile) && isalpha(c));
		ungetc(c, fFile);
		return true;
	}
	return false;		
}

bool Scanner::ReadString(BString *s) 
{
	SkipSpaces();
	*s = "";

	int c = fgetc(fFile);
	if (c == '"') {
		c = fgetc(fFile);
		while (!feof(fFile) && c != '"') {
			s->Append((char)c, 1);
			c = fgetc(fFile);
		}
		if (c == '"') return true;
	}	
	return false;
}

bool Scanner::ReadFloat(float *value) 
{
	SkipSpaces();
	BString s = "";
	*value = 0.0;
	
	int c = fgetc(fFile);
	if (isdigit(c) || c == '.') {
		do {
			s.Append((char)c, 1);
			c = fgetc(fFile);
		} while(isdigit(c) || c == '.');	
		if (EOF != sscanf(s.String(), "%f", value)) {
			ungetc(c, fFile);
			return true;
		}
	}
	return false;
}

bool Scanner::NextChar(char ch) {
	SkipSpaces();
	int c = fgetc(fFile);
	if (c == ch) {
		return true;
	} else {
		ungetc(c, fFile);
		return false;
	}
}

