#ifndef _BOOKMARK_H
#define _BOOKMARK_H

#include <String.h>
#include <Font.h>
#include <List.h>

class PDFWriter;

const int kMaxBookmarkLevels = 10;

class Bookmark {
	PDFWriter* fWriter;
	BList      fDefinitions;
	
	class Definition {
	public:
		int   fLevel;
		BFont fFont;
		Definition(int level, BFont* font);
		bool  Matches(font_family* family, font_style* style, float size) const;
	};
	
	int fLevels[kMaxBookmarkLevels+1];

	bool Find(BFont* font, int &level) const;

public:
	
	Bookmark(PDFWriter* writer);
	~Bookmark();

	// level starts with 1	
	void AddDefinition(int level, BFont* font);
	void AddBookmark(BString* text, BFont* font);
};

#endif
