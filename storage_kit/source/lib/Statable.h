#ifndef __sk_statable_h__

#include <sys/types.h>
#include <sys/stat.h>
#include <SupportDefs.h>

#ifdef USE_OPENBEOS_NAMESPACE
namespace OpenBeOS {
#endif // USE_OPENBEOS_NAMESPACE
  
  struct node_ref;
  class BVolume;
  

	//! BStatable - A nice C++ wrapper to <code>\sa stat()</code>
	/*! A purly abstract class which provieds an expenive, but convenet 
	 * C++ wrapper to the posix <code>\sa stat()</code> command. 
	 *
	 * @see <a href="http://www.opensource.org/licenses/mit-license.html">MIT</a>
	 * @author <a href="mailto:mrmlk@users.sf.net"> Michael Lloyd Lee </a>
	 * @author Be Inc
	 * @version 0
	 */
  class BStatable {
	
  public:
	
	//! Returns the stat stucture for the node
	/*! @param st The returned stucture.
	 * @return <UL>
	 * <UI><code> B_OK </code> Worked fine </UI>
	 * <UI> <CODE> B_NO_MEMORY <CODE> Could not allocate the memory 
	 *   for the call</UI>
	 * <UI> <code>B_BAD_VALUE </code> The current node does not exist.</ui>
	 * <ui> <code>B_NOT_ALLOWED</code> Read only node or volume. </ui>
	 * </UL>
	 */
	virtual status_t GetStat(struct stat *st) const = 0;
	
	//! returns if the current node is a file
	bool IsFile() const;
	//! returns if the current node a directory
	bool IsDirectory() const;
	//! returns if the current node a symbolic link 
	bool IsSymLink() const;
	
	//! Returns an node_ref for the current node, 
	/*! @see GetStat() for return codes */
	status_t GetNodeRef(node_ref *ref) const;
	
	//! Returns the owner of the node,
	/*! @see GetStat() for return codes */
	status_t GetOwner(uid_t *owner) const;	
	//! Sets the owner of the node,
	/*! @see GetStat() for return codes */
	  
	status_t SetOwner(uid_t owner);
	
	//! Returns the group owner of the node,
	/*! @see GetStat() for return codes */
	status_t GetGroup(gid_t *group) const;
	
	//! Sets the group owner of the node,
	/*!  @see GetStat() for return codes */
	status_t SetGroup(gid_t group);
	
	//! Returns the permissions of the node,
	/*! @see GetStat() for return codes */
	status_t GetPermissions(mode_t *perms) const;

	//! Sets the permissions of the node,
	/*! @see GetStat() for return codes */
	status_t SetPermissions(mode_t perms);
	
	//! Get the size of the node's data (not counting attributes)
	/*! @see GetStat() for return codes */
	 status_t GetSize(off_t *size) const; 
	
	//! Returns the last time the node was modified,
	/*! @see GetStat() for return codes */
	status_t GetModificationTime(time_t *mtime) const;
	//! Sets the last time the node was modified,
	/*! @see GetStat() for return codes */
	status_t SetModificationTime(time_t mtime);
	
	
	//! Returns the time the node was created,
	/*! @see GetStat() for return codes */
	status_t GetCreationTime(time_t *ctime) const;
	//! Sets the time the node was created,
	/*! @see GetStat() for return codes */
	 status_t SetCreationTime(time_t ctime);
	
	//! Returns the time the node was accessed,
	/*! Not used.
	 * @see GetModificationTime()
	 * @see GetStat() for return codes */
	status_t GetAccessTime(time_t *atime) const;
	//! Sets the time the node was accessed,
	/*! Not used.
	 * @see GetModificationTime()
	   @see GetStat() for return codes */
	status_t SetAccessTime(time_t atime);
	
	//! Returns the volume the node lives on,
	/*! @see BVolume
	    @see GetStat() for return codes */
	status_t GetVolume(BVolume *vol) const;
	
  private:
	
	friend class BEntry;
	friend class BNode;

	
	virtual	status_t set_stat(struct stat &st, uint32 what) = 0;

	virtual	void _OhSoStatable1(); 	//< FBC
	virtual	void _OhSoStatable2(); 	//< FBC
	virtual	void _OhSoStatable3(); 	//< FBC
	uint32 _ohSoData[4]; 			//< FBC
	
  };

#ifdef USE_OPENBEOS_NAMESPACE
}
#endif // USE_OPENBEOS_NAMESPACE

#endif // __sk_statable_h__
