/* Journal - transaction and logging
**
** Initial version by Axel Dörfler, axeld@pinc-software.de
** This file may be used under the terms of the OpenBeOS License.
*/


#include "Journal.h"
#include "Debug.h"


int32 gMaxCount = 0;


void
blockNotify(off_t blockNumber,size_t numBlocks,void *arg)
{
	PRINT(("block info called with: block %Ld, %lu blocks, volume = %p\n",blockNumber,numBlocks,arg));
}


//	#pragma mark -


Transaction::~Transaction()
{
	if (fCount == 0) {
		PRINT(("Transaction aborted or not written any blocks to it\n"));
	}
}


void
Transaction::Done()
{
	if (fCount > gMaxCount) {
		PRINT(("Transaction done with %ld blocks\n",fCount));
		gMaxCount = fCount;
	}
	fCount = -1;
}


status_t
Transaction::WriteBlocks(off_t blockNumber,const uint8 *buffer,size_t numBlocks = 1)
{
	fCount += numBlocks;
	status_t status = cached_write/*_locked*/(fVolume->Device(),blockNumber,buffer,numBlocks,fVolume->BlockSize());
	//set_blocks_info(fVolume->Device(),&blockNumber,numBlocks,blockNotify,fVolume);
	return status;
}

