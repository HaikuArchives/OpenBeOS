// Very, very basic BString header -- neither binary compatible nor complete.
// Just here to allow for a complete BNode implementation.
// To be replaced by the OpenBeOS version to be provided by the IK Team.

#ifndef	__sk_string_h__
#define	__sk_string_h__

#include <SupportDefs.h>

class BString {
public:
	BString();
	BString(const char *str);
	BString(const char *str, int32 charCount);
	BString(const BString &str);
	virtual ~BString();

	BString &SetTo(const char *str);
	BString &SetTo(const char *str, int32 charCount);
	BString &SetTo(const BString &str);

	const char *String() const;
	int32 Length() const;

	char *LockBuffer(int32 maxLength);
	BString &UnlockBuffer(int32 length = -1);

	BString &operator=(const BString& str);
	bool operator==(const BString& str);
	bool operator!=(const BString& str);

private:
	void _Init(const char *str = NULL, int32 len = -1);

private:
	char *fData;
};

#endif	// __sk_string_h__

