#include <NodeInfo.h>

#include <Node.h>

#include <be/kernel/fs_attr.h>

#define NI_BEOS "BEOS"
// I've added a BEOS hash-define incase we wish to change to OBOS
#define NI_TYPE NI_BEOS ":TYPE"
#define NI_ICON "STD_ICON"
#define NI_M_ICON NI_BEOS ":M:" NI_ICON
#define NI_L_ICON NI_BEOS ":L:" NI_ICON
#define NI_ICON_SIZE(k) ((k == B_LARGE_ICON) ? NI_L_ICON : NI_M_ICON )


BNodeInfo::BNodeInfo(BNode *node)
{
  fCStatus = SetTo(node);
}

BNodeInfo::~BNodeInfo()
{
  fCStatus = B_NO_INIT;
}

status_t
BNodeInfo::SetTo(BNode *node)
{
  fCStatus = B_BAD_VALUE;
  if(node != NULL) {
	fCStatus = node->InitCheck();
	fNode = node;
  }
  return fCStatus;
}
status_t 
BNodeInfo::InitCheck() const
{
  return fCStatus;
}

status_t 
BNodeInfo::GetType(char *type) const
{
  if(fCStatus == B_OK) {
	attr_info attrInfo;
	int error;

	error = fNode->GetAttrInfo(NI_TYPE, &attrInfo);
	if(error == B_OK) {
	  error = fNode->ReadAttr(NI_TYPE, attrInfo.type, 0, type, attrInfo.size);
	}

	return (error > B_OK ? B_OK : error);
  } else
	return B_NO_INIT;
}

status_t SetType(const char *type)
{
  return B_ERROR;
}


status_t GetIcon(BBitmap *icon, icon_size k = B_LARGE_ICON) const
{
  if(fCStatus == B_OK) {
	attr_info attrInfo;
	int error;

	error = fNode->GetAttrInfo(NI_ICON_SIZE(k), &attrInfo);
	if(error == B_OK) {

	  error = fNode->ReadAttr(NI_ICON_SIZE(k), attrInfo.type, 0, icon, 
								attrInfo.size);
	  if(error < B_OK) {
		// The file does not have an icon.
		
	  }
	}

	return (error > B_OK ? B_OK : error);
  } else
	return B_NO_INIT;
}

status_t SetIcon(const BBitmap *icon, icon_size k = B_LARGE_ICON)
{
  return B_ERROR;
}

status_t GetPreferredApp(char *signature,
						   app_verb verb = B_OPEN) const
{
  return B_ERROR;
}

status_t SetPreferredApp(const char *signature,
						   app_verb verb = B_OPEN)
{
  return B_ERROR;
}

status_t GetAppHint(entry_ref *ref) const
{
  return B_ERROR;
}

status_t SetAppHint(const entry_ref *ref)
{
  return B_ERROR;
}

status_t GetTrackerIcon(BBitmap *icon,
						icon_size k = B_LARGE_ICON) const
{
  return B_ERROR;
}


static status_t GetTrackerIcon(const entry_ref *ref,
							   BBitmap *icon,
							   icon_size k = B_LARGE_ICON)
{
  return B_ERROR;
}

void _ReservedNodeInfo1()
{
}

void _ReservedNodeInfo2()
{
}

void _ReservedNodeInfo3()
{
}

BNodeInfo &
BNodeInfo::operator=(const BNodeInfo &nodeInfo)
{
  return *this;
}

BNodeInfo(const BNodeInfo &)
{
}
