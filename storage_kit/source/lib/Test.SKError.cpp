#include "SKError.h"
#include <stdio.h>

int main()
{

	// Pointer test
	try
	{
		throw new SKError(-2, "Pointer test");
		printf("This teext will never see the light of day\n");	
	}
	catch (SKError *e)
	{
		printf("Caught SKError\n");
		printf(" e.Error() == %d\n", e->Error());
		printf(" e.ErrorMessage() == %s\n", e->ErrorMessage());
		delete e;
	}
	
	return 0;	

}