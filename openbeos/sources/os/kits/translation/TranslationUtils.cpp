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
#include <Resources.h>
#include <File.h>
#include <TranslatorRoster.h>
#include <TranslatorFormats.h> // for TranslatorStyledText* structs
#include <Bitmap.h>
#include <BitmapStream.h>
#include <MenuItem.h>
#include <TextView.h>
#include <stdio.h>
#include <stdlib.h>
#include "TranslationUtils.h"

// Constructor, I have no idea why anyone would ever call this
BTranslationUtils::BTranslationUtils()
{
}

// Destructor
BTranslationUtils::~BTranslationUtils()
{
}

// Copy Constructor
BTranslationUtils::BTranslationUtils(const BTranslationUtils &tu)
{
}

// Assignment
BTranslationUtils & BTranslationUtils::operator=(const BTranslationUtils &tu)
{
	return (*this);
}

// Get bitmap - first try as file name, then as B_TRANSLATOR_BITMAP 
// resource type from app file -- can be of any kind for which a 
// translator is installed (TGA, etc)
// NOTE: The user of this function is responsible for deleting the returned BBitmap *
BBitmap * BTranslationUtils::GetBitmap(
				const char * name,
				BTranslatorRoster * use /*= NULL*/
				)
{
	BBitmap *pBitmap = NULL;		// pointer to the BBitmap that will be returned to the user
	
	// Try loading a bitmap from the file named name 
	pBitmap = GetBitmapFile( name, use );
	
	if ( pBitmap == NULL )
	{
		// Try loading the bitmap as an application resource 
		pBitmap = GetBitmap( B_TRANSLATOR_BITMAP, name, use );	
	}
	
	return pBitmap;
}

// Get bitmap - from app resource file only (using resource id)
// NOTE: The user of this function is responsible for deleting the returned BBitmap *
BBitmap * BTranslationUtils::GetBitmap(
				uint32 type,
				int32 id,
				BTranslatorRoster * use /*= NULL*/
				)
{
	size_t outSize = 0;					// size of the bitmap
	BResources *pResources = NULL;		// pointer to the application resources
	
	// Remember: pResources must not be freed because
	// it belongs to the application
	pResources = BApplication::AppResources();
	
	// Make sure the application resources were successfully retrieved 
	if ( pResources == NULL )
	{
		return NULL;	
	}

	// 	Make sure that the resource that I am looking for is available 
	if ( pResources->HasResource( type, id ) == false )
	{
		return NULL;
	}
	
	// Load the bitmap resource from the application file 
	// (pRawData should be NULL if the resource is an unknown type or not available)
	const void *pRawData = pResources->LoadResource( type, id, &outSize );
	
	if ( pRawData == NULL )
	{
		return NULL;
	}
	
	if ( outSize == 0 )
	{
		return NULL;
	}
	
	// Put the pointer to the raw image data into a BMemoryIO object so that it 
	// can be used with BTranslatorRoster->Translate() in the TranslateToBitmap() function
	BMemoryIO memio( pRawData, outSize );
	
	// Translate the data in memio to the type type using the BTranslatorRoster use
	return TranslateToBitmap( &memio, type, use );
}

// Get bitmap - from app resource file only (using resource name) 
// NOTE: A resource type and name does not uniquely identify a resource in a file
// NOTE: The user of this function is responsible for deleting the returned BBitmap *
BBitmap * BTranslationUtils::GetBitmap(
				uint32 type,
				const char * name,
				BTranslatorRoster * use /*= NULL*/
				)
{
	size_t outSize = 0;					// size of the bitmap
	BResources *pResources = NULL;		// pointer to the application resources
	
	pResources = BApplication::AppResources();
	
	// Make sure the application resources where successfully retrieved 
	if ( pResources == NULL )
	{
		return NULL;	
	}

	// 	Make sure that the resource that I am looking for is available 
	if ( pResources->HasResource( type, name ) == false )
	{
		return NULL;
	}
	
	// Load the bitmap resource from the application file 
	// NOTE: Do I need to check the value of type to see if it is a valid type for bitmaps? 
	const void *pRawData = pResources->LoadResource( type, name, &outSize );
	
	if ( pRawData == NULL )
	{
		return NULL;
	}
	
	if ( outSize == 0 )
	{
		return NULL;
	}
	
	// Put the pointer to the raw image data into a BMemoryIO object so that it 
	// can be used with BTranslatorRoster->Translate() 
	BMemoryIO memio( pRawData, outSize );
	
	// Translate the data in memio to the type type using the BTranslatorRoster use
	return TranslateToBitmap( &memio, type, use );
}

