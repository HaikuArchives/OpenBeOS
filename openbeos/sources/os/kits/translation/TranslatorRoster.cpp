/*****************************************************************************/
//               File: TranslatorRoster.cpp
//              Class: BTranslatorRoster
//             Author: Michael Wilber, Translation Kit Team
//   Reimplimentation: 2002-06-11
//
// Description: This class is the guts of the translation kit, it makes the
//              whole thing happen. It bridges the applications using this
//              object with the translators that the apps need to access.
//
//
// Copyright (c) 2002 OpenBeOS Project
//
// Original Version: Copyright 1998, Be Incorporated, All Rights Reserved.
//                   Copyright 1995-1997, Jon Watte
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense, 
// and/or sell copies of the Software, and to permit persons to whom the 
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included 
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
/*****************************************************************************/

#include <TranslatorRoster.h>

// Initialize static member variable
BTranslatorRoster *BTranslatorRoster::fspDefaultTranslators = NULL;

// private unimplemented
BTranslatorRoster::BTranslatorRoster(const BTranslatorRoster &tr)
	: BArchivable()
{
	Initialize();
}

// private unimplemented
BTranslatorRoster &
BTranslatorRoster::operator=(const BTranslatorRoster &tr)
{
	return *this;
}

// Constructor that loads no translators
BTranslatorRoster::BTranslatorRoster() : BArchivable()
{
	Initialize();
}

// Loads all the translators from the directories
// specified in the "be:translator_path" named fields in model
BTranslatorRoster::BTranslatorRoster(BMessage *model) : BArchivable()
{
	Initialize();
	
	if (model) {
		BString bstr;
		for (int32 i = 0;
			model->FindString("be:translator_path", i, &bstr) == B_OK; i++) {
			AddTranslators(bstr.String());
			bstr = "";
		}
	}		
}

// initialization code that all constructors use
void BTranslatorRoster::Initialize()
{
	fpTranslators = NULL;
	fSem = create_sem(1, "BTranslatorRoster Lock");
}

// Unloads any translators that were loaded and frees 
// all memory allocated by the BTranslatorRoster 
BTranslatorRoster::~BTranslatorRoster()
{
	if (fSem > 0 && acquire_sem(fSem) == B_NO_ERROR) {
	
		// FIRST PASS: release BTranslator objects
		translator_node *pTranNode = fpTranslators;
		while (pTranNode) {
			translator_node *pRelTranNode = pTranNode;
			pTranNode = pTranNode->next;
			pRelTranNode->translator->Release();
		}
		
		// SECOND PASS: unload images and delete nodes
		// (I can't delete a BTranslator if I've deleted
		// the code for it)
		pTranNode = fpTranslators;
		while (pTranNode) {
			translator_node *pDelTranNode = pTranNode;
			pTranNode = pTranNode->next;
			
			// only try to unload actual images
			if (pDelTranNode->image >= 0)
				unload_add_on(pDelTranNode->image);
					// I may end up trying to unload the same image
					// more than once, but I don't think
					// that should be a problem
			delete[] pDelTranNode->path;
			delete pDelTranNode;
		}		
		
		fpTranslators = NULL;
	}
	
	delete_sem(fSem);
}

// Archives the BTranslatorRoster by recording its 
// loaded add-ons in the BMessage into.
// When deep == true, more data is archived from BTranslatorRoster
// than if deep == false.
//
// For More Info: See "Deep and Shallow Archives" in the
// BArchivable BeBook documentation
//
// NOTE: The deep variable appears to have no impact on
//       the data that is written to the BMessage
status_t
BTranslatorRoster::Archive(BMessage *into, bool deep) const
{
	if (!into)
		return B_BAD_VALUE;

	status_t result = B_NOT_INITIALIZED;

	if (fSem > 0 && acquire_sem(fSem) == B_OK) {
		result = BArchivable::Archive(into, deep);
		if (result == B_OK) {
			result = into->AddString("class", "BTranslatorRoster");
			
			translator_node *pTranNode = NULL;
			for (pTranNode = fpTranslators; result == B_OK && pTranNode;
				pTranNode = pTranNode->next) {
				if (pTranNode->path[0])
					result = into->AddString("be:translator_path",
						pTranNode->path);
			}
		}
		release_sem(fSem);
	}
	
	return result;
}

// Returns a new BTranslatorRoster object, allocated by 
// new and created with the version of the constructor that 
// takes a BMessage archive. However, if the archive doesn't 
// contain data for a BTranslatorRoster object, Instantiate() 
// returns NULL.
//
// The BeBook incorrectly states that this function returns
// a BTranslatorRoster. In the BArchivable BeBook documentation,
// it states that all Instantiate functions must return a
// BArchiveable *object.
//
// NOTE: static member function
BArchivable *
BTranslatorRoster::Instantiate(BMessage *from)
{
	if (!from || !validate_instantiation(from, "BTranslatorRoster"))
		return NULL;
	else
		return new BTranslatorRoster(from);
}

