/*

Detection and adding of web links.

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

#include <ctype.h>
#include "Link.h"
#include "PDFWriter.h"
#include "Log.h"

// TODO: check this list and add more prefixes
char* Link::fURLPrefix[] = {
	"http://",
	"https://",
	"ftp://",
	"telnet://",
	"mailto:",
	"news://",
	"nntp://",
	"gopher://",
	"wais://",
	"prospero://",
	"mid:",
	"cid:",
	"afs://",
	NULL
};


Link::Link(PDFWriter* writer, BString* utf8, BFont* font)
	: fWriter(writer)
	, fUtf8(utf8)
	, fFont(font)
	, fPos(0)
	, fContainsLink(false)
{
	DetectUrl(0);
}


// TODO: check for valid characters in url
bool
Link::IsValidCodePoint(const char* cp, int cps) {
	return ' ' < *cp && cps == 1; 
}


bool 
Link::DetectUrlWithPrefix(int start)
{
	int pos = INT_MAX;
	char* prefix = NULL;
	const char* utf8 = fUtf8->String();

	// search prefix with smallest position
	for (int i = 0; fURLPrefix[i]; i ++) {
		int p = fUtf8->FindFirst(fURLPrefix[i], start);
		if (p >= start && p < pos) {
			pos = p;
			prefix = fURLPrefix[i];
		}
	}
	
	if (pos != INT_MAX) {
		fStartPos = pos;
		fEndPos   = pos + strlen(prefix);
		
		// search end of url
		int         prev;		
		int         cps;
		const char* cp;

		prev = fEndPos;
		pos  = fEndPos;
		cp   = &utf8[pos];
		cps  = fWriter->CodePointSize(cp);
		while (IsValidCodePoint(cp, cps)) {
			prev = pos;
			pos += cps;
			cp  = &utf8[pos];
			cps  = fWriter->CodePointSize(cp);
		}
		
		// skip from end over '.' and '@'
		while (fStartPos < prev && (utf8[prev] == '.' || utf8[prev] == '@')) {
			// search to begin of code point
			do {
				prev --;
			} while (!fWriter->BeginsChar(utf8[prev]));
		}
		
		if (prev != fEndPos) { // url not empty
			fEndPos = prev; 
			return true;
		}
	}

	return false;
}


bool
Link::IsValidStart(const char* cp)
{
	return *cp != '.' && *cp != '@' && isalpha(*cp);
}

bool
Link::IsValidChar(const char* cp)
{
	return !isspace(*cp); // isalnum(*cp) || *cp == '-' || *cp == '.' || *cp == '@';
}

// TODO: add better url detection (ie. "e.g." is detected as "http://e.g")
// A complete implementation should identify these kind of urls correctly:
//   domain/~user
//   domain/page.html?value=key
bool
Link::DetectUrlWithoutPrefix(int start)
{
	const char* utf8 = fUtf8->String();	

	while (utf8[start]) {
		// search to start
		while (utf8[start] && !IsValidStart(&utf8[start])) {
			start += fWriter->CodePointSize(&utf8[start]);
		}
		
		// search end and count dots and @s
		int numDots = 0, numAts = 0;
		int end = start;
		int prev = end;
		int length = 0;
		while (utf8[end] && IsValidChar(&utf8[end])) {
			length ++;
			if (utf8[end] == '.') numDots ++;
			else if (utf8[end] == '@') numAts ++;
			prev = end; end += fWriter->CodePointSize(&utf8[end]);
		}
		
		// skip from end over '.' and '@'
		while (start < prev && (utf8[prev] == '.' || utf8[prev] == '@')) {
			if (utf8[prev] == '.') numDots --;
			else if (utf8[prev] == '@') numAts --;
			// search to begin of code point
			do {
				prev --;
			} while (!fWriter->BeginsChar(utf8[prev]));
		}
		
		fStartPos = start; fEndPos = prev;

		if (numAts == 1) {
			fKind = kMailtoKind;
			return true;
		} else if (numDots >= 2 && length > numDots && numAts == 0) {
			if (strncmp(&utf8[start], "ftp", 3) == 0) fKind = kFtpKind;
			else fKind = kHttpKind;
			return true;
		}
		
		// next iteration
		start = end;
	}	
	return false;
}

void 
Link::DetectUrl(int start)
{
	fContainsLink = true;
	fKind         = kUnknownKind;
	if (DetectUrlWithPrefix(start)) return;
	if (DetectUrlWithoutPrefix(start)) return;
	fContainsLink = false;
}


// create link from fStartPos to fEndPos and fStart to end
void 
Link::CreateLink(BPoint end)
{
	if (fFont->Rotation() != 0.0) {
		LOG((fWriter->fLog, "Warning: Can not create link for rotated font!\n"));
		return;
	}

	BString url;

	// prepend protocol if required
	switch (fKind) {
		case kMailtoKind:  url = "mailto:"; break;
		case kHttpKind:    url = "http://"; break;
		case kFtpKind:     url = "ftp://"; break;
		case kUnknownKind: break;
		default: 
			LOG((fWriter->fLog, "Error: Link kind missing!!!"));
	}

	// append url	
	fEndPos += fWriter->CodePointSize(&fUtf8->String()[fEndPos]);
	
	for (int i = fStartPos; i < fEndPos; i ++) {
		url.Append(fUtf8->ByteAt(i), 1);
	}		

	// calculate rectangle for url
	font_height height;
	fFont->GetHeight(&height);

	float llx, lly, urx, ury;	

	llx = fWriter->tx(fStart.x);
	urx = fWriter->tx(end.x);
	lly = fWriter->ty(fStart.y + height.descent);
	ury = fWriter->ty(end.y - height.ascent);

	// XXX: url should be in 7-bit ascii encoded!
	PDF_add_weblink(fWriter->fPdf, llx, lly, urx, ury, url.String());
}


void
Link::NextChar(int cps, float x, float y, float w)
{
	if (fContainsLink) {
		if (fPos == fStartPos) {
			fStart.Set(x, y);
		} else if (fPos == fEndPos) {
			CreateLink(BPoint(x + w, y));
			DetectUrl(fPos + cps);
		}
		fPos += cps;
	}
}

