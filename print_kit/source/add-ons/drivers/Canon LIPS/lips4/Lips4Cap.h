/*
 * Lips4Cap.h
 * Copyright 1999-2000 Y.Takagi. All Rights Reserved.
 */

#ifndef __LIPS4CAP_H
#define __LIPS4CAP_H

#include "PrinterCap.h"

class Lips4Cap : public PrinterCap {
public:
	Lips4Cap(const PrinterData *printer_data) : PrinterCap(printer_data) {}
	virtual int countCap(CAPID) const;
	virtual bool isSupport(CAPID) const;
	virtual const BaseCap **enumCap(CAPID) const;
};

#endif	/* __LIPS4CAP_H */