// Sets outCurVersion to the Translation Kit protocol 
// version number and outMinVersion to the minimum 
// protocol version number supported. Returns a human-
// readable string containing version information. 
// Currently, inAppVersion must be 
// B_TRANSLATION_CURRENT_VERSION
//
// NOTE: static
// NOTE: inAppVersion appears to be completely ignored
//       in Be's implementation of this function.
// NOTE: The constants used in this function come
//       from <TranslationDefs.h>
const char *
BTranslatorRoster::Version(int32 *outCurVersion,
	int32 *outMinVersion, int32 inAppVersion)
{
	if (!outCurVersion || !outMinVersion)
		return "";

	static char vString[50];
	static char vDate[] = __DATE__;
	if (!vString[0]) {
		sprintf(vString, "Datatypes Library v%d.%d.%d %s\n",
			B_TRANSLATION_CURRENT_VERSION/100,
			(B_TRANSLATION_CURRENT_VERSION/10)%10,
			B_TRANSLATION_CURRENT_VERSION%10,
			vDate);
	}
	*outCurVersion = B_TRANSLATION_CURRENT_VERSION;
	*outMinVersion = B_TRANSLATION_MIN_VERSION;
	return vString;
}

///////////////// Notes from Header File /////////////////////
// This call will return the "default" set of translators.
// If there is not yet a deafult set of translators, it will
// instantiate one and load translator add-ons from the
// default location (~/config/add-ons/Datatypes, and is
// modifiable with the DATATYPES environment variable)
//
///////////////// Notes from BeBook ///////////////////////////
// This returns a BTranslatorRoster loaded with the default 
// set of translators, loaded from the colon-deliminated list 
// of files and directories found in the environment variable 
// TRANSLATORS. If no such variable exists, the 
// translators are loaded from the default locations
// /boot/home/config/add-ons/Translators,
// /boot/home/config/add-ons/Datatypes, and 
// /system/add-ons/Translators. 
// The instance of BTranslatorRoster returned by this 
// function is global to the application and should not be 
// deleted. 
//
// NOTE: static
//
// IS THIS CODE THREAD SAFE? -- its probably ok after fsp... is
//                              created and assigned, but not on first call
BTranslatorRoster *
BTranslatorRoster::Default()
{
	// If the default translators have not been loaded,
	// create a new BTranslatorRoster for them, and load them.
	if (!fspDefaultTranslators) {
		fspDefaultTranslators = new BTranslatorRoster();
		fspDefaultTranslators->AddTranslators(NULL);
	}
	
	return fspDefaultTranslators;
}

// ///////////////// Header Documentation //////////////////////
// You can pass a folder (which will be scanned for loadable translators)
// or a specific translator (which will just be loaded)
// When load_path is NULL the default translators are loaded
//
// ////////////////// BeBook Documentation ///////////////////////
// Loads all the translators located in the colon-deliminated 
// list of files and directories found in load_path. All 
// specified paths must be absolute. If load_path is NULL, it 
// loads the translators from the locations specified in the 
// TRANSLATORS environment variable. If the 
// environment variable is not defined, then it loads all the 
// files in the default directories /boot/home/config/add-ons/
// Translators, /boot/home/config/add-ons/Datatypes, and /
// system/add-ons/Translators. 
//
// RETURN CODES 
//
// B_OK. Identification of inSource was successful. 
// B_BAD_VALUE. Error parsing load_path. 
// Anything else. Error loading add-ons
status_t
BTranslatorRoster::AddTranslators(const char *load_path)
{
	if (fSem <= 0)
		return fSem;
		
	status_t loadErr = B_ERROR;
	int32 nLoaded = 0;

	if (acquire_sem(fSem) == B_OK) {
		if (load_path == NULL)
			load_path = getenv("TRANSLATORS");
		if (load_path == NULL)
			load_path = kgDefaultTranslatorPath;
			
		char pathbuf[PATH_MAX];
		const char *ptr = load_path;
		const char *end = ptr;
		struct stat stbuf;
		while (*ptr != 0) {
			//	find segments specified by colons
			end = strchr(ptr, ':');
			if (end == NULL)
				end = ptr + strlen(ptr);
			if (end-ptr > PATH_MAX - 1)
				loadErr = B_BAD_VALUE;
			else {
				//	copy this segment of the path into a path, and load it
				memcpy(pathbuf, ptr, end - ptr);
				pathbuf[end - ptr] = 0;
	
				if (!stat(pathbuf, &stbuf)) {
					//	files are loaded as translators
					if (S_ISREG(stbuf.st_mode)) {
						status_t err = LoadTranslator(pathbuf);
						if (err != B_OK)
							loadErr = err;
						else
							nLoaded++;
					} else
						//	directories are scanned
						LoadDir(pathbuf, loadErr, nLoaded);
				}
			}
			ptr = end + 1;
			if (*end == 0)
				break;
		} // while (*ptr != 0)
		
		release_sem(fSem);
		
	} // if (acquire_sem(fSem) == B_OK)

	//	if anything loaded, it's not too bad
	if (nLoaded)
		loadErr = B_OK;

	return loadErr;
}

