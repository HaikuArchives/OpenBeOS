






/*###############################################################################*\
  #                                                                             #
  #        Open BeOS Server                                                     #
  #        New Media Server                                                     #
  #                                                                             #
  #        Level : Tested                                                       #
  #                                                                             #
  #        Media Kit Team Leader : Marcus Overhagen                             #
  #        Author                : Aleksander Lodwich                           #
  #                                                                             #
  #                                                                             #
  #        MODULE: Auxiliary Functions Nov 2001                                 #
\*###############################################################################*/









#include "PrivateFunctionsModule.h"
#include <Alert.h>
#include <String.h>

















//Length of a C-String====================================================
int len( char *t)
{
int i = 0;

while (t[i] > 0) ++i;
return i;
}







//Checks a string to fit a string-rule====================================
bool Private::StringFitsRule( char *Input, char *Rule)
{
int InputIndex, RuleIndex;
int InputLength = len(Input);
int RuleLength = len(Rule);
bool special_mode = false;
int last_wildcard_pos = 0;
int last_input_hook = 0;

for (InputIndex = 0, RuleIndex = 0; InputIndex < InputLength; ++InputIndex, ++RuleIndex)
	{
	if ( Rule[RuleIndex] != '?' )
		{
		if (Rule[RuleIndex] == '[')
			{
			bool alright = false;
			RuleIndex++;
			while (Rule[RuleIndex] != ']')
				{
				if (Input[InputIndex] == Rule[RuleIndex]) alright = true;
				RuleIndex++;
				}
			}
		else{
			if ( Rule[RuleIndex] == '*')
				{
				if (RuleIndex == RuleLength -1 ) return true;
				special_mode = true;
				last_wildcard_pos = RuleIndex;
				last_input_hook = InputIndex;
				RuleIndex++;
				}
			if (Input[InputIndex] != Rule[RuleIndex])
				{
				switch (special_mode)
					{
					case false: {return false;}
					case true:  {
								InputIndex = last_input_hook++;
								RuleIndex = last_wildcard_pos;
								if (InputIndex >= InputLength-1) return false;
								}
					}
				}
			}
		}
	}

if ( RuleIndex  < RuleLength )
	{
	while (Rule[RuleLength-1] == '*') RuleLength--;
	}

if ( (RuleIndex == RuleLength) && (InputIndex == InputLength))
	 return true;

return false;
}











//Simplified BAlert box to form a message box=======================================
int Private::OKBox (const char *Output)
{
int button_index;
BAlert *myAlert=new BAlert("OKBox",Output,"OK",NULL,NULL,B_WIDTH_AS_USUAL,B_EVEN_SPACING,B_INFO_ALERT);
myAlert->SetShortcut(0,B_ENTER);
button_index = myAlert->Go();
return button_index;
}

//Simplified BAlert box to form a decision box======================================
int Private::YesNoBox (const char *Output)
{
int button_index;
BAlert *myAlert=new BAlert("YesNoBox",Output,"Yes","No",NULL,B_WIDTH_AS_USUAL,B_EVEN_SPACING,B_WARNING_ALERT);
myAlert->SetShortcut(0,B_ENTER);
button_index = myAlert->Go();
return button_index;
}











//Identifies a text line within a buffer, copies it 
//and returns the length of the line================================================
int Private::CopyLine( char *source, char *target, int limit = 1000)
{
for (int i=0; i < limit; ++i)
	{
	target[i] = source[i];
	
	if (source[i] == 13)
		{
		++i;
		return i;
		}
	}
	
return limit;
}



















//Converts an integer value to a BString object
//======================================================================================

BString Private::IntegerToString(int what)
{
BString erg("");
long testarr[10] = { 1 , 10 , 100 , 1000 , 10000 , 100000 , 1000000 , 10000000 , 100000000 , 1000000000 };

bool startaddzeros;
long i,c,min;

min=0;
startaddzeros = false;

if (what < 0) {erg.SetTo("-"); what = 0 - what;}

for (i=9; i>=0; i--)
{
c = what / testarr[i];
c -= min;

switch (c)
	{
	case 0: {
			if (startaddzeros == true) erg.Append("0");
			break;}
	case 1: {startaddzeros=true; erg.Append("1");break;}
	case 2: {startaddzeros=true;erg.Append("2");break;}
	case 3: {startaddzeros=true;erg.Append("3");break;}
	case 4: {startaddzeros=true;erg.Append("4");break;}
	case 5: {startaddzeros=true;erg.Append("5");break;}
	case 6: {startaddzeros=true;erg.Append("6");break;}
	case 7: {startaddzeros=true;erg.Append("7");break;}
	case 8: {startaddzeros=true;erg.Append("8");break;}
	case 9: {startaddzeros=true;erg.Append("9");break;}
	}
min += c;	
min *= 10;
}

if (startaddzeros == false) erg.SetTo("0");
return erg;
}

