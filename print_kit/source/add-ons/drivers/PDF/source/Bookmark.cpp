#include <Debug.h>
#include "PDFWriter.h"
#include "Bookmark.h"
#include "Scanner.h"

Bookmark::Definition::Definition(int level, BFont* font)
	: fLevel(level)
	, fFont(*font)
{	
}


bool Bookmark::Definition::Matches(font_family* family, font_style* style, float size) const
{
	font_family family0;
	font_style  style0;
	
	if (fFont.Size() != size) return false;
	
	fFont.GetFamilyAndStyle(&family0, &style0);

	return strcmp(family0, *family) == 0 &&
		strcmp(style0, *style) == 0;
}


Bookmark::Bookmark(PDFWriter* writer)
	: fWriter(writer)
{
	for (int i = 0; i < kMaxBookmarkLevels; i ++) {
		fLevels[i] = 0;
	}
}

bool Bookmark::Find(BFont* font, int& level) const
{
	font_family family;
	font_style  style;
	float       size;

	font->GetFamilyAndStyle(&family, &style);
	size = font->Size();
	
	for (int i = 0; i < fDefinitions.CountItems(); i++) {
		Definition* d = fDefinitions.ItemAt(i);
		if (d->Matches(&family, &style, size)) {
			level = d->fLevel;
			return true;
		}
	}
	return false;
}

void Bookmark::AddDefinition(int level, BFont* font)
{
	ASSERT(level > 0);
	if (!Find(font, level)) {
		fDefinitions.AddItem(new Definition(level, font));
	}
}

void Bookmark::AddBookmark(const char* text, BFont* font)
{
	int level;
	if (Find(font, level)) {
		fprintf(fWriter->fLog, "Bookmark: %s\n", text);
		int bookmark;
		BString ucs2;
		
		fWriter->ToPDFUnicode(text, ucs2);

		bookmark = PDF_add_bookmark(fWriter->fPdf, ucs2.String(), fLevels[level-1], 1);
		
		if (bookmark < 0) bookmark = 0;
		
		for (int i = level; i < kMaxBookmarkLevels; i ++) {
			fLevels[i] = bookmark;
		} 
	}
}


// Reads bookmark definitions from file

/*
File Format: Definition.
Line comment starts with '#'.

Definition = Version { Font }.
Version    = "Bookmarks" "1.0".
Font       = Level Family Style Size.
Level      = int.
Family     = String.
Style      = String.
Size       = float.
String     = '"' string '"'.
*/

bool Bookmark::Read(const char* name) {
	Scanner scnr(name);
#define BAILOUT goto Error	
	if (scnr.InitCheck() == B_OK) {
		BString s; float f; bool ok;
		ok = scnr.ReadName(&s) && scnr.ReadFloat(&f);
		if (!ok || strcmp(s.String(), "Bookmarks") != 0 || f != 1.0) BAILOUT;
		
		while (!scnr.IsEOF()) {
			float   level, size;
			BString family, style;
			ok = scnr.ReadFloat(&level) && scnr.ReadString(&family) &&
				scnr.ReadString(&style) && scnr.ReadFloat(&size) &&
				level >= 1.0 && level <= 10.0;
			
			if (!ok) BAILOUT;
			
			BFont font;
			font.SetFamilyAndStyle(family.String(), style.String());
			font.SetSize(size);
			
			AddDefinition((int)level, &font);
			
			scnr.SkipSpaces();
		}
		return true;
	}

#undef BAILOUT	
Error:	
	return false;
}