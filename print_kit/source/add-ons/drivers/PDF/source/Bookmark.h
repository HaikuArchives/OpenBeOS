#ifndef _BOOKMARK_H
#define _BOOKMARK_H

#include <String.h>
#include <Font.h>
#include <List.h>

class PDFWriter;

const int kMaxBookmarkLevels = 10;

class Bookmark {
	class Definition {
	public:
		int   fLevel;
		BFont fFont;
		Definition(int level, BFont* font);
		bool  Matches(font_family* family, font_style* style, float size) const;
	};
	
	PDFWriter*         fWriter;
	TList<Definition>  fDefinitions;	

	int                fLevels[kMaxBookmarkLevels+1];

	bool Exists(const char* family, const char* style) const;
	bool Find(BFont* font, int &level) const;

public:
	
	Bookmark(PDFWriter* writer);

	// level starts with 1	
	void AddDefinition(int level, BFont* font);
	void AddBookmark(const char* text, BFont* font);
	bool Read(const char* name); // adds definitions from file
};

#endif
