#include "Printer.h"


#include "pr_server.h"

	// BeOS API
#include <PropertyInfo.h>
#include <Message.h>
#include <AppDefs.h>

static property_info prop_list[] = {
	{ "Name", { B_GET_PROPERTY }, { B_DIRECT_SPECIFIER },
		"Get name of printer" }, 
	{ "TransportAddon", { B_GET_PROPERTY }, { B_DIRECT_SPECIFIER },
		"Get name of the transport add-on used for this printer" },
	{ "TransportConfig", { B_GET_PROPERTY }, { B_DIRECT_SPECIFIER },
		"Get the transport configuration for this printer" },
	{ "PrinterAddon", { B_GET_PROPERTY }, { B_DIRECT_SPECIFIER },
		"Get name of the printer add-on used for this printer" },
	{ "Comments", { B_GET_PROPERTY }, { B_DIRECT_SPECIFIER },
		"Get comments about this printer" },
	{ 0 } // terminate list 
};

void Printer::HandleScriptingCommand(BMessage* msg)
{
	status_t rc = B_OK;
	BString propName;
	BString result;
	BMessage spec;
	int32 idx;

	if (msg->GetCurrentSpecifier(&idx,&spec) == B_OK &&
		spec.FindString("property",&propName) == B_OK) {
		switch(msg->what) {
			case B_GET_PROPERTY:
				if (propName == "Name")
					rc = fNode.ReadAttrString(PSRV_PRINTER_ATTR_PRT_NAME, &result);
				else if (propName == "TransportAddon")
					rc = fNode.ReadAttrString(PSRV_PRINTER_ATTR_TRANSPORT, &result);
				else if (propName == "TransportConfig")
					rc = fNode.ReadAttrString(PSRV_PRINTER_ATTR_TRANSPORT_ADDR, &result);
				else if (propName == "PrinterAddon")
					rc = fNode.ReadAttrString(PSRV_PRINTER_ATTR_DRV_NAME, &result);
				else if (propName == "Comments")
					rc = fNode.ReadAttrString(PSRV_PRINTER_ATTR_COMMENTS, &result);

				BMessage reply(B_REPLY);
				reply.AddString("result", result);
				reply.AddInt32("error", rc);
				msg->SendReply(&reply);
				break;
		}
	}
}

BHandler* Printer::ResolveSpecifier(BMessage* msg, int32 index, BMessage* spec,
	int32 form, const char* prop)
{
	BPropertyInfo prop_info(prop_list);
	BHandler* rc = this;
	
	spec->PrintToStream();
	
	int32 idx;
	switch( idx=prop_info.FindMatch(msg,0,spec,form,prop) ) {
		case B_ERROR:
			rc = Inherited::ResolveSpecifier(msg,index,spec,form,prop);
			break;
	}
	
	return rc;
}

status_t Printer::GetSupportedSuites(BMessage* msg)
{
	msg->AddString("suites", "application/x-vnd.OpenBeOS-printer");
	
	BPropertyInfo prop_info(prop_list);
	msg->AddFlat("messages", &prop_info);
	
	return Inherited::GetSupportedSuites(msg);
}