//	Get bitmap - from file only
// NOTE: The user of this function is responsible for deleting the returned BBitmap *
BBitmap * BTranslationUtils::GetBitmapFile(
				const char * name,
				BTranslatorRoster * use /*= NULL*/)
{
	BFile file( name, B_READ_ONLY );	// the file to read the bitmap from
	
	// Make sure file was properly initialized 
	if ( file.InitCheck() != B_OK )
	{
		return NULL;
	}

	// Translate the data in memio to the type B_TRANSLATOR_BITMAP using the BTranslatorRoster use
	return TranslateToBitmap( &file, B_TRANSLATOR_BITMAP, use );
}

// Get bitmap from entry_ref
// NOTE: The user of this function is responsible for deleting the returned BBitmap *
BBitmap * BTranslationUtils::GetBitmap(
				const entry_ref * ref,
				BTranslatorRoster * use /*= NULL*/)
{
	BFile file( ref, B_READ_ONLY );		// the file to read the bitmap from
	
	// Make sure file was properly initialized 
	if ( file.InitCheck() != B_OK )
	{
		return NULL;
	}

	// Translate the data in memio to the type B_TRANSLATOR_BITMAP using the BTranslatorRoster use
	return TranslateToBitmap( &file, B_TRANSLATOR_BITMAP, use );
}

//	Get bitmap - from open file or IO type object
// NOTE: The user of this function is responsible for deleting the returned BBitmap *
BBitmap * BTranslationUtils::GetBitmap(
				BPositionIO * stream,	/*	not NULL	*/
				BTranslatorRoster * use /*= NULL*/)
{	
	// Translate the data in memio to the type type using the BTranslatorRoster use
	return TranslateToBitmap( stream, B_TRANSLATOR_BITMAP, use );
}

// Translates the image data from pio to the type type using the
// supplied BTranslatorRoster. If BTranslatorRoster is not supplied 
// the default BTranslatorRoster is used.
// NOTE: The user of this function is responsible for deleting the returned BBitmap *
BBitmap * BTranslationUtils::TranslateToBitmap(
				BPositionIO * pio,
				uint32 type,
				BTranslatorRoster * use /*= NULL*/)
{
	BBitmapStream stream;		// the stream where the bitmap data is translated to
	BBitmap *pBitmap = NULL;	// the bitmap data that is returned to the user
	
	// I don't believe that Be's implementation of the GetBitmap() functions does this check,
	// But I'm going to do it, because there is no sense in not doing it
	if ( pio == NULL )
	{
		return NULL;
	}
	
	// Use default Translator if none is specified 
	if ( use == NULL )
	{
		use = BTranslatorRoster::Default();
		
		// If for some reason BTranslatorRoster couldn't load the default translators
		if ( use == NULL )
		{
			return NULL;
		}
	}

	// Translate the file from whatever format it is in the file
	// to the type format so that it can be stored in a BBitmap
	if ( use->Translate( pio, NULL, NULL, &stream, type ) < B_OK )
	{
		return NULL;
	}
	
	// Detach the BBitmap from the BBitmapStream so the user
	// of this function can do what they please with it.
	if ( stream.DetachBitmap( &pBitmap ) == B_NO_ERROR )
	{
		return pBitmap;
	}
	
	else
	{
		return NULL;
	}
}