// You can add a BTranslator object you create yourself, too.
// ///////////////// Notes from BTranslator ////////////////
// When you add a BTranslator to a BTranslatorRoster, the BTranslator
// is automatically Acquire()'d. When the BTranslatorRoster is 
// deleted, its BTranslators are Release()'d. Thus, when you
// instantiate a BTranslator and add it to a BTranslatorRoster,
// you and the Roster maintain joint ownership of the object.
// To give up ownership (such that the BTranslatorRoster will
// destroy the object when the Roster itself is destroyed),
// call Release() after adding the BTranslator to the Roster. 
//
// //////////////// MY NOTES: /////////////////////////////
//
// I may have to implement BTranslator before I can
// figure this function out.
//
// This function is only listed in the header file for 
// some reason, does it even work?
//
// Is there an example of this function being used or 
// is there any mention of this function outside the 
// header?
status_t
BTranslatorRoster::AddTranslator(BTranslator *translator)
{
	if (!translator)
		return B_BAD_VALUE;

	status_t result = B_NOT_INITIALIZED;
		
	if (fSem > 0 && acquire_sem(fSem) == B_NO_ERROR) {
		// Determine if translator is already in the list
		translator_node *pNode = fpTranslators;
		while (pNode) {
			// If translator is in the list, don't add it
			if (!strcmp(pNode->translator->TranslatorName(),
				translator->TranslatorName())) {
				result = B_OK;
				break;
			}
			pNode = pNode->next;
		}
		
		// if translator is NOT in the list, add it
		if (!pNode)
			result = AddTranslatorToList(translator);
		
		release_sem(fSem);
	}

	return result;
}

////////// Notes from Header ///////////////////////////////////
// these functions call through to the translators
// when wantType is not 0, will only take into consideration
// translators that can read input data and produce output data
//
///////////////// Notes from BeBook //////////////////////////
// Identifies the media in inSource, returning a best guess of 
// the format and the translator best equipped to handle it 
// in outInfo. If inHintType or inHintMIME is specified, 
// only those translators that can accept data of the 
// specified type are searched. If inWantType is specified, 
// only those translators that can output data of that type 
// are searched. ioExtension offers an opportunity for the 
// application to specify additional configuration 
// information to the add-ons. If more than one translator 
// can identify inSource, then the one with the highest 
// quality*capability is returned. 
//
// RETURN CODES 
//
// B_OK. Identification of inSource was successful. 
// B_NO_TRANSLATOR. No suitable translator found. 
// B_NOT_INITIALIZED. Internal Translation Kit error. 
// B_BAD_VALUE. inSource or outInfo is NULL. 
// Anything else. Error operating on inSource.
//
// NOTE: virtual
status_t
BTranslatorRoster::Identify(BPositionIO *inSource,
	BMessage *ioExtension, translator_info *outInfo,
	uint32 inHintType, const char *inHintMIME,
	uint32 inWantType)
{
	if (!inSource || !outInfo)
		return B_BAD_VALUE;

	status_t result = B_NOT_INITIALIZED;
	bool bFoundMatch = false;
	
	if (fSem > 0 && acquire_sem(fSem) == B_OK) {
	
		translator_node *pTranNode = fpTranslators;
		float bestWeight = 0.0;
		for (; pTranNode; pTranNode = pTranNode->next) {

			const translation_format *format = NULL;
			translator_info tmpInfo;
			float weight = 0.0;
			bool addmatch = false;
				// eliminates need for a goto

			result = inSource->Seek(0, SEEK_SET);
			if (result == B_OK) {
			
				int32 inputFormatsCount = 0;
				const translation_format *inputFormats =
					pTranNode->translator->InputFormats(&inputFormatsCount);
				
				if (CheckFormats(inputFormats, inputFormatsCount, inHintType,
					 inHintMIME, &format)) {

					// after checking the formats for hints, we still need to make
					// sure the translator recognizes the data and can output the
					// desired format, so we call its' Identify() function.
					if (format && !pTranNode->translator->Identify(inSource,
						format, ioExtension, &tmpInfo, inWantType))
						addmatch = true;
					
				} else if (!pTranNode->translator->Identify(inSource, NULL,
					ioExtension, &tmpInfo, inWantType))
					addmatch = true;
				
				if (addmatch) {
					tmpInfo.translator = pTranNode->id;
					weight = tmpInfo.quality * tmpInfo.capability;

					if (weight > bestWeight) {
						bFoundMatch = true;
						bestWeight = weight;
						*outInfo = tmpInfo;
					}
				}
			} // if (result == B_OK)
		} // for (; pTranNode; pTranNode = pTranNode->next)
		
		if (bFoundMatch)
			result = B_NO_ERROR;
		else if (result == B_OK)
			result = B_NO_TRANSLATOR;
		
		release_sem(fSem);
	} // if (acquire_sem(fSem) == B_OK)
	
	return result;
}

// /////////////// Notes from Header ///////////////////
// Finds all translators for a type
// call delete[] on *outInfo when done
//
// //////////////// Note from BeBook ///////////////////////
// Identifies the media in inSource, returning an array of 
// valid formats and translators equipped to handle them in 
// outInfo. outNumInfo holds the number of elements in the 
// array. If inHintType or inHintMIME is specified, only 
// those translators that can accept data of the specified 
// type are searched. If inWantType is specified, only those 
// translators that can output data of that type are searched. 
// ioExtension offers an opportunity for the application to 
// specify additional configuration information to the add-
// ons. The application assumes responsibility for 
// deallocating the array. 
//
// RETURN CODES 
//
// B_OK. Identification of inSource was successful. 
// B_NO_TRANSLATOR. No suitable translators found. 
// B_NOT_INITIALIZED. Internal Translation Kit error. 
// B_BAD_VALUE. inSource, outInfo, or outNumInfo is 
// NULL. 
// Anything else. Error operating on inSource  
//
// NOTE: virtual

	static int
	compare_data(
		const void *a,
		const void *b)
	{
		register translator_info *ai = (translator_info *)a;
		register translator_info *bi = (translator_info *)b;
		return (int) (- ai->quality*ai->capability + bi->quality*bi->capability);
	}
	
