#ifndef INDEX_H
#define INDEX_H
/* Index - index access functions
**
** Initial version by Axel Dörfler, axeld@pinc-software.de
*/


#include <KernelExport.h>

class Volume;
class Inode;


class Index {
	public:
		Index(Volume *volume);
		~Index();

		status_t SetTo(const char *name);

		Inode *Node() const { return fNode; };
		uint32 Type();

	private:
		Volume		*fVolume;
		Inode		*fNode;
};

#endif	/* INDEX_H */
