/********************************************************************************
/
/      File:           TranslationUtils.cpp
/
/      Description:    Utility functions for using Translation Kit
/
/      Author: Michael Wilber, Open-BeOS Translation Kit Team
/
********************************************************************************/

#include <Application.h>
#include <Bitmap.h>
#include <BitmapStream.h>
#include <File.h>
#include <MenuItem.h>
#include <Resources.h>
#include <stdlib.h>
#include <TextView.h>
#include <TranslatorFormats.h>
#include <TranslatorRoster.h>
#include <TranslationUtils.h>

BTranslationUtils::BTranslationUtils()
{
}

BTranslationUtils::~BTranslationUtils()
{
}

BTranslationUtils::BTranslationUtils(const BTranslationUtils &kUtils)
{
}

BTranslationUtils &
BTranslationUtils::operator=(const BTranslationUtils &kUtils)
{
	return *this;
}

// Get bitmap - first try as file name, then as B_TRANSLATOR_BITMAP 
// resource type from app file -- can be of any kind for which a 
// translator is installed (TGA, etc)
// NOTE: The user of this function is responsible for deleting the returned BBitmap *
BBitmap *
BTranslationUtils::GetBitmap(const char *kName, BTranslatorRoster *roster)
{
	BBitmap *pBitmap = GetBitmapFile(kName, roster);
		// Try loading a bitmap from the file named name
	
	// Try loading the bitmap as an application resource
	if (pBitmap == NULL)
		pBitmap = GetBitmap(B_TRANSLATOR_BITMAP, kName, roster);
	
	return pBitmap;
}

// Get bitmap - from app resource file only (using resource id)
// NOTE: The user of this function is responsible for deleting the returned BBitmap *
BBitmap *
BTranslationUtils::GetBitmap(uint32 type, int32 id, BTranslatorRoster *roster)
{
	BResources *pResources = BApplication::AppResources();
		// Remember: pResources must not be freed because
		// it belongs to the application
	if (pResources == NULL || pResources->HasResource(type, id) == false)
		return NULL;
	
	// Load the bitmap resource from the application file 
	// pRawData should be NULL if the resource is an
	// unknown type or not available
	size_t bitmapSize = 0;
	const void *kpRawData = pResources->LoadResource(type, id, &bitmapSize);
	if (kpRawData == NULL || bitmapSize == 0)
		return NULL;
	
	BMemoryIO memio(kpRawData, bitmapSize);
		// Put the pointer to the raw image data into a BMemoryIO object
		// so that it can be used with BTranslatorRoster->Translate() in
		// the TranslateToBitmap() function
	
	return TranslateToBitmap(&memio, type, roster);
		// Translate the data in memio to the type type using
		// the BTranslatorRoster roster
}

// Get bitmap - from app resource file only (using resource name) 
// NOTE: A resource type and name does not uniquely identify a resource in a file
// NOTE: The user of this function is responsible for deleting the returned BBitmap *
BBitmap *
BTranslationUtils::GetBitmap(uint32 type, const char *kName, BTranslatorRoster *roster)
{
	BResources *pResources = BApplication::AppResources();
		// Remember: pResources must not be freed because
		// it belongs to the application
	if (pResources == NULL || pResources->HasResource(type, kName) == false)
		return NULL;
	
	// Load the bitmap resource from the application file 
	size_t bitmapSize = 0;
	const void *kpRawData = pResources->LoadResource(type, kName, &bitmapSize);
	if (kpRawData == NULL || bitmapSize == 0)
		return NULL;
	
	BMemoryIO memio(kpRawData, bitmapSize);
		// Put the pointer to the raw image data into a BMemoryIO object so that 
		// it can be used with BTranslatorRoster->Translate() 
	
	return TranslateToBitmap(&memio, type, roster);
		// Translate the data in memio to the type type using the
		// BTranslatorRoster roster
}

// Get bitmap - from file only
// NOTE: The user of this function is responsible for deleting the returned BBitmap *
BBitmap *
BTranslationUtils::GetBitmapFile(const char *kName, BTranslatorRoster *roster)
{
	BFile bitmapFile(kName, B_READ_ONLY);
	if (bitmapFile.InitCheck() != B_OK)
		return NULL;

	return TranslateToBitmap(&bitmapFile, B_TRANSLATOR_BITMAP, roster);
		// Translate the data in memio to the type B_TRANSLATOR_BITMAP
		// using the BTranslatorRoster roster
}

