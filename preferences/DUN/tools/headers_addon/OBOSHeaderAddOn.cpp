/*************************************************/
// BeIDE Editor AddOn
//
// - Insert OpenBeOS Header -
/*************************************************/

#include <time.h>
#include <Alert.h>
#include <String.h>
#include "MTextAddOn.h"
#include "Headers.h"

// The string inserted to each copyright statement.
const char* pCopyrightHolders = "OpenBeOS Project";


void update_license( BString& header )
{
	// The tm_year field of the returned structure is a count
	// of the number of years since 1900.
	time_t now = time( NULL );
	struct tm* time_struct = localtime( &now );
	int year = 1900 + time_struct->tm_year;
	BString yearString;
	yearString << year;
	
	// Replace the tags in the header text with the current 
	// year and the name of the copyright holder in the copyright
	// statement that's about to be inserted.
	header.ReplaceFirst( "<year>", yearString.String() );
	header.ReplaceFirst( "<copyright holders>", pCopyrightHolders );
}

// This method serves as the interface to the add-on, so it
// must be implemented and exported.
extern "C"
{
	__declspec(dllexport) status_t perform_edit( MTextAddOn* pAddon );
}

// The first of the three types is header is intended as the
// documentation for an entire "application".
void insert_app_header( MTextAddOn* pAddon )
{
	BString appHeader( pApplicationHeader );
	update_license( appHeader );
		
	pAddon->Insert( appHeader.String() );
}

// The header that is intended to be inserted at the start of
// each class file.
void insert_class_header( MTextAddOn* pAddon )
{
	BString classHeader( pClassHeader );
	update_license( classHeader );
	
	pAddon->Insert( classHeader.String() );
}

// These are intended as documentation for each method.
void insert_method_header( MTextAddOn* pAddon )
{
	// Just insert the method header text directly. We've
	// got no fancy replacements to do.
	pAddon->Insert( pMethodHeader );
}

// ------------------------------------------------ 
// perform_edit
//
// This method presents the interface to the
// editor addon. We are given a pointer to an
// addon object, which hides the internals of the
// BeIDE editor to allow for forward compatibility.
// ------------------------------------------------ 
status_t perform_edit( MTextAddOn* pAddon )
{
	// Display an alert to the user asking which kind
	// of header they'd like to insert, and then insert it.
	char* message = "Please select the type of header you "
		"would like to insert:";
	
	BAlert* pAlert = new BAlert( "Insert Header", message,
		"Application", "Class", "Method", B_WIDTH_AS_USUAL,
		B_OFFSET_SPACING, B_IDEA_ALERT );
	
	int32 headerType = pAlert->Go();

	switch( headerType )
	{
	case 0:
		insert_app_header( pAddon );
		break;
		
	case 1:
		insert_class_header( pAddon );
		break;
		
	case 2:
		insert_method_header( pAddon );
		break;
	
	default:
		return B_ERROR;
	}
	
	return B_NO_ERROR;
}
