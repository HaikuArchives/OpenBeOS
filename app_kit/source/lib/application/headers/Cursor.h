#ifndef _CURSOR_H
#define _CURSOR_H

#include <BeBuild.h>
#include "interface/InterfaceDefs.h"
#include <Archivable.h>

#ifdef USE_OPENBEOS_NAMESPACE
namespace OpenBeOS {
#endif

class BCursor : BArchivable {
  public:
						BCursor(const void *cursorData);
						BCursor(BMessage *data);
	virtual				~BCursor();

	virtual	status_t	Archive(BMessage *into, bool deep = true) const;
	static	BArchivable	*Instantiate(BMessage *data);

	virtual status_t	Perform(perform_code d, void *arg);

  private:

	virtual	void		_ReservedCursor1();
	virtual	void		_ReservedCursor2();
	virtual	void		_ReservedCursor3();
	virtual	void		_ReservedCursor4();

	friend class		BApplication;
	friend class		BView;

	int32				m_serverToken;
	int32				m_needToFree;
	uint32				_reserved[6];
};

#ifdef USE_OPENBEOS_NAMESPACE
}	// namespace OpenBeOS
using namespace OpenBeOS;
#endif

#endif