// Get bitmap from entry_ref
// NOTE: The user of this function is responsible for deleting the returned BBitmap *
BBitmap *
BTranslationUtils::GetBitmap(const entry_ref *kRef, BTranslatorRoster *roster)
{
	BFile bitmapFile(kRef, B_READ_ONLY);
	if (bitmapFile.InitCheck() != B_OK)
		return NULL;

	return TranslateToBitmap(&bitmapFile, B_TRANSLATOR_BITMAP, roster);
		// Translate the data in bitmapFile to the type B_TRANSLATOR_BITMAP
		// using the BTranslatorRoster roster
}

// Get bitmap - from open file or IO type object
// NOTE: The user of this function is responsible for deleting the returned BBitmap *
BBitmap *
BTranslationUtils::GetBitmap(BPositionIO *stream, BTranslatorRoster *roster)
{	
	return TranslateToBitmap(stream, B_TRANSLATOR_BITMAP, roster);
		// Translate the data in memio to the type type
		// using the BTranslatorRoster roster
}

// This function translates the styled text in fromStream and inserts it at the end
// of the text in intoView, using the BTranslatorRoster *roster to do the translation.
// The structs that make it possible to work with the translated data are defined
// in /boot/develop/headers/be/translation/TranslatorFormats.h
status_t
BTranslationUtils::GetStyledText(BPositionIO *fromStream, BTextView *intoView,
	BTranslatorRoster *roster)
{
	if (fromStream == NULL || intoView == NULL)
		return B_BAD_VALUE;
	
	// Use default Translator if none is specified 
	if (roster == NULL) {
		roster = BTranslatorRoster::Default();
		if (roster == NULL)
			return B_ERROR;
	}

	// Translate the file from whatever format it is in the file
	// to the B_STYLED_TEXT_FORMAT, placing the translated data into mallio
	BMallocIO mallio;
	if (roster->Translate(fromStream, NULL, NULL, &mallio, B_STYLED_TEXT_FORMAT)
		< B_OK)
		return B_ERROR;
	
	// make sure there is enough data to fill the stream header
	const size_t kStreamHeaderSize = sizeof(TranslatorStyledTextStreamHeader);
	if (mallio.BufferLength() < kStreamHeaderSize)
		return B_ERROR;
	
	// copy the stream header from the mallio buffer
	TranslatorStyledTextStreamHeader stm_header =
		*((TranslatorStyledTextStreamHeader *) mallio.Buffer());
	
	// convert the stm_header.header struct to the host format
	const size_t kRecordHeaderSize = sizeof(TranslatorStyledTextRecordHeader);
	if (swap_data(B_UINT32_TYPE, &stm_header.header, kRecordHeaderSize,
		B_SWAP_BENDIAN_TO_HOST) != B_OK)
		return B_ERROR;
	if (swap_data(B_INT32_TYPE, &stm_header.version, sizeof(int32),
		B_SWAP_BENDIAN_TO_HOST) != B_OK)
		return B_ERROR;
	if (stm_header.header.magic != 'STXT')
		return B_ERROR;
	
	// copy the text header from the mallio buffer
	uint32 offset = stm_header.header.header_size + stm_header.header.data_size;
	const size_t kTextHeaderSize = sizeof(TranslatorStyledTextTextHeader);
	if (mallio.BufferLength() < offset + kTextHeaderSize)
		return B_ERROR;
	
	TranslatorStyledTextTextHeader txt_header = 
		*((TranslatorStyledTextTextHeader *) ((char *) mallio.Buffer() + offset));
	
	// convert the stm_header.header struct to the host format
	if (swap_data(B_UINT32_TYPE, &txt_header.header, kRecordHeaderSize,
		B_SWAP_BENDIAN_TO_HOST) != B_OK)
		return B_ERROR;
	if (swap_data(B_INT32_TYPE, &txt_header.charset, sizeof(int32),
		B_SWAP_BENDIAN_TO_HOST) != B_OK)
		return B_ERROR;
	if (txt_header.header.magic != 'TEXT')
		return B_ERROR;
	if (txt_header.charset != B_UNICODE_UTF8)
		return B_ERROR;
	
	offset += txt_header.header.header_size;
	if (mallio.BufferLength() < offset + txt_header.header.data_size)
		return B_ERROR;
	
	const char *pTextData = ((const char *) mallio.Buffer()) + offset;
		// point text pointer at the actual character data
	
	if (mallio.BufferLength() > offset + txt_header.header.data_size) {
		// If the stream contains information beyond the text data
		// (which means that this data is probably styled text data)

		offset += txt_header.header.data_size;
		const size_t kStyleHeaderSize  = sizeof(TranslatorStyledTextStyleHeader);
		if (mallio.BufferLength() < offset + kStyleHeaderSize)
			return B_ERROR;
		
		TranslatorStyledTextStyleHeader stl_header = 
			*((TranslatorStyledTextStyleHeader *)
				((char *) mallio.Buffer() + offset));
		if (swap_data(B_UINT32_TYPE, &stl_header.header, kRecordHeaderSize,
			B_SWAP_BENDIAN_TO_HOST) != B_OK)
			return B_ERROR;
		if (swap_data(B_UINT32_TYPE, &stl_header.apply_offset, sizeof(uint32),
			B_SWAP_BENDIAN_TO_HOST) != B_OK)
			return B_ERROR;
		if (swap_data(B_UINT32_TYPE, &stl_header.apply_length, sizeof(uint32),
			B_SWAP_BENDIAN_TO_HOST) != B_OK)
			return B_ERROR;
		if (stl_header.header.magic != 'STYL')
			return B_ERROR;
		
		offset += stl_header.header.header_size;
		if (mallio.BufferLength() < offset + stl_header.header.data_size)
			return B_ERROR;
		
		// set pRawData to the flattened run array data
		const void *kpRawData = (const void *) ((char *) mallio.Buffer() + offset);
		text_run_array *pRunArray = BTextView::UnflattenRunArray(kpRawData);
	
		if (pRunArray) {
			intoView->Insert(intoView->TextLength(), pTextData,
				txt_header.header.data_size, pRunArray);
			free(pRunArray);
			pRunArray = NULL;
		} else
			return B_ERROR;
	} else
		intoView->Insert(intoView->TextLength(), pTextData,
			txt_header.header.data_size);

	return B_NO_ERROR;
}

