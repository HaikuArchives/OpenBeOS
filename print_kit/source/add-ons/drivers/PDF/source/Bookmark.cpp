#include <Debug.h>
#include "PDFWriter.h"
#include "Bookmark.h"


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

Bookmark::~Bookmark()
{
	for (int i = 0; i < fDefinitions.CountItems(); i ++) {
		Definition* d = (Definition*)fDefinitions.ItemAt(i);
		delete d;
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
		Definition* d = (Definition*) fDefinitions.ItemAt(i);
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

void Bookmark::AddBookmark(BString* text, BFont* font)
{
	int level;
	if (Find(font, level)) {
		fprintf(fWriter->fLog, "Bookmark: %s\n", text->String());
		int bookmark;
		BString ucs2;
		
		fWriter->ToPDFUnicode(text->String(), ucs2);

		bookmark = PDF_add_bookmark(fWriter->fPdf, ucs2.String(), fLevels[level-1], 1);
		
		if (bookmark < 0) bookmark = 0;
		
		for (int i = level; i < kMaxBookmarkLevels; i ++) {
			fLevels[i] = bookmark;
		} 
	}
}
