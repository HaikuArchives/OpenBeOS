/***********************************************************************
 * AUTHOR: Marcus Overhagen
 *   FILE: MediaAddOn.cpp
 *  DESCR: 
 ***********************************************************************/
#include <MediaAddOn.h>
#include "debug.h"

/*************************************************************
 * public dormant_node_info
 *************************************************************/

// final
dormant_node_info::dormant_node_info()
	: addon(-1),
	flavor_id(-1)
{
	CALLED();
	name[0] = '\0';
}

// final
dormant_node_info::~dormant_node_info()
{
	CALLED();
}

/*************************************************************
 * private flavor_info
 *************************************************************/

/* DO NOT IMPLEMENT */
/*
flavor_info &flavor_info::operator=(const flavor_info &other)
*/

/*************************************************************
 * public dormant_flavor_info
 *************************************************************/

dormant_flavor_info::dormant_flavor_info()
{
	UNIMPLEMENTED();
}


/* virtual */ 
dormant_flavor_info::~dormant_flavor_info()
{
	UNIMPLEMENTED();
}


dormant_flavor_info::dormant_flavor_info(const dormant_flavor_info &)
{
	UNIMPLEMENTED();
}


dormant_flavor_info &
dormant_flavor_info::operator=(const dormant_flavor_info &)
{
	UNIMPLEMENTED();
	return *this;
}


dormant_flavor_info &
dormant_flavor_info::operator=(const flavor_info &)
{
	UNIMPLEMENTED();
	return *this;
}


void
dormant_flavor_info::set_name(const char *in_name)
{
	UNIMPLEMENTED();
}


void
dormant_flavor_info::set_info(const char *in_info)
{
	UNIMPLEMENTED();
}


void
dormant_flavor_info::add_in_format(const media_format &in_format)
{
	UNIMPLEMENTED();
}


void
dormant_flavor_info::add_out_format(const media_format &out_format)
{
	UNIMPLEMENTED();
}


/* virtual */ bool
dormant_flavor_info::IsFixedSize() const
{
	UNIMPLEMENTED();
	bool dummy;

	return dummy;
}


/* virtual */ type_code
dormant_flavor_info::TypeCode() const
{
	UNIMPLEMENTED();
	type_code dummy;

	return dummy;
}


/* virtual */ ssize_t
dormant_flavor_info::FlattenedSize() const
{
	UNIMPLEMENTED();
	ssize_t dummy;

	return dummy;
}


/* virtual */ status_t
dormant_flavor_info::Flatten(void *buffer,
							 ssize_t size) const
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


/* virtual */ status_t
dormant_flavor_info::Unflatten(type_code c,
							   const void *buf,
							   ssize_t size)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}

/*************************************************************
 * private dormant_flavor_info
 *************************************************************/


void
dormant_flavor_info::assign_atoms(const flavor_info &that)
{
	UNIMPLEMENTED();
}

/*************************************************************
 * public BMediaAddOn
 *************************************************************/

/* explicit */ 
BMediaAddOn::BMediaAddOn(image_id image)
{
	UNIMPLEMENTED();
}


/* virtual */ 
BMediaAddOn::~BMediaAddOn()
{
	UNIMPLEMENTED();
}


/* virtual */ status_t
BMediaAddOn::InitCheck(const char **out_failure_text)
{
	UNIMPLEMENTED();

	return B_OK;
}


/* virtual */ int32
BMediaAddOn::CountFlavors()
{
	UNIMPLEMENTED();
	int32 dummy;

	return dummy;
}


/* virtual */ status_t
BMediaAddOn::GetFlavorAt(int32 n,
						 const flavor_info **out_info)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


/* virtual */ BMediaNode *
BMediaAddOn::InstantiateNodeFor(const flavor_info *info,
								BMessage *config,
								status_t *out_error)
{
	UNIMPLEMENTED();
	return NULL;
}


/* virtual */ status_t
BMediaAddOn::GetConfigurationFor(BMediaNode *your_node,
								 BMessage *into_message)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


/* virtual */ bool
BMediaAddOn::WantsAutoStart()
{
	UNIMPLEMENTED();
	bool dummy;

	return dummy;
}


/* virtual */ status_t
BMediaAddOn::AutoStart(int in_count,
					   BMediaNode **out_node,
					   int32 *out_internal_id,
					   bool *out_has_more)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


/* virtual */ status_t
BMediaAddOn::SniffRef(const entry_ref &file,
					  BMimeType *io_mime_type,
					  float *out_quality,
					  int32 *out_internal_id)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


/* virtual */ status_t
BMediaAddOn::SniffType(const BMimeType &type,
					   float *out_quality,
					   int32 *out_internal_id)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


/* virtual */ status_t
BMediaAddOn::GetFileFormatList(int32 flavor_id,
							   media_file_format *out_writable_formats,
							   int32 in_write_items,
							   int32 *out_write_items,
							   media_file_format *out_readable_formats,
							   int32 in_read_items,
							   int32 *out_read_items,
							   void *_reserved)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


/* virtual */ status_t
BMediaAddOn::SniffTypeKind(const BMimeType &type,
						   uint64 in_kinds,
						   float *out_quality,
						   int32 *out_internal_id,
						   void *_reserved)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


image_id
BMediaAddOn::ImageID()
{
	UNIMPLEMENTED();
	image_id dummy;

	return dummy;
}


media_addon_id
BMediaAddOn::AddonID()
{
	UNIMPLEMENTED();
	media_addon_id dummy;

	return dummy;
}

/*************************************************************
 * protected BMediaAddOn
 *************************************************************/


status_t
BMediaAddOn::NotifyFlavorChange()
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}

/*************************************************************
 * private BMediaAddOn
 *************************************************************/

/*
unimplemented:
BMediaAddOn::BMediaAddOn()
BMediaAddOn::BMediaAddOn(const BMediaAddOn &clone)
BMediaAddOn & BMediaAddOn::operator=(const BMediaAddOn &clone)
*/


/*************************************************************
 * private BMediaAddOn
 *************************************************************/

status_t BMediaAddOn::_Reserved_MediaAddOn_0(void *) { return B_ERROR; }
status_t BMediaAddOn::_Reserved_MediaAddOn_1(void *) { return B_ERROR; }
status_t BMediaAddOn::_Reserved_MediaAddOn_2(void *) { return B_ERROR; }
status_t BMediaAddOn::_Reserved_MediaAddOn_3(void *) { return B_ERROR; }
status_t BMediaAddOn::_Reserved_MediaAddOn_4(void *) { return B_ERROR; }
status_t BMediaAddOn::_Reserved_MediaAddOn_5(void *) { return B_ERROR; }
status_t BMediaAddOn::_Reserved_MediaAddOn_6(void *) { return B_ERROR; }
status_t BMediaAddOn::_Reserved_MediaAddOn_7(void *) { return B_ERROR; }

void BMediaAddOn::SetOwner(status_t (*hook)(void *, BMediaAddOn *),void * cookie)
{
	UNIMPLEMENTED();
}