// This function takes styled text data from fromView and writes it to
// intoStream.  The plain text data and styled text data are combined
// when they are written to intoStream.  This is different than how
// a save operation in StyledEdit works.  With StyledEdit, it writes
// plain text data to the file, but puts the styled text data in
// the "styles" attribute.  In other words, this function writes
// styled text data to files in a manner that isn't human readable.
// 
// So, if you want to write styled text
// data to a file, and you want it to behave the way StyledEdit does,
// you want to use the BTranslationUtils::WriteStyledEditFile() function.
//
// Michael Wilber's NOTE: based on tests I've done, it looks like the parameter
// BTranslatorRoster * roster is not used at all. I passed random integers to 
// the roster pointer of Be's version of this function, and it behaved exactly the
// same way as when the roster pointer was NULL. If the roster pointer was used,
// my program probably would have crashed when I sent junk numbers to it.
status_t
BTranslationUtils::PutStyledText(BTextView *fromView, BPositionIO *intoStream,
	BTranslatorRoster *roster)
{
	if (fromView == NULL || intoStream == NULL)
		return B_BAD_VALUE;
	
	int32 textLength = fromView->TextLength();
	if (textLength < 0)
		return B_ERROR;
	
	const char *pTextData = fromView->Text();
		// its OK if the result of fromView->Text() is NULL
	
	int32 runArrayLength = 0;
	text_run_array *pRunArray = fromView->RunArray(0, textLength, &runArrayLength);
	if (pRunArray == NULL)
		return B_ERROR;
	
	int32 flatRunArrayLength = 0;
	void *pflatRunArray =
		BTextView::FlattenRunArray(pRunArray, &flatRunArrayLength);
	if (pflatRunArray == NULL) {
		free(pRunArray);
		pRunArray = NULL;
		
		return B_ERROR;
	}
	
	// Rather than use a goto, I put a whole bunch of code that
	// could error out inside of a loop, and break out of the loop
	// if there is an error. If there is no error, loop is set
	// to false.  This is so that I don't have to put free()
	// calls everywhere there could be an error.
	
	// This block of code is where I do all of the writting of the
	// data to the stream.  I've gathered all of the data that I
	// need at this point.
	bool loop = true;
	while (loop) {
		const size_t kStreamHeaderSize = sizeof(TranslatorStyledTextStreamHeader);
		TranslatorStyledTextStreamHeader stm_header;
		stm_header.header.magic = 'STXT';
		stm_header.header.header_size = kStreamHeaderSize;
		stm_header.header.data_size = 0;
		stm_header.version = 100;
		
		// convert the stm_header.header struct to the host format
		const size_t kRecordHeaderSize = sizeof(TranslatorStyledTextRecordHeader);
		if (swap_data(B_UINT32_TYPE, &stm_header.header, kRecordHeaderSize,
			B_SWAP_HOST_TO_BENDIAN) != B_OK)
			break;
		if (swap_data(B_INT32_TYPE, &stm_header.version, sizeof(int32),
			B_SWAP_HOST_TO_BENDIAN) != B_OK)
			break;
		
		const size_t kTextHeaderSize = sizeof(TranslatorStyledTextTextHeader);
		TranslatorStyledTextTextHeader txt_header;
		txt_header.header.magic = 'TEXT';
		txt_header.header.header_size = kTextHeaderSize;
		txt_header.header.data_size = textLength;
		txt_header.charset = B_UNICODE_UTF8;
		
		// convert the stm_header.header struct to the host format
		if (swap_data(B_UINT32_TYPE, &txt_header.header, kRecordHeaderSize,
			B_SWAP_HOST_TO_BENDIAN) != B_OK)
			break;
		if (swap_data(B_INT32_TYPE, &txt_header.charset, sizeof(int32),
			B_SWAP_HOST_TO_BENDIAN) != B_OK)
			break;
		
		const size_t kStyleHeaderSize = sizeof(TranslatorStyledTextStyleHeader);
		TranslatorStyledTextStyleHeader stl_header;
		stl_header.header.magic = 'STYL';
		stl_header.header.header_size = kStyleHeaderSize;
		stl_header.header.data_size = flatRunArrayLength;
		stl_header.apply_offset = 0;
		stl_header.apply_length = textLength;
		
		// convert the stl_header.header struct to the host format
		if (swap_data(B_UINT32_TYPE, &stl_header.header, kRecordHeaderSize,
			B_SWAP_HOST_TO_BENDIAN) != B_OK)
			break;
		if (swap_data(B_UINT32_TYPE, &stl_header.apply_offset, sizeof(uint32),
			B_SWAP_HOST_TO_BENDIAN) != B_OK)
			break;
		if (swap_data(B_UINT32_TYPE, &stl_header.apply_length, sizeof(uint32),
			B_SWAP_HOST_TO_BENDIAN) != B_OK)
			break;
		
		// Here, you can see the structure of the styled text data by
		// observing the order that the various structs and data are
		// written to the stream
		ssize_t amountWritten = 0;
		amountWritten = intoStream->Write(&stm_header, kStreamHeaderSize);
		if ((size_t) amountWritten != kStreamHeaderSize)
			break;
		amountWritten = intoStream->Write(&txt_header, kTextHeaderSize);
		if ((size_t) amountWritten != kTextHeaderSize)
			break;
		amountWritten = intoStream->Write(pTextData, textLength);
		if (amountWritten != textLength)
			break;
		amountWritten = intoStream->Write(&stl_header, kStyleHeaderSize);
		if ((size_t) amountWritten != kStyleHeaderSize)
			break;
		amountWritten = intoStream->Write(pflatRunArray, flatRunArrayLength);
		if (amountWritten != flatRunArrayLength)
			break;
		
		loop = false;
			// gracefully break out of the loop
	} // end of while(loop)
	
	free(pflatRunArray);
	pflatRunArray = NULL;
	free(pRunArray);
	pRunArray = NULL;
	
	if (loop)
		return B_ERROR;
	else
		return B_NO_ERROR;
}

