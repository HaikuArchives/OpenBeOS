#include <stdio.h>
#include "Path.h"

void printfDivider()
{
	for (int i = 0; i < 70; i++)
		printf("-");
	printf("\n");
}


int main()
{
	BPath path;
	
	const char noLeadingSlash[] = "Test/For/Fun";
	const char leadingSlashSlash[] = "//Test/Forever...";
	const char leadingDotSlash[] = "./Test/Because";
	const char leadingDotDotSlash[]	= "../Test/Because/";
	const char trailingSlash[] = "/Here/We/Go/Again/";
	const char trailingSlashSlash[] = "/And/Again//";
	const char trailingSlashDot[] = "/And/Again/.";
	const char trailingSlashDotDot[] = "/And/One/More/Time/..";
	const char embeddedSlashSlash[] = "Look/What/We//Have/Here";
	const char embeddedDotSlash[] = "Look/For/./Redundancy";
	const char embeddedDotDotSlash[] = "Look/Real/../Real/Hard";
	const char noNormalization1[] = "/This/Is/Just/Fine/Thank/You/Very/Much";
	const char noNormalization2[] = "/This/.Is/Just./Fine.dir/Thank/You/Very/Much.txt";
	const char noNormalization3[] = "/Th.is/Is../Jus.t/F..ine/Thank/You/Very/Much..";
	const char noNormalization4[] = "/This/Is/Just/Fine/Thank/You/Very/Much.";
	
	printfDivider();
	printf("Following should all be true:\n");
	printfDivider();
	printf("noLeadingSlash -- %s -- %s\n", noLeadingSlash, path.MustNormalize(noLeadingSlash) ? "true" : "false");
	printf("leadingSlashSlash -- %s -- %s\n", leadingSlashSlash, path.MustNormalize(leadingSlashSlash) ? "true" : "false");
	printf("leadingDotSlash -- %s -- %s\n", leadingDotSlash, path.MustNormalize(leadingDotSlash) ? "true" : "false");
	printf("leadingDotDotSlash -- %s -- %s\n", leadingDotDotSlash, path.MustNormalize(leadingDotDotSlash) ? "true" : "false");
	printf("trailingSlash -- %s -- %s\n", trailingSlash, path.MustNormalize(trailingSlash) ? "true" : "false");
	printf("trailingSlashSlash -- %s -- %s\n", trailingSlashSlash, path.MustNormalize(trailingSlashSlash) ? "true" : "false");
	printf("trailingSlashDot -- %s -- %s\n", trailingSlashDot, path.MustNormalize(trailingSlashDot) ? "true" : "false");
	printf("trailingSlashDotDot -- %s -- %s\n", trailingSlashDotDot, path.MustNormalize(trailingSlashDotDot) ? "true" : "false");
	printf("embeddedSlashSlash -- %s -- %s\n", embeddedSlashSlash, path.MustNormalize(embeddedSlashSlash) ? "true" : "false");
	printf("embeddedDotSlash -- %s -- %s\n", embeddedDotSlash, path.MustNormalize(embeddedDotSlash) ? "true" : "false");
	printf("embeddedDotDotSlash -- %s -- %s\n", embeddedDotDotSlash, path.MustNormalize(embeddedDotDotSlash) ? "true" : "false");

	printf("\n");
	printfDivider();
	printf("Following should all be false:\n");
	printfDivider();
	printf("noNormalization1 -- %s -- %s\n", noNormalization1, path.MustNormalize(noNormalization1) ? "true" : "false");
	printf("noNormalization2 -- %s -- %s\n", noNormalization2, path.MustNormalize(noNormalization2) ? "true" : "false");
	printf("noNormalization3 -- %s -- %s\n", noNormalization3, path.MustNormalize(noNormalization3) ? "true" : "false");
	printf("noNormalization4 -- %s -- %s\n", noNormalization4, path.MustNormalize(noNormalization4) ? "true" : "false");
	
	return 0;
}