status_t
BTranslatorRoster::GetTranslators(BPositionIO *inSource,
	BMessage *ioExtension, translator_info **outInfo,
	int32 *outNumInfo, uint32 inHintType,
	const char *inHintMIME, uint32 inWantType)
{
	if (!inSource || !outInfo || !outNumInfo)
		return B_BAD_VALUE;

	status_t result = B_NOT_INITIALIZED;
	
	*outInfo = NULL;
	*outNumInfo = 0;

	if (fSem > 0 && acquire_sem(fSem) == B_OK) {

		int32 physCnt = 10;
		*outInfo = new translator_info[physCnt];
		*outNumInfo = 0;

		translator_node *pTranNode = fpTranslators;
		for (; pTranNode; pTranNode = pTranNode->next) {

			const translation_format *format = NULL;
			translator_info tmpInfo;
			bool addmatch = false;
				// avoid the need for a goto

			result = inSource->Seek(0, SEEK_SET);
			if (result < B_OK) {
				// break out of the loop if error reading from source
				delete *outInfo;
				*outInfo = NULL;
				break;
			} else {
				int32 inputFormatsCount = 0;
				const translation_format *inputFormats =
					pTranNode->translator->InputFormats(&inputFormatsCount);
				
				if (CheckFormats(inputFormats, inputFormatsCount, inHintType,
					inHintMIME, &format)) {
					if (format && !pTranNode->translator->Identify(inSource, format,
						ioExtension, &tmpInfo, inWantType))
						addmatch = true;
					
				} else if (!pTranNode->translator->Identify(inSource, NULL, ioExtension, &tmpInfo,
					inWantType))
					addmatch = true;

				if (addmatch) {
					//	dynamically resize output list
					//
					if (physCnt <= *outNumInfo) {
						physCnt += 10;
						translator_info *nOut = new translator_info[physCnt];
						for (int ix = 0; ix < *outNumInfo; ix++)
							nOut[ix] = (*outInfo)[ix];
						
						delete[] *outInfo;
						*outInfo = nOut;
					}

					 // XOR to discourage taking advantage of undocumented features
					tmpInfo.translator = pTranNode->id;
					(*outInfo)[(*outNumInfo)++] = tmpInfo;
				}
			}
		} // for (; pTranNode; pTranNode = pTranNode->next)
		
		// if exited loop WITHOUT errors
		if (!pTranNode) {
		
			if (*outNumInfo > 1)
				qsort(*outInfo, *outNumInfo, sizeof(**outInfo), compare_data);
				
			if (*outNumInfo > 0)
				result = B_NO_ERROR;
			else
				result = B_NO_TRANSLATOR;
		}
			
		release_sem(fSem);
		
	} // if (acquire_sem(fSem) == B_OK)
	
	return result;
}

// ////////////// Notes from Header ///////////////
// Find all translator IDs
// call delete[] on *outList when done
// 
// /////////////////// Notes from BeBook //////////////
// Returns, in outList, an array of all the translators loaded 
// by the BTranslationRoster. The number of elements in 
// the array is placed in outCount. The application assumes 
// responsibility for deallocating the array. 
//
// RETURN CODES 
//
// B_OK. Success. 
// B_NOT_INITIALIZED. Internal Translation Kit error. 
// B_BAD_VALUE. outList or outCount is NULL. 
//
// NOTE: virtual
status_t
BTranslatorRoster::GetAllTranslators(
	translator_id **outList, int32 *outCount)
{
	if (!outList || !outCount)
		return B_BAD_VALUE;

	status_t result = B_NOT_INITIALIZED;
	
	*outList = NULL;
	*outCount = 0;

	if (fSem > 0 && acquire_sem(fSem) == B_OK) {
		//	count handlers
		translator_node *pTranNode = NULL;
		for (pTranNode = fpTranslators; pTranNode; pTranNode = pTranNode->next)
			(*outCount)++;
		*outList = new translator_id[*outCount];
		*outCount = 0;
		for (pTranNode = fpTranslators; pTranNode; pTranNode = pTranNode->next)
			(*outList)[(*outCount)++] = pTranNode->id;
			
		result = B_NO_ERROR;
		release_sem(fSem);
	} 

	return result;
}

// /////////////// Notes from Header ////////////
// Given a translator, get user-visible info
//
// ///////////// Notes from BeBook ////////////////
// Returns public information about translator 
// forTranslator. Sets outName with a short description of 
// the translator, outInfo with a longer description, and 
// outVersion with the translator's version number. 
//
// RETURN CODES 
//
// B_OK. Success. 
// B_NO_TRANSLATOR. forTranslator not a valid 
// translator_id. 
// B_NOT_INITIALIZED. Internal Translation Kit error. 
//
// NOTE: virtual
status_t
BTranslatorRoster::GetTranslatorInfo(
	translator_id forTranslator, const char **outName,
	const char **outInfo, int32 *outVersion)
{
	if (!outName || !outInfo || !outVersion)
		return B_BAD_VALUE;

	status_t result = B_NOT_INITIALIZED;
	
	if (fSem > 0 && acquire_sem(fSem) == B_OK) {
		//	find the translator we've requested
		translator_node *pTranNode = FindTranslatorNode(forTranslator);
		if (!pTranNode)
			result = B_NO_TRANSLATOR;
		else {
			*outName = pTranNode->translator->TranslatorName();
			*outInfo = pTranNode->translator->TranslatorInfo();
			*outVersion = pTranNode->translator->TranslatorVersion();
			
			result = B_NO_ERROR;
		}
		release_sem(fSem);
	}

	return result;
}