// This function writes the styled text data from fromView
// and stores it in the file intoFile.
//
// This function is similar to PutStyledText() except that it
// only writes styled text data to files and it puts the
// plain text data in the file and stores the styled data as
// the attribute "styles".
//
// You can use PutStyledText() to write styled text data
// to files, but it writes the data in a format that isn't
// human readable.
//
// It is important to note that this function doesn't
// write files in exactly the same manner that you get
// when you do a File->Save operation in StyledEdit.
// This function doesn't write all of the attributes
// that StyledEdit does, even though it easily could.
status_t
BTranslationUtils::WriteStyledEditFile(BTextView *fromView, BFile *intoFile)
{
	if (fromView == NULL || intoFile == NULL)		
		return B_BAD_VALUE;
	
	int32 textLength = fromView->TextLength();
	if (textLength < 0)
		return B_ERROR;
	
	const char *kpTextData = fromView->Text();
	if (kpTextData == NULL && textLength != 0)
		return B_ERROR;
	
	// Write plain text data to file
	ssize_t amtWritten = intoFile->Write(kpTextData, textLength);
	if (amtWritten != textLength)
		return B_ERROR;
	
	// Write attributes
	// BEOS:TYPE
	// (this is so that the BeOS will recognize this file as a text file)
	amtWritten = intoFile->WriteAttr("BEOS:TYPE", 'MIMS', 0, "text/plain", 11);
	if ((size_t) amtWritten != 11)
		return B_ERROR;
	
	text_run_array *pRunArray = fromView->RunArray(0, fromView->TextLength());
	if (pRunArray == NULL)
		return B_ERROR;
	
	int32 runArraySize = 0;
	void *pflatRunArray = BTextView::FlattenRunArray(pRunArray, &runArraySize);
	if (pflatRunArray == NULL) {
		free(pRunArray);
		pRunArray = NULL;
		return B_ERROR;
	}
	if (runArraySize < 0) {
		free(pflatRunArray);
		pflatRunArray = NULL;
		free(pRunArray);
		pRunArray = NULL;
		return B_ERROR;
	}
	
	// This is how the styled text data is stored in the file
	// (the trick is that it isn't actually stored in the file, its stored as an attribute
	// in the file's node)
	amtWritten = intoFile->WriteAttr("styles", B_RAW_TYPE, 0, pflatRunArray, runArraySize);
	free(pflatRunArray);
	pflatRunArray = NULL;		
	free(pRunArray);
	pRunArray = NULL;
	if (amtWritten == runArraySize)
		return B_OK;
	else
		return B_ERROR;
}

