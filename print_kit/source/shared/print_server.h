/*

Definitions for interfacing with the PrintServer.
Parts are taken from the pr_server.h file in the OpenTracker distro.

Copyright (c) 2001 OpenBeOS. Written by I.R. Adema.

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

#ifndef _PRINT_SERVER_H
#define _PRINT_SERVER_H

	// Suites
#define B_PSRV_PRINTSERVER_SUITE	"suite/vnd.OBOS-PrintServer"
#define B_PSRV_PRINTER_SUITE		"suite/vnd.OBOS-Printer"
#define B_PSRV_JOB_SUITE			"suite/vnd.OBOS-Job"

	// FileTypes
#define B_PSRV_APP_SIGNATURE		"application/x-vnd.Be-PSRV"
#define B_PSRV_PRINTER_FILETYPE		"application/x-vnd.Be.printer"
#define B_PSRV_SPOOL_FILETYPE		"application/x-vnd.Be.printer-spool"

// Attributes of B_PSRV_SPOOL_FILETYPE
#define B_PSRV_SPOOL_ATTR_MIMETYPE			"_spool/MimeType"
#define B_PSRV_SPOOL_ATTR_PAGECOUNT			"_spool/Page Count"
#define B_PSRV_SPOOL_ATTR_DESCRIPTION		"_spool/Description"
#define B_PSRV_SPOOL_ATTR_PRINTER			"_spool/Printer"
#define B_PSRV_SPOOL_ATTR_STATUS			"_spool/Status"
#define B_PSRV_SPOOL_ATTR_ERRCODE			"_spool/_errorcode"

// Attributes of B_PSRV_PRINTER_FILETYPE
#define B_PSRV_PRINTER_ATTR_DRV_NAME		"Driver Name"
#define B_PSRV_PRINTER_ATTR_PRT_NAME		"Printer Name"
#define B_PSRV_PRINTER_ATTR_COMMENTS		"Comments"
#define B_PSRV_PRINTER_ATTR_STATE			"state"
#define B_PSRV_PRINTER_ATTR_TRANSPORT		"transport"
#define B_PSRV_PRINTER_ATTR_TRANSPORT_ADDR	"transport_address"
#define B_PSRV_PRINTER_ATTR_CNX				"connection"
#define B_PSRV_PRINTER_ATTR_PNP				"_PNP"
#define B_PSRV_PRINTER_ATTR_MDL				"_MDL"

#endif