// /////////// Notes from Header /////////////////////
// Find all input formats for translator
// note that translators may choose to be "invisible" to
// the public formats, and just kick in when they
// recognize a file format by its data.
//
// ///////////////// Notes from BeBook /////////////////
// Returns an array of the published accepted input or 
// output formats for translator forTranslator. 
// outNumFormats is filled with the number of elements in 
// the array. 
//
// RETURN CODES 
//
// B_OK. Success. 
// B_NO_TRANSLATOR. forTranslator not a valid 
// translator_id. 
// B_NOT_INITIALIZED. Internal Translation Kit error. 
// B_BAD_VALUE. outFormats or outNumFormats is 
// NULL
//
// NOTE: virtual
status_t
BTranslatorRoster::GetInputFormats( translator_id forTranslator,
	const translation_format **outFormats, int32 *outNumFormats)
{
	if (!outFormats || !outNumFormats)
		return B_BAD_VALUE;

	status_t result = B_NOT_INITIALIZED;
	
	*outFormats = NULL;
	*outNumFormats = 0;

	if (fSem > 0 && acquire_sem(fSem) == B_OK) {
		//	find the translator we've requested
		translator_node *pTranNode = FindTranslatorNode(forTranslator);
		if (!pTranNode)
			result = B_NO_TRANSLATOR;
		else {
			*outFormats = pTranNode->translator->InputFormats(outNumFormats);
			result = B_NO_ERROR;
		}
		release_sem(fSem);
	}
	
	return result;
}

// ////////////// Notes from Header ///////////////
// Find all output formats for translator
// 
// ///////////////// Notes from BeBook //////////////////
// Returns an array of the published accepted input or 
// output formats for translator forTranslator. 
// outNumFormats is filled with the number of elements in 
// the array. 
//
// RETURN CODES 
//
// B_OK. Success. 
// B_NO_TRANSLATOR. forTranslator not a valid 
// translator_id. 
// B_NOT_INITIALIZED. Internal Translation Kit error. 
// B_BAD_VALUE. outFormats or outNumFormats is 
// NULL
//
// NOTE: virtual
status_t
BTranslatorRoster::GetOutputFormats(
	translator_id forTranslator,
	const translation_format **outFormats,
	int32 *outNumFormats)
{
	if (!outFormats || !outNumFormats)
		return B_BAD_VALUE;

	status_t result = B_NOT_INITIALIZED;
	
	*outFormats = NULL;
	*outNumFormats = 0;

	if (fSem > 0 && acquire_sem(fSem) == B_OK) {
		// find the translator we've requested
		translator_node *pTranNode = FindTranslatorNode(forTranslator);
		if (!pTranNode)
			result = B_NO_TRANSLATOR;
		else {
			*outFormats = pTranNode->translator->OutputFormats(outNumFormats);
			result = B_NO_ERROR;
		}
		release_sem(fSem);
	}
	
	return result;
}

// ///////////// Notes from Header ///////////////
// Morph data into form we want
// actually do some work
// -- Translate() and Identify() are thread safe (can be re-entered)
// as long as you don't call AddTranslators() or delete the object
// at the same time. Making sure you don't is up to you; there is
// no explicit lock provided. --
//
// /////////////// Notes from BeBook //////////////////
// These two translate functions carry out data conversion, 
// converting the data in inSource to type inWantoutType 
// and placing the resulting output in outDestination. inInfo 
// should always contain either the output of an Identify() 
// call or NULL. The translation uses the translator 
// identified by inInfo->infoTranslator or inTranslator as 
// appropriate. If inInfo is NULL, Translate() will call first 
// Identify() to discover the format of the input stream. 
// ioExtension, if it is not NULL, provides a 
// communication path between the translator and the 
// application. inHintType and inHintMIME, if provided, 
// are passed as hints to the translator. 
//
// RETURN CODES 
//
// B_OK. Success. 
// B_NO_TRANSLATOR. No suitable translators found. 
// B_NOT_INITIALIZED. Internal Translation Kit error. 
// B_BAD_VALUE. inSource or outSource is NULL. 
// Anything else. Error passed on from add-on. 
//
// Translate() and Identify() are thread safe (can be re-
// entered) as long as you don't call AddTranslators() 
// or delete the object at the same time. Making sure 
// you don't is up to you; there is no explicit lock 
// provided. 
//
// NOTE: virtual
status_t
BTranslatorRoster::Translate(BPositionIO * inSource,
	const translator_info *inInfo, BMessage *ioExtension,
	BPositionIO *outDestination, uint32 inWantOutType,
	uint32 inHintType, const char *inHintMIME)
{
	if (!inSource || !outDestination)
		return B_BAD_VALUE;
		
	if (fSem <= 0)
		return B_NOT_INITIALIZED;

	status_t result = B_OK;
	translator_info stat_info;

	if (!inInfo) {
		//	go look for a suitable translator
		inInfo = &stat_info;

		result = Identify(inSource, ioExtension, &stat_info, 
				inHintType, inHintMIME, inWantOutType);
			// Identify is a locked function, so it cannot be
			// called from code that is already locked
	} 
	
	if (result >= B_OK && acquire_sem(fSem) == B_OK) {
		translator_node *pTranNode = FindTranslatorNode(inInfo->translator);
		if (!pTranNode)
			result = B_NO_TRANSLATOR;
		else {
			result = inSource->Seek(0, SEEK_SET);
			if (result == B_OK)
				result = pTranNode->translator->Translate(inSource, inInfo,
					ioExtension, inWantOutType, outDestination);
		}
		release_sem(fSem);
	}

	return result;
}