// Each translator can have default settings, set by the "translations"
// control panel. You can read these settings to pass on to a translator
// using one of these functions.
BMessage *
BTranslationUtils::GetDefaultSettings(translator_id forTranslator,
	BTranslatorRoster *roster)
{
	// Use default Translator if none is specified 
	if (roster == NULL) {
		roster = BTranslatorRoster::Default();
		if (roster == NULL)
			return NULL;
	}
	
	BMessage *pMessage = new BMessage();
	if (pMessage == NULL)
		return NULL;
	
	status_t result = roster->GetConfigurationMessage(forTranslator, pMessage);
	switch (result) {
		case B_OK:
			break;
			
		case B_NO_TRANSLATOR:
			// Be's version seems to just pass an empty
			// BMessage for this case, well, in some cases anyway
			break;
			
		case B_NOT_INITIALIZED:
			delete pMessage;
			pMessage = NULL;
			break;
			
		case B_BAD_VALUE:
			delete pMessage;
			pMessage = NULL;
			break;
			
		default:
			break;
	}
	
	return pMessage;
}

// Attempts to find the translator settings for 
// the translator named translator_name with a version of
// translator_version
BMessage *
BTranslationUtils::GetDefaultSettings(const char *kTranslatorName,
	int32 translatorVersion)
{
	BTranslatorRoster *roster = BTranslatorRoster::Default();
	translator_id *translators = NULL;
	int32 numTranslators = 0;
	if (roster == NULL
		|| roster->GetAllTranslators(&translators, &numTranslators) != B_OK)
		return NULL;

	// Cycle through all of the default translators
	// looking for a translator that matches the name and version
	// that I was given
	BMessage *pMessage = NULL;
	const char *currentTranName = NULL, *currentTranInfo = NULL;
	int32 currentTranVersion = 0;
	for (int i = 0; i < numTranslators; i++) {

		if (roster->GetTranslatorInfo(translators[i], &currentTranName,
			&currentTranInfo, &currentTranVersion) == B_OK) {

			if (currentTranVersion == translatorVersion
				&& strcmp(currentTranName, kTranslatorName) == 0) {
				pMessage = GetDefaultSettings(translators[i], roster);
				break;				
			}
		}
	}
	
	delete[] translators;
	return pMessage;
}

