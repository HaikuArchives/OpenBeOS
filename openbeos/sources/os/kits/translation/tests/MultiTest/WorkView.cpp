// WorkView.cpp

#include <Application.h>
#include <TranslationUtils.h>
#include <Bitmap.h>
#include <stdio.h>
#include "MultiTest.h"
#include "WorkView.h"

const char *kPath1 = "/boot/home/cloudcg.jpg";
const char *kPath2 = "/boot/home/screen1.tga";

WorkView::WorkView(BRect rect)
	: BView(rect, "Work View", B_FOLLOW_ALL, B_WILL_DRAW | B_PULSE_NEEDED)
{
	fbImage = true;
	fPath = kPath1;
}

void
WorkView::AttachedToWindow()
{
	BTranslatorRoster *pRoster = NULL;
	BBitmap *pBitmap;
	
	//pRoster = ((MultiTestApplication *) be_app)->GetTranslatorRoster();
	
	pBitmap = BTranslationUtils::GetBitmap(fPath, pRoster);
	if (pBitmap) {
		SetViewBitmap(pBitmap);
		delete pBitmap;
	}
}

void
WorkView::Pulse()
{
	if (fbImage) {
		ClearViewBitmap();
		fbImage = false;
		if (fPath == kPath1)
			fPath = kPath2;
		else
			fPath = kPath1;
	} else {
		BTranslatorRoster *pRoster = NULL;
		BBitmap *pBitmap = BTranslationUtils::GetBitmap(fPath, pRoster);
		if (pBitmap) {
			ClearViewBitmap();
			SetViewBitmap(pBitmap);
			delete pBitmap;
		} else
			printf("-- failed to get bitmap!\n");
		fbImage = true;
	}
	
	Invalidate();
}
