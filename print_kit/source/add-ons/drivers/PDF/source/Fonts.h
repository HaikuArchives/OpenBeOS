/*

PDF Writer printer driver.

Copyright (c) 2001 OpenBeOS. 

Authors: 
	Philippe Houdoin
	Simon Gauvin	
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

#ifndef FONTS_H
#define FONTS_H

#include <Archivable.h>
#include <String.h>
#include <List.h>


enum font_type 
{
	true_type_type,
	type1_type,
	unknown_type
};


class FontFile : public BArchivable
{
private:
	BString   fName;
	BString   fPath;
	int64     fSize;
	font_type fType;
	bool      fEmbed;
	BString   fSubst;  

public:
	FontFile() { }
	FontFile(const char *n, const char *p, int64 s, font_type t, bool embed) : fName(n), fPath(p), fSize(s), fType(t), fEmbed(embed) { }
	FontFile(BMessage *archive);
	~FontFile() {};

	static BArchivable* Instantiate(BMessage *archive);
	status_t Archive(BMessage *archive, bool deep = true) const;

	// accessors
	const char*		Name() const  { return fName.String(); }
	const char*		Path() const  { return fPath.String(); }
	int64			Size() const  { return fSize; }
	font_type		Type() const  { return fType; }
	bool			Embed() const { return fEmbed; }
	const char*     Subst() const { return fSubst.String(); }

	// setters
	void SetEmbed(bool embed)        { fEmbed = embed; }
	void SetSubst(const char* subst) { fSubst = subst; }
};


class Fonts : public BArchivable {
private:
	BList       fFontFiles;

	status_t	LookupFontFiles(BPath path);	

public:
	Fonts();
	Fonts(BMessage *archive);
	~Fonts();
	
	static BArchivable* Instantiate(BMessage *archive);
	status_t Archive(BMessage *archive, bool deep = true) const;

	status_t	CollectFonts();
	void        SetTo(BMessage *archive);

	FontFile*	At(int i) const { return (FontFile*)fFontFiles.ItemAt(i); }
	int32		Length() const  { return fFontFiles.CountItems(); }
};

#endif // FONTS_H