// /////////////// Notes from Header ///////////////
// Make it easy to use a specific translator
//
// //////////////// Notes from BeBook ////////////////
// These two translate functions carry out data conversion, 
// converting the data in inSource to type inWantoutType 
// and placing the resulting output in outDestination. inInfo 
// should always contain either the output of an Identify() 
// call or NULL. The translation uses the translator 
// identified by inInfo->infoTranslator or inTranslator as 
// appropriate. If inInfo is NULL, Translate() will call first 
// Identify() to discover the format of the input stream. 
// ioExtension, if it is not NULL, provides a 
// communication path between the translator and the 
// application. inHintType and inHintMIME, if provided, 
// are passed as hints to the translator. 
//
// RETURN CODES 
//
// B_OK. Success. 
// B_NO_TRANSLATOR. No suitable translators found. 
// B_NOT_INITIALIZED. Internal Translation Kit error. 
// B_BAD_VALUE. inSource or outSource is NULL. 
// Anything else. Error passed on from add-on. 
//
// Translate() and Identify() are thread safe (can be re-
// entered) as long as you don't call AddTranslators() 
// or delete the object at the same time. Making sure 
// you don't is up to you; there is no explicit lock 
// provided. 
// 
// NOTE: virtual
status_t
BTranslatorRoster::Translate(translator_id inTranslator,
	BPositionIO *inSource, BMessage *ioExtension,
	BPositionIO *outDestination, uint32 inWantOutType)
{
	if (!inSource || !outDestination)
		return B_BAD_VALUE;

	status_t result = B_NOT_INITIALIZED;
	
	if (fSem > 0 && acquire_sem(fSem) == B_OK) {
		translator_node *pTranNode = FindTranslatorNode(inTranslator);
		if (!pTranNode)
			result = B_NO_TRANSLATOR;
		else {
			result = inSource->Seek(0, SEEK_SET);
			if (result == B_OK) {
				translator_info traninfo;
				result = pTranNode->translator->Identify(inSource, NULL,
					ioExtension, &traninfo, inWantOutType);	
				if (result == B_OK) {
					result = inSource->Seek(0, SEEK_SET);
					if (result == B_OK) {
						result = pTranNode->translator->Translate(inSource,
							&traninfo, ioExtension, inWantOutType,
							outDestination);
					}
				}
			}
		}
		release_sem(fSem);
	}

	return result;
}

// ////////////////// Notes from Header /////////////////
// For configuring options of the translator, a translator can support
// creating a view to cofigure the translator. The translator can save
// its preferences in the database or settings file as it sees fit.
// As a convention, the translator should always save whatever
// settings are changed when the view is deleted or hidden.
//
// ///////////////////// Notes from BeBook //////////////////
// Returns, in outView, a BView containing controls to 
// configure translator forTranslator. It is the application's 
// responsibility to attach the BView to a BWindow. The 
// initial size of the BView is given in outExtent but may be 
// resized by the application. 
//
// RETURN CODES 
//
// B_OK. Success. 
// B_NO_TRANSLATOR. forTranslator not a valid 
// translator_id. 
// B_NOT_INITIALIZED. Internal Translation Kit error. 
// B_BAD_VALUE. outView or outExtent is NULL. 
// Anything else. Error passed on from add-on.
//
// NOTE: virtual
status_t
BTranslatorRoster::MakeConfigurationView(
	translator_id forTranslator, BMessage *ioExtension,
	BView **outView, BRect *outExtent)
{
	if (!outView || !outExtent)
		return B_BAD_VALUE;

	status_t result = B_NOT_INITIALIZED;

	if (fSem > 0 && acquire_sem(fSem) == B_OK) {
		translator_node *pTranNode = FindTranslatorNode(forTranslator);
		if (!pTranNode)
			result = B_NO_TRANSLATOR;
		else
			result = pTranNode->translator->MakeConfigurationView(ioExtension,
				outView, outExtent);
			
		release_sem(fSem);
	}
	
	return result;
}

// ///////////////// Notes from Header ////////////////
// For saving settings and using them later, your app can get the
// current settings from a translator into a BMessage that you create
// and pass in empty. Pass this message (or a copy thereof) to the 
// translator later in a call to Translate() to translate using
// those settings.
//
// ///////////////////// Notes from BeBook ////////////////////
// Saves the current configuration information for translator 
// forTranslator in ioExtension. This information may be 
// flattened, unflattened, and passed to Translate() to 
// configure it. 
//
// RETURN CODES 
//
// B_OK. Success. 
// B_NO_TRANSLATOR. forTranslator not a valid 
// translator_id. 
// B_NOT_INITIALIZED. Internal Translation Kit error. 
// B_BAD_VALUE. ioExtension is NULL. 
// Anything else. Error passed on from add-on
//
// NOTE: virtual
status_t
BTranslatorRoster::GetConfigurationMessage(
	translator_id forTranslator, BMessage *ioExtension)
{
	if (!ioExtension)
		return B_BAD_VALUE;
		
	status_t result = B_NOT_INITIALIZED;

	if (fSem > 0 && acquire_sem(fSem) == B_OK) {
		translator_node *pTranNode = FindTranslatorNode(forTranslator);
		if (!pTranNode)
			result = B_NO_TRANSLATOR;
		else
			result = pTranNode->translator->GetConfigurationMessage(ioExtension);
		
		release_sem(fSem);
	}
	
	return result;
}