// Envious of that "Save As" menu in ShowImage? Well, you can have your own!
// AddTranslationItems will add menu items for all translations from the
// basic format you specify (B_TRANSLATOR_BITMAP, B_TRANSLATOR_TEXT etc).
// The translator ID and format constant chosen will be added to the message
// that is sent to you when the menu item is selected.

// The following code is written by Jon Watte from
// http://www.b500.com/bepage/TranslationKit2.html
status_t
BTranslationUtils::AddTranslationItems(BMenu *intoMenu, uint32 fromType,
	const BMessage *kModel, const char *kTranslatorIdName,
	const char *kTranslatorTypeName, BTranslatorRoster *roster)
{ 
	if (roster == NULL)
		roster = BTranslatorRoster::Default();

	if (kTranslatorIdName == NULL)
		kTranslatorIdName = "be:translator";

	if (kTranslatorTypeName == NULL)
		kTranslatorTypeName = "be:type";

	translator_id * ids = NULL;
	int32 count = 0;
	status_t err = roster->GetAllTranslators(&ids, &count);
	if (err < B_OK)
		return err;

	for (int tix = 0; tix < count; tix++) {
		const translation_format *formats = NULL;
		int32 numFormats = 0;
		bool ok = false;
		err = roster->GetInputFormats(ids[tix], &formats, &numFormats);
		if (err == B_OK) {
			for (int iix = 0; iix < numFormats; iix++) {
				if (formats[iix].type == fromType) {
					ok = true;
					break;
				}
			}
		}
		if (!ok)
			continue;

		err = roster->GetOutputFormats(ids[tix], &formats, &numFormats); 
		if (err == B_OK) {
			for (int oix = 0; oix < numFormats; oix++) { 
				if (formats[oix].type != fromType) { 
					BMessage *itemmsg; 
					if (kModel)
						itemmsg = new BMessage(*kModel);
					else
						itemmsg = new BMessage(B_TRANSLATION_MENU);
					itemmsg->AddInt32(kTranslatorIdName, ids[tix]);
					itemmsg->AddInt32(kTranslatorTypeName, formats[oix].type);
					intoMenu->AddItem(
						new BMenuItem(formats[oix].name, itemmsg));
				}
			}
		}
	}

	delete[] ids;
	return B_OK;
}

// Translates the image data from pio to the type type using the
// supplied BTranslatorRoster. If BTranslatorRoster is not supplied 
// the default BTranslatorRoster is used.
// NOTE: The user of this function is responsible for deleting the returned BBitmap *
BBitmap *
BTranslationUtils::TranslateToBitmap(BPositionIO *pio, uint32 type,
	BTranslatorRoster *roster)
{
	if (pio == NULL)
		return NULL;
	
	// Use default Translator if none is specified 
	if (roster == NULL) {
		roster = BTranslatorRoster::Default();
		if (roster == NULL)
			return NULL;
	}

	// Translate the file from whatever format it is in the file
	// to the type format so that it can be stored in a BBitmap
	BBitmapStream bitmapStream;
	if (roster->Translate(pio, NULL, NULL, &bitmapStream, type) < B_OK)
		return NULL;
	
	// Detach the BBitmap from the BBitmapStream so the user
	// of this function can do what they please with it.
	BBitmap *pBitmap = NULL;
	if (bitmapStream.DetachBitmap(&pBitmap) == B_NO_ERROR)
		return pBitmap;
	else
		return NULL;
}