// This function translate the styled text in fromStream and inserts it at the end of the
// text in intoView, using the BTranslatorRoster * use to do the translation.

// BTranslatorRoster::Translate() does the real work of reading in the text and
// style data from the stream.  The bulk of this function deals with converting
// the translated data to a manner which a BTextView can work with.

// The funny thing about the default BTranslatorRoster::Translate() is that it is able to
// extract the styles attribute information from a BFile object, even though
// the attribute functionality of a BFile comes from its BNode inheritance, not its
// BPositionIO inheritance.  This means that BTranslatorRoster::Translate() must
// be casting the BPositionIO * sent to it to a BFile in order to obtain the
// styles attribute.

// The structs that make it possible to work with the translated data are defined
// in develop/headers/be/translation/TranslatorFormats.h
status_t BTranslationUtils::GetStyledText(
				BPositionIO * fromStream,
				BTextView * intoView,	/*	not NULL	*/
				BTranslatorRoster * use /*= NULL*/)
{
	BMallocIO mallio;					// The data from fromStream is translated and stored in this object.
										// I then extract the text and style data from this object.
										
	const void *pRawData = NULL;		// Pointer to the flattened text_run_array that is used to
										// get the actual style information from the translated stream
										
	const char *pTextData = NULL;		// Pointer to the plain text data found in the translated stream
	
	text_run_array *pRunArray = NULL;	// Pointer to the text_run_array created by BTextView::UnflattenRunArray()
										// using malloc().  It is used to give the text_run_array information
										// to intoView.  It must be freed with free().
	
	
	// This struct contains the data found in the very beginning
	// of the translated stream data.  This data includes:
	//
	//		stm_header.header.magic				Identifies the data as a TranslatorStyledTextStreamHeader ( should be 'STXT' )
	//		stm_header.header.header_size		Size of the header in bytes ( should be 16 )
	//		stm_header.header.data_size			Size of the header data ( should be zero )
	//		stm_header.version					Version of styled text data ( should be 100 )
	TranslatorStyledTextStreamHeader stm_header;
	
	// This struct contains the data found after the TranslatorStyledTextStreamHeader
	// This data includes:
	//
	//		txt_header.header.magic				Identifies the data as a TranslatorStyledTextTextHeader ( should be 'TEXT' )
	//		txt_header.header.header_size		Size of the header in bytes ( should be 16 )
	//		txt_header.header.data_size			Size of the text data ( size of the plain text data in bytes ( 1 byte per character ) )
	//		txt_header.charset					Identifies the data character set ( should be 0 ( B_UNICODE_UTF8 ) )
	TranslatorStyledTextTextHeader txt_header;
	
	// ( The data that follows the TranslatorStyledTextTextHeader is the plain text data )
	
	// This struct contains the data found after the plain text data
	// NOTE: This data will not always be present in the stream because
	//       some documents do not include styled text data
	// This data includes:
	//
	//		stl_header.header.magic				Identifies the data as a TranslatorStyledTextStyleHeader ( should be 'STYL' )
	//		stl_header.header.header_size		Size of the header in bytes ( should be 20 )
	//		stl_header.header.data_size			Size of the style data in bytes
	//		stl_header.apply_offset				Not sure exactly what this is, its something like the character index where the
	//											styles are first applied ( should be zero )
	//		stl_header.apply_length				Should be the same as txt_header.header.data_size
	TranslatorStyledTextStyleHeader stl_header;
	
	// This variable indicates how far I've read into the translated
	// stream data ( BMallocIO mallio )
	uint32 offset = 0;
	
	
	// The sizes of the various structs involved in the styled text data
	const size_t streamHeaderSize = sizeof( TranslatorStyledTextStreamHeader );
	const size_t textHeaderSize   = sizeof( TranslatorStyledTextTextHeader );
	const size_t recordHeaderSize = sizeof( TranslatorStyledTextRecordHeader );
	const size_t styleHeaderSize  = sizeof( TranslatorStyledTextStyleHeader );
	
	if ( fromStream == NULL )
	{
		return B_BAD_VALUE;
	}
	
	// Be's implementation of this function crashes when
	// intoView is NULL, but I figured there is no sense
	// in writing functions that crash on bad input
	// when that case can be easily avoided
	if ( intoView == NULL )
	{
		return B_BAD_VALUE;
	}
	
	// Use default Translator if none is specified 
	if ( use == NULL )
	{
		use = BTranslatorRoster::Default();
		
		// If for some reason BTranslatorRoster couldn't load the default translators
		if ( use == NULL )
		{
			return B_ERROR;
		}
	}

	// Translate the file from whatever format it is in the file
	// to the B_STYLED_TEXT_FORMAT, placing the translated data into mallio
	if ( use->Translate( fromStream, NULL, NULL, &mallio, B_STYLED_TEXT_FORMAT ) < B_OK )
	{
		return B_ERROR;
	}
	
	// make sure there is enough data to fill the stream header
	if ( mallio.BufferLength() < streamHeaderSize )
	{
		return B_ERROR;
	}
	
	// copy the stream header from the mallio buffer
	stm_header = *( (TranslatorStyledTextStreamHeader *) mallio.Buffer() );
	
	// convert the stm_header.header struct to the host format
	if ( swap_data( B_UINT32_TYPE, &stm_header.header, recordHeaderSize, B_SWAP_BENDIAN_TO_HOST ) != B_OK )
	{
		return B_ERROR;
	}
	
	// covert the stm_header.version int32 to the host format
	if ( swap_data( B_INT32_TYPE, &stm_header.version, sizeof( int32 ), B_SWAP_BENDIAN_TO_HOST ) != B_OK )
	{
		return B_ERROR;
	}
	
	// make sure stream header is valid
	if ( stm_header.header.magic != 'STXT' )
	{
		return B_ERROR;
	}
	
	// copy the text header from the mallio buffer
	offset = stm_header.header.header_size + stm_header.header.data_size;
	
	if ( mallio.BufferLength() < offset + textHeaderSize )
	{
		return B_ERROR;
	}
	
	txt_header = *( (TranslatorStyledTextTextHeader *) ( (char *) mallio.Buffer() + offset ) );
	
	// convert the stm_header.header struct to the host format
	if ( swap_data( B_UINT32_TYPE, &txt_header.header, recordHeaderSize, B_SWAP_BENDIAN_TO_HOST ) != B_OK )
	{
		return B_ERROR;
	}
	
	// covert the stm_header.version int32 to the host format
	if ( swap_data( B_INT32_TYPE, &txt_header.charset, sizeof( int32 ), B_SWAP_BENDIAN_TO_HOST ) != B_OK )
	{
		return B_ERROR;
	}
	
	if ( txt_header.header.magic != 'TEXT' )
	{
		return B_ERROR;
	}
	
	if ( txt_header.charset != B_UNICODE_UTF8 /* B_UTF8 */ )
	{
		return B_ERROR;
	}
	
	// point text pointer at the actual character data
	offset += txt_header.header.header_size;
	
	if ( mallio.BufferLength() < offset + txt_header.header.data_size )
	{
		return B_ERROR;
	}
	
	pTextData = ( (const char *) mallio.Buffer() ) + offset;
	
	// If the stream contains information beyond the text data
	// (which means that this data is probably styled text data)
	if ( mallio.BufferLength() > offset + txt_header.header.data_size )
	{		
		// read in the style header
		offset += txt_header.header.data_size;
		
		if ( mallio.BufferLength() < offset + styleHeaderSize )
		{
			return B_ERROR;
		}
		
		stl_header = *( (TranslatorStyledTextStyleHeader *) ( (char *) mallio.Buffer() + offset ) );
		
		// convert the stl_header.header struct to the host format
		if ( swap_data( B_UINT32_TYPE, &stl_header.header, recordHeaderSize, B_SWAP_BENDIAN_TO_HOST ) != B_OK )
		{
			return B_ERROR;
		}
	
		// covert the stl_header.apply_offset uint32 to the host format
		if ( swap_data( B_UINT32_TYPE, &stl_header.apply_offset, sizeof( uint32 ), B_SWAP_BENDIAN_TO_HOST ) != B_OK )
		{	
			return B_ERROR;
		}
		
		// covert the stl_header.apply_length uint32 to the host format
		if ( swap_data( B_UINT32_TYPE, &stl_header.apply_length, sizeof( uint32 ), B_SWAP_BENDIAN_TO_HOST ) != B_OK )
		{	
			return B_ERROR;
		}
		
		if ( stl_header.header.magic != 'STYL' )
		{
			return B_ERROR;
		}
		
		offset += stl_header.header.header_size;
		
		if ( mallio.BufferLength() < offset + stl_header.header.data_size )
		{
			return B_ERROR;
		}
		
		// set pRawData to the flattened run array data
		pRawData = (const void *) ( (char *) mallio.Buffer() + offset );
		pRunArray = BTextView::UnflattenRunArray( pRawData );
	
		if ( pRunArray )
		{		
			// Be's version of GetStyledText() inserts the text at the end of the file,
			// regardless of where the cursor is, so that is what I will do here
			intoView->Insert( intoView->TextLength(), pTextData, txt_header.header.data_size, pRunArray );
		
			// UnflattenRunArray() uses malloc to allocate pRunArray
			free( pRunArray );
			pRunArray = NULL;
		}
		
		else
		{
			return B_ERROR;
		}
	}
	
	else
	{
		// Be's version of GetStyledText() inserts the text at the end of the file,
		// regardless of where the cursor is, so that is what I will do here
		intoView->Insert( intoView->TextLength(), pTextData, txt_header.header.data_size );
	}

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
// BTranslatorRoster * use is not used at all. I passed random integers to 
// the use pointer of Be's version of this function, and it behaved exactly the
// same way as when the use pointer was NULL. If the use pointer was used,
// my program probably would have crashed when I sent junk numbers to it.
status_t BTranslationUtils::PutStyledText(
				BTextView * fromView,
				BPositionIO * intoStream,
				BTranslatorRoster * use /*= NULL*/)
{
	const char *pTextData = NULL;		// pointer to the plain text data to be written to the stream
	
	int32	textLength = 0,				// length of the plain text data
			runArrayLength = 0,			// length of the text_run_array
			flatRunArrayLength = 0;		// length of the flattened text_run_array
			
	text_run_array *pRunArray = NULL;	// pointer to a copy of fromView's text_run_array
										// this data must be freed with free()
	
	void *pflatRunArray = NULL;			// pointer to the flattened version of fromView's text_run_array
										// this data must be freed with free()
										// (this is how the styled data is stored in the file)
	
	// Set GetStyledText() for information on
	// what these structs are for
	TranslatorStyledTextStreamHeader stm_header;
	TranslatorStyledTextTextHeader txt_header;
	TranslatorStyledTextStyleHeader stl_header;
	
	// The sizes of the various structs involved in the styled text data
	const size_t streamHeaderSize = sizeof( TranslatorStyledTextStreamHeader );
	const size_t textHeaderSize   = sizeof( TranslatorStyledTextTextHeader );
	const size_t recordHeaderSize = sizeof( TranslatorStyledTextRecordHeader );
	const size_t styleHeaderSize  = sizeof( TranslatorStyledTextStyleHeader );
	
	// amount of data written to intoStream
	ssize_t amountWritten = 0;
	
	bool loop = true;
	
	if ( fromView == NULL )
	{
		return B_BAD_VALUE;
	}
	
	if ( intoStream == NULL )
	{
		return B_BAD_VALUE;
	}
	
	textLength = fromView->TextLength();
	
	if ( textLength < 0 )
	{
		return B_ERROR;
	}
	
	// its OK if pTextData is NULL
	pTextData = fromView->Text();
	
	pRunArray = fromView->RunArray( 0, textLength, &runArrayLength );
	
	if ( pRunArray == NULL )
	{
		return B_ERROR;
	}
	
	pflatRunArray = BTextView::FlattenRunArray( pRunArray, &flatRunArrayLength );
	
	if ( pflatRunArray == NULL )
	{
		free( pRunArray );
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
	while ( loop )
	{
		////////
		// TranslatorStyledTextStreamHeader
		////////
		stm_header.header.magic = 'STXT';
		stm_header.header.header_size = streamHeaderSize;
		stm_header.header.data_size = 0;
		stm_header.version = 100;
		
		// convert the stm_header.header struct to the host format
		if ( swap_data( B_UINT32_TYPE, &stm_header.header, recordHeaderSize, B_SWAP_HOST_TO_BENDIAN ) != B_OK )
		{
			break;
		}
		
		// covert the stm_header.version int32 to the host format
		if ( swap_data( B_INT32_TYPE, &stm_header.version, sizeof( int32 ), B_SWAP_HOST_TO_BENDIAN ) != B_OK )
		{
			break;
		}
		
		////////
		// TranslatorStyledTextTextHeader
		////////
		txt_header.header.magic = 'TEXT';
		txt_header.header.header_size = textHeaderSize;
		txt_header.header.data_size = textLength;
		txt_header.charset = B_UNICODE_UTF8; // B_UTF8 ?
		
		// convert the stm_header.header struct to the host format
		if ( swap_data( B_UINT32_TYPE, &txt_header.header, recordHeaderSize, B_SWAP_HOST_TO_BENDIAN ) != B_OK )
		{
			break;
		}
		
		// covert the stm_header.version int32 to the host format
		if ( swap_data( B_INT32_TYPE, &txt_header.charset, sizeof( int32 ), B_SWAP_HOST_TO_BENDIAN ) != B_OK )
		{
			break;
		}
		
		////////
		// TranslatorStyledTextStyleHeader
		////////
		stl_header.header.magic = 'STYL';
		stl_header.header.header_size = styleHeaderSize;
		stl_header.header.data_size = flatRunArrayLength;
		stl_header.apply_offset = 0;
		stl_header.apply_length = textLength;
		
		// convert the stl_header.header struct to the host format
		if ( swap_data( B_UINT32_TYPE, &stl_header.header, recordHeaderSize, B_SWAP_HOST_TO_BENDIAN ) != B_OK )
		{
			break;
		}
		
		// covert the stl_header.apply_offset uint32 to the host format
		if ( swap_data( B_UINT32_TYPE, &stl_header.apply_offset, sizeof( uint32 ), B_SWAP_HOST_TO_BENDIAN ) != B_OK )
		{	
			break;
		}
			
		// covert the stl_header.apply_length uint32 to the host format
		if ( swap_data( B_UINT32_TYPE, &stl_header.apply_length, sizeof( uint32 ), B_SWAP_HOST_TO_BENDIAN ) != B_OK )
		{	
			break;
		}
		
		// Here, you can see the structure of the styled text data by
		// observing the order that the various structs and data are
		// written to the stream
		
		amountWritten = intoStream->Write( &stm_header, streamHeaderSize );
		if ( (size_t) amountWritten != streamHeaderSize )
		{
			break;
		}
		
		amountWritten = intoStream->Write( &txt_header, textHeaderSize );
		if ( (size_t) amountWritten != textHeaderSize )
		{
			break;
		}
		
		amountWritten = intoStream->Write( pTextData, textLength );
		if ( amountWritten != textLength )
		{
			break;
		}
		
		amountWritten = intoStream->Write( &stl_header, styleHeaderSize );
		if ( (size_t) amountWritten != styleHeaderSize )
		{
			break;
		}
		
		amountWritten = intoStream->Write( pflatRunArray, flatRunArrayLength );
		if ( amountWritten != flatRunArrayLength )
		{
			break;
		}
		
		// gracefully break out of the loop
		loop = false;
	}
	
	free( pflatRunArray );
	pflatRunArray = NULL;
	
	free( pRunArray );
	pRunArray = NULL;
	
	if ( loop )
	{
		return B_ERROR;
	}
	
	else
	{	
		return B_NO_ERROR;
	}
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
status_t BTranslationUtils::WriteStyledEditFile(
				BTextView * fromView,
				BFile * intoFile)
{
	const char	*pTextData = NULL,			// pointer to the plain text data
				*beType = "text/plain";		// the mime type of the file to be written
				
	int32	textLength = 0,					// length of the plain text
			runArraySize = 0;				// size of fromView's text_run_array
			
	ssize_t amtWritten = 0;					// amount of data written to intoFile
	size_t beTypeLength = 11; 				// length of the string beType, including the
											// '\0' character.
	
	text_run_array *pRunArray = NULL;		// pointer to a copy of fromView's text_run_array
											// this variable must be freed with free()
											
	void *pflatRunArray = NULL;				// pointer to a flattened copy of fromView's text_run_array
											// this variable must be freed with free()
											// (this is how the styled data is stored in the file)
	
	if ( fromView == NULL )
	{		
		return B_BAD_VALUE;
	}
	
	if ( intoFile == NULL )
	{		
		return B_BAD_VALUE;
	}
	
	textLength = fromView->TextLength();
	
	if ( textLength < 0 )
	{		
		return B_ERROR;
	}
	
	pTextData = fromView->Text();
	
	if ( pTextData == NULL && textLength != 0 )
	{		
		return B_ERROR;
	}
	
	// Write plain text data to file
	amtWritten = intoFile->Write( pTextData, textLength );
	
	if ( amtWritten != textLength )
	{		
		return B_ERROR;
	}
	
	// Write attributes
	
	// BEOS:TYPE
	// (this is so that the BeOS will recognize this file as a text file)
	amtWritten = intoFile->WriteAttr( "BEOS:TYPE", 'MIMS', 0, beType, beTypeLength );
	
	if ( (size_t) amtWritten != beTypeLength )
	{		
		return B_ERROR;
	}
	
	pRunArray = fromView->RunArray( 0, fromView->TextLength() );
	
	if ( pRunArray == NULL )
	{		
		return B_ERROR;
	}
	
	pflatRunArray = BTextView::FlattenRunArray( pRunArray, &runArraySize );
	
	if ( pflatRunArray == NULL )
	{
		free( pRunArray );
		pRunArray = NULL;
		
		return B_ERROR;
	}
	
	if ( runArraySize < 0 )
	{
		free( pflatRunArray );
		pflatRunArray = NULL;
		
		free( pRunArray );
		pRunArray = NULL;
		
		return B_ERROR;
	}
	
	// This is how the styled text data is stored in the file
	// (the trick is that it isn't actually stored in the file, its stored as an attribute
	// in the file's node)
	amtWritten = intoFile->WriteAttr( "styles", B_RAW_TYPE, 0, pflatRunArray, runArraySize );
	
	if ( amtWritten != runArraySize )
	{
		free( pflatRunArray );
		pflatRunArray = NULL;
		
		free( pRunArray );
		pRunArray = NULL;
		
		return B_ERROR;
	}
	
	free( pflatRunArray );
	pflatRunArray = NULL;
		
	free( pRunArray );
	pRunArray = NULL;
	
	return B_OK;
}

/* Each translator can have default settings, set by the "translations" */
/* control panel. You can read these settings to pass on to a translator */
/* using one of these functions. */
BMessage * BTranslationUtils::GetDefaultSettings(
				translator_id for_translator,
				BTranslatorRoster * use /*= NULL*/)
{
	BMessage *pMessage = NULL;
	status_t result;
	
	// Use default Translator if none is specified 
	if ( use == NULL )
	{
		use = BTranslatorRoster::Default();
		
		// If for some reason BTranslatorRoster couldn't load the default translators
		if ( use == NULL )
		{
			return NULL;
		}
	}
	
	pMessage = new BMessage();
	if ( pMessage == NULL )
	{
		return NULL;
	}
	
	result = use->GetConfigurationMessage( for_translator, pMessage );
	
	switch ( result )
	{
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
BMessage * BTranslationUtils::GetDefaultSettings(
				const char * translator_name,
				int32 translator_version)
{
	BMessage *pMessage = NULL;
	BTranslatorRoster *roster = BTranslatorRoster::Default();
	const char *currentTranName = NULL, *currentTranInfo = NULL;
	int32 i = 0, currentTranVersion = 0, numTranslators = 0;
	translator_id *translators = NULL;
	
	if ( roster == NULL )
	{	
		return NULL;
	}
	
	if ( roster->GetAllTranslators( &translators, &numTranslators ) != B_OK )
	{		
		return NULL;
	}
	
	// Cycle through all of the default translators
	// looking for a translator that matches the name and version
	// that I was given
	for ( i = 0; i < numTranslators; i++ )
	{
		if ( roster->GetTranslatorInfo( translators[i], &currentTranName, &currentTranInfo, &currentTranVersion ) == B_OK )
		{
			if ( currentTranVersion == translator_version && strcmp( currentTranName, translator_name ) == 0 )
			{
				pMessage = GetDefaultSettings( translators[i], roster );
				
				break;				
			}
		}
	}
	
	delete[] translators;
	
	return pMessage;
}

/* Envious of that "Save As" menu in ShowImage? Well, you can have your own! */
/* AddTranslationItems will add menu items for all translations from the */
/* basic format you specify (B_TRANSLATOR_BITMAP, B_TRANSLATOR_TEXT etc). */
/* The translator ID and format constant chosen will be added to the message */
/* that is sent to you when the menu item is selected. */

// The following code is written by Jon Watte from
// http://www.b500.com/bepage/TranslationKit2.html
status_t BTranslationUtils::AddTranslationItems(
				BMenu * intoMenu,
				uint32 from_type,
				const BMessage * model /*= NULL*/,	/* default B_TRANSLATION_MENU */
				const char * translator_id_name /*= NULL*/, /* default "be:translator" */
				const char * translator_type_name /*= NULL*/, /* default "be:type" */
				BTranslatorRoster * use /*= NULL*/)
{ 
        if (use == NULL) { 
                use = BTranslatorRoster::Default(); 
        } 
        if (translator_id_name == NULL) { 
                translator_id_name = "be:translator"; 
        } 
        if (translator_type_name == NULL) { 
                translator_type_name = "be:type"; 
        } 
        translator_id * ids = NULL; 
        int32 count = 0; 
        status_t err = use->GetAllTranslators(&ids, &count); 
        if (err < B_OK) return err; 
        for (int tix=0; tix<count; tix++) { 
                const translation_format * formats = NULL; 
                int32 num_formats = 0; 
                bool ok = false; 
                err = use->GetInputFormats(ids[tix], &formats, &num_formats); 
                if (err == B_OK) for (int iix=0; iix<num_formats; iix++) { 
                        if (formats[iix].type == from_type) { 
                                ok = true; 
                                break; 
                        } 
                } 
                if (!ok) continue; 
                err = use->GetOutputFormats(ids[tix], &formats, &num_formats); 
                if (err == B_OK) for (int oix=0; oix<num_formats; oix++) { 
                        if (formats[oix].type != from_type) { 
                                BMessage * itemmsg; 
                                if (model) { 
                                        itemmsg = new BMessage(*model); 
                                } 
                                else { 
                                        itemmsg = new BMessage(B_TRANSLATION_MENU); 
                                } 
                                itemmsg->AddInt32(translator_id_name, ids[tix]); 
                                itemmsg->AddInt32(translator_type_name, formats[oix].type); 
                                intoMenu->AddItem(new BMenuItem(formats[oix].name, itemmsg)); 
                        } 
                } 
        } 
        delete[] ids; 
        return B_OK; 
}

