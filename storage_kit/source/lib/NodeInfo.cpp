#include <NodeInfo.h>

BNodeInfo::BNodeInfo(BNode *node)
{
  
}

BNodeInfo::~BNodeInfo()
{
}

status_t
BNodeInfo::SetTo(BNode *node)
{
  return B_BAD_VALUE;
}
status_t 
BNodeInfo::InitCheck() const
{
  return B_ERROR;
}

status_t 
BNodeInfo::GetType(char *type) const
{
  return B_ERROR;
}

status_t SetType(const char *type)
{
  return B_ERROR;
}

status_t GetIcon(BBitmap *icon, icon_size k = B_LARGE_ICON) const
{
  return B_ERROR;
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