// Maybe this gets the entry_ref for the file that is the actual translator
status_t
BTranslatorRoster::GetRefFor(translator_id translator,
	entry_ref *out_ref)
{
	if (!out_ref)
		return B_BAD_VALUE;

	status_t result = B_NOT_INITIALIZED;
	
	if (fSem > 0 && acquire_sem(fSem) == B_OK) {
		translator_node *pTranNode = FindTranslatorNode(translator);
		if (!pTranNode)
			result = B_NO_TRANSLATOR;
		else {
			if (pTranNode->path[0] == '\0')
				result = B_ERROR;
			else 
				result = get_ref_for_path(pTranNode->path, out_ref);
		}
		release_sem(fSem);
	}
	
	return result;
}

////////////////////////////////////////////////////
////////////////////////////////////////////////////
/// PRIVATE
/// FUNCTIONS
////////////////////////////////////////////////////
////////////////////////////////////////////////////

// NOTE: SHOULD ONLY BE CALLED FROM INSIDE LOCKED CODE
translator_node *
BTranslatorRoster::FindTranslatorNode(translator_id id)
{
	translator_node *pTranNode = NULL;
	for (pTranNode = fpTranslators; pTranNode; pTranNode = pTranNode->next)
		if (pTranNode->id == id)
			break;
			
	return pTranNode;
}

// I imagine it returns error codes in the same 
// manner as the other functions
//
// I bet this function does the real work behind 
// AddTranslators
//
// The code for this must be in Datatypes somewhere
//
// I bet if you send this a directory, it will include all 
// of the translators from the dir rather than failing
//
// NOTE: SHOULD ONLY BE CALLED FROM INSIDE LOCKED CODE
status_t
BTranslatorRoster::LoadTranslator(const char *path)
{
	if (!path)
		return B_BAD_VALUE;

	//	check that this ref is not already loaded
	const char *name = strrchr(path, '/');
	if (name)
		name++;
	else
		name = path;
	for (translator_node *i = fpTranslators; i; i=i->next)
		if (!strcmp(name, i->translator->TranslatorName()))
			return B_NO_ERROR;
			//	we use name for determining whether it's loaded
			//	that is not entirely foolproof, but making SURE will be 
			//	a very slow process that I don't much care for.
			
	image_id image = load_add_on(path);
		// Load the data and code for the Translator into memory
	if (image < 0)
		return image;
	
	// Function pointer used to create post R4.5 style translators
	BTranslator *(*pMakeNthTranslator)(int32 n,image_id you,uint32 flags,...);
	
	status_t err = get_image_symbol(image, "make_nth_translator",
		B_SYMBOL_TYPE_TEXT, (void **)&pMakeNthTranslator);
	if (!err) {
		// If the translator add-on supports the post R4.5
		// translator creation mechanism
		
		BTranslator *ptran = NULL;
		// WARNING: This code assumes that the ref count on the 
		// BTranslators from MakeNth... begin at 1!!!
		// I NEED TO WRITE CODE TO TEST WHAT THE REF COUNT FOR 
		// THESE BTRANSLATORS START AS!!!!
		for (int32 n = 0; (ptran = pMakeNthTranslator(n, image, 0)); n++) {
/* DEBUG */	printf("ADD R5: %s\n", name);
			AddTranslatorToList(ptran, path, image, false);
		}
		
		return B_NO_ERROR;
		
	} else {
		// If the translator add-on is in the R4.0 / R4.5 format		
		translator_data trandata;
		
		//	find all the symbols
		err = get_image_symbol(image, "translatorName", B_SYMBOL_TYPE_DATA,
				(void **)&trandata.translatorName);
		if (!err && get_image_symbol(image, "translatorInfo",
				B_SYMBOL_TYPE_DATA, (void **)&trandata.translatorInfo))
			trandata.translatorInfo = NULL;
		long * vptr = NULL;
		if (!err)
			err = get_image_symbol(image, "translatorVersion",
					B_SYMBOL_TYPE_DATA, (void **)&vptr);
		if (!err && (vptr != NULL))
			trandata.translatorVersion = *vptr;
		if (!err && get_image_symbol(image, "inputFormats",
				B_SYMBOL_TYPE_DATA, (void **)&trandata.inputFormats))
			trandata.inputFormats = NULL;
		if (!err && get_image_symbol(image, "outputFormats",
				B_SYMBOL_TYPE_DATA, (void **)&trandata.outputFormats))
			trandata.outputFormats = NULL;
		if (!err)
			err = get_image_symbol(image, "Identify", B_SYMBOL_TYPE_TEXT,
					(void **)&trandata.Identify);
		if (!err)
			err = get_image_symbol(image, "Translate",
					B_SYMBOL_TYPE_TEXT, (void **)&trandata.Translate);
		if (!err && get_image_symbol(image, "MakeConfig",
				B_SYMBOL_TYPE_TEXT, (void **)&trandata.MakeConfig))
			trandata.MakeConfig = NULL;
		if (!err && get_image_symbol(image, "GetConfigMessage",
				B_SYMBOL_TYPE_TEXT, (void **)&trandata.GetConfigMessage))
			trandata.GetConfigMessage = NULL;

		// if add-on is not in the correct format, return with error
		if (err)
			return err;

		// add this translator to the list
		BR4xTranslator *pR4xTran = new BR4xTranslator(&trandata);
/*DEBUG*/ printf("ADD R4x: %s\n", name);
		AddTranslatorToList(pR4xTran, path, image, false);
			// do not call Acquire() on ptran because I want it to be
			// deleted the first time Release() is called on it.
			
		return B_NO_ERROR;
	}
}

