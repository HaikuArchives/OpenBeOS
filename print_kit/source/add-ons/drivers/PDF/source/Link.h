#ifndef _LINK_H
#define _LINK_H

#include <String.h>
#include <Font.h>
#include <Point.h>

class PDFWriter;

class Link {
	enum kind {
		kMailtoKind,
		kHttpKind,
		kFtpKind,
		kUnknownKind
	};

	PDFWriter* fWriter;
	BString*   fUtf8;
	BFont*     fFont;
	int        fPos; // into fUtf8
		
	bool       fContainsLink;
	int        fStartPos, fEndPos;
	BPoint     fStart;
	enum kind  fKind;

	static char* fURLPrefix[];

	bool IsValidStart(const char* cp);
	bool IsValidChar(const char* cp);
	bool DetectUrlWithoutPrefix(int start);

	bool IsValidCodePoint(const char* cp, int cps);
	bool DetectUrlWithPrefix(int start);	
	void DetectUrl(int start);
	void CreateLink(BPoint end);
	
public:
	Link(PDFWriter* writer, BString* utf8, BFont* font);
	void NextChar(int cps, float x, float y, float w);
};

#endif
