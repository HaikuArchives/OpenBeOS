#ifndef __sk_node_info_h__
#define __sk_node_info_h__

#ifndef _BE_BUILD_H
#include <BeBuild.h>
#endif
#include <SupportDefs.h>
#include <Mime.h>
#include <Message.h>
#include <File.h>
#include <Entry.h>

#ifdef USE_OPENBEOS_NAMESPACE
namespace OpenBeOS {
#endif // USE_OPENBEOS_NAMESPACE

class BBitmap;
class BResources;


//! BNodeInfo provides file type information
/*! BNodeInfo provides a nice wrapper to all sorts of usefull meta data. 
 * Like it's mime type, the files icon and the application which will load
 * the file.
 *
 * @see <a href="http://www.opensource.org/licenses/mit-license.html">MIT</a>
 * @author <a href="mailto:mrmlk@users.sf.net"> Michael Lloyd Lee </a>
 * @author Be Inc
 * @version 0
 */
class BNodeInfo {
public:
  
  //! Uninitialized Constuctor
  /*! After created a BNodeInfo with this, you should call \sa SetTo()
   * @see SetTo(BNode *node)
   */
  BNodeInfo();
  //! The Constuctor
  /*! This just calls SetTo 
   * @param node The node to gather information on. Can be any favour */
  BNodeInfo(BNode *node);
  virtual ~BNodeInfo();

  //! Sets the node for which you want data on
  /*! @param node The node to play with
   *  @return <UL><li><code>B_OK</code>All is fine (still check InitCheck)</li>
   * <li><code>B_BAD_VALUE</code>The node was bad</li><ul>
   */
  status_t SetTo(BNode *node);
  //! Has it been created OK?
  /*! @returns <ul><li><code>B_OK</code></li>
   * <li><code>B_NO_INIT</code></li></ul>
   */
  status_t InitCheck() const;

  virtual status_t GetType(char *type) const;
  virtual status_t SetType(const char *type);
  virtual status_t GetIcon(BBitmap *icon, icon_size k = B_LARGE_ICON) const;
  virtual status_t SetIcon(const BBitmap *icon, icon_size k = B_LARGE_ICON);

  status_t GetPreferredApp(char *signature,
						   app_verb verb = B_OPEN) const;
  status_t SetPreferredApp(const char *signature,
						   app_verb verb = B_OPEN);
  status_t GetAppHint(entry_ref *ref) const;
  status_t SetAppHint(const entry_ref *ref);

  status_t GetTrackerIcon(BBitmap *icon,
						  icon_size k = B_LARGE_ICON) const;
  static status_t GetTrackerIcon(const entry_ref *ref,
								 BBitmap *icon,
								 icon_size k = B_LARGE_ICON);
private:
  friend class BAppFileInfo;
  
  virtual void _ReservedNodeInfo1(); //< FBC
  virtual void _ReservedNodeInfo2(); //< FBC
  virtual void _ReservedNodeInfo3(); //< FBC

  BNodeInfo	&operator=(const BNodeInfo &);
  BNodeInfo(const BNodeInfo &);

  BNode *fNode; //< The Node in question
  uint32 _reserved[2];
  status_t fCStatus; //< The status to return from InitCheck
};

#ifdef USE_OPENBEOS_NAMESPACE
}
#endif // USE_OPENBEOS_NAMESPACE

#endif // __sk_node_info_h__