// This loads all translators in a directory
// NOTE: SHOULD ONLY BE CALLED FROM INSIDE LOCKED CODE
void
BTranslatorRoster::LoadDir(const char *path, int32 &loadErr, int32 &nLoaded)
{
	if (!path) {
		loadErr = B_FILE_NOT_FOUND;
		return;
	}

	loadErr = B_OK;
	DIR *dir = opendir(path);
	if (!dir) {
		loadErr = B_FILE_NOT_FOUND;
		return;
	}
	struct dirent *dent;
	struct stat stbuf;
	char cwd[PATH_MAX] = "";
	while (NULL != (dent = readdir(dir))) {
		strcpy(cwd, path);
		strcat(cwd, "/");
		strcat(cwd, dent->d_name);
		status_t err = stat(cwd, &stbuf);

		if (!err && S_ISREG(stbuf.st_mode) &&
			strcmp(dent->d_name, ".") && strcmp(dent->d_name, "..")) {
			
			err = LoadTranslator(cwd);
			if (err == B_OK)
				nLoaded++;
			else
				loadErr = err;
		}
	}
	closedir(dir);
}

// /////////////// From DataTypes Source Code /////////////////
//	CheckFormats is a utility function that returns true if the 
//	data provided and translator info can be used to make a 
//	determination (even if that termination is negative) and 
//	false if content identification has to be done.
//
// NOTE: SHOULD ONLY BE CALLED FROM INSIDE LOCKED CODE
bool
BTranslatorRoster::CheckFormats(const translation_format *inputFormats,
	int32 inputFormatsCount, uint32 hintType, const char *hintMIME,
	const translation_format **outFormat)
{
	if (!inputFormats || inputFormatsCount <= 0 || !outFormat)
		return false;

	*outFormat = NULL;

	//	return false if we can't use hints for this module
	//
	if (!hintType && !hintMIME)
		return false;

	//	check for the length of the MIME string, since it may be just a prefix
	//	so we use strncmp().
	int mlen = 0;
	if (hintMIME)
		mlen = strlen(hintMIME);

	//	scan for suitable format
	//
	const translation_format *fmt = inputFormats;
	for (int32 i = 0; i < inputFormatsCount && fmt->type; i++, fmt++) {
		if ((fmt->type == hintType) ||
			(hintMIME && mlen && !strncmp(fmt->MIME, hintMIME, mlen))) {
			*outFormat = fmt;
			return true;
		}
	}
	//	the module did export formats, but none matched.
	//	we return true (uses formats) but set outFormat to NULL
	return true;
}

// Assumes the object is already locked!
// If bool acquire is true, BTranslator::Acquire() is called,
// if false, it is not called.
//
// I don't want to call BTranslator::Acquire() on BTranslators that
// I create inside of this class, only on BTranslators that the user
// adds to this class themselves.
//
// NOTE: SHOULD ONLY BE CALLED FROM INSIDE LOCKED CODE
status_t
BTranslatorRoster::AddTranslatorToList(BTranslator *translator)
{
	return AddTranslatorToList(translator, "", -1, true);
		// add translator to list with no add-on image to unload,
		// and call Acquire() on it
}

// NOTE: Should only be called from inside locked code
status_t
BTranslatorRoster::AddTranslatorToList(BTranslator *translator,
	const char *path, image_id image, bool acquire)
{
	if (!translator || !path)
		return B_BAD_VALUE;

	translator_node *pTranNode = new translator_node;
	
	if (acquire)
		pTranNode->translator = translator->Acquire();
	else
		pTranNode->translator = translator;
		
	if (fpTranslators)
		pTranNode->id = fpTranslators->id + 1;
	else
		pTranNode->id = 1;
	
	pTranNode->path = new char[strlen(path) + 1];
	strcpy(pTranNode->path, path);
	pTranNode->image = image;
	pTranNode->next = fpTranslators;
	fpTranslators = pTranNode;
	
	return B_OK;
}

////////////////////////////////////////////////////////
/// Private/Reserved Virtual Functions
////////////////////////////////////////////////////////

// NOTE: virtual
void
BTranslatorRoster::ReservedTranslatorRoster1()
{
}

// NOTE: virtual
void
BTranslatorRoster::ReservedTranslatorRoster2()
{
}

// NOTE: virtual
void
BTranslatorRoster::ReservedTranslatorRoster3()
{
}

// NOTE: virtual
void
BTranslatorRoster::ReservedTranslatorRoster4()
{
}

// NOTE: virtual
void
BTranslatorRoster::ReservedTranslatorRoster5()
{
}

// NOTE: virtual
void
BTranslatorRoster::ReservedTranslatorRoster6()
{
}

