/***********************************************************************
 * AUTHOR: Marcus Overhagen
 *   FILE: ParameterWeb.cpp
 *  DESCR: 
 ***********************************************************************/
#include <ParameterWeb.h>
#include "debug.h"


/*************************************************************
 * 
 *************************************************************/

const char * const B_GENERIC			 = "XXX fixme";
const char * const B_MASTER_GAIN		 = "XXX fixme";
const char * const B_GAIN				 = "XXX fixme";
const char * const B_BALANCE			 = "XXX fixme";
const char * const B_FREQUENCY			 = "XXX fixme";
const char * const B_LEVEL				 = "XXX fixme";
const char * const B_SHUTTLE_SPEED		 = "XXX fixme";
const char * const B_CROSSFADE			 = "XXX fixme";
const char * const B_EQUALIZATION		 = "XXX fixme";
const char * const B_COMPRESSION		 = "XXX fixme";
const char * const B_QUALITY			 = "XXX fixme";
const char * const B_BITRATE			 = "XXX fixme";
const char * const B_GOP_SIZE			 = "XXX fixme";
const char * const B_MUTE				 = "XXX fixme";
const char * const B_ENABLE				 = "XXX fixme";
const char * const B_INPUT_MUX			 = "XXX fixme";
const char * const B_OUTPUT_MUX			 = "XXX fixme";
const char * const B_TUNER_CHANNEL		 = "XXX fixme";
const char * const B_TRACK				 = "XXX fixme";
const char * const B_RECSTATE			 = "XXX fixme";
const char * const B_SHUTTLE_MODE		 = "XXX fixme";
const char * const B_RESOLUTION			 = "XXX fixme";
const char * const B_COLOR_SPACE		 = "XXX fixme";
const char * const B_FRAME_RATE			 = "XXX fixme";
const char * const B_VIDEO_FORMAT		 = "XXX fixme";
const char * const B_WEB_PHYSICAL_INPUT	 = "XXX fixme";
const char * const B_WEB_PHYSICAL_OUTPUT = "XXX fixme";
const char * const B_WEB_ADC_CONVERTER	 = "XXX fixme";
const char * const B_WEB_DAC_CONVERTER	 = "XXX fixme";
const char * const B_WEB_LOGICAL_INPUT	 = "XXX fixme";
const char * const B_WEB_LOGICAL_OUTPUT	 = "XXX fixme";
const char * const B_WEB_LOGICAL_BUS	 = "XXX fixme";
const char * const B_WEB_BUFFER_INPUT	 = "XXX fixme";
const char * const B_WEB_BUFFER_OUTPUT	 = "XXX fixme";
const char * const B_SIMPLE_TRANSPORT	 = "XXX fixme";

/*************************************************************
 * public BParameterWeb
 *************************************************************/

BParameterWeb::BParameterWeb()
{
	UNIMPLEMENTED();
}


BParameterWeb::~BParameterWeb()
{
	UNIMPLEMENTED();
}


media_node
BParameterWeb::Node()
{
	UNIMPLEMENTED();
	media_node dummy;

	return dummy;
}


BParameterGroup *
BParameterWeb::MakeGroup(const char *name)
{
	UNIMPLEMENTED();
	return NULL;
}


int32
BParameterWeb::CountGroups()
{
	UNIMPLEMENTED();
	int32 dummy;

	return dummy;
}


BParameterGroup *
BParameterWeb::GroupAt(int32 index)
{
	UNIMPLEMENTED();
	return NULL;
}


int32
BParameterWeb::CountParameters()
{
	UNIMPLEMENTED();
	int32 dummy;

	return dummy;
}


BParameter *
BParameterWeb::ParameterAt(int32 index)
{
	UNIMPLEMENTED();
	return NULL;
}


bool
BParameterWeb::IsFixedSize() const
{
	UNIMPLEMENTED();
	bool dummy;

	return dummy;
}


type_code
BParameterWeb::TypeCode() const
{
	UNIMPLEMENTED();
	type_code dummy;

	return dummy;
}


ssize_t
BParameterWeb::FlattenedSize() const
{
	UNIMPLEMENTED();
	ssize_t dummy;

	return dummy;
}


status_t
BParameterWeb::Flatten(void *buffer,
					   ssize_t size) const
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


bool
BParameterWeb::AllowsTypeCode(type_code code) const
{
	UNIMPLEMENTED();
	bool dummy;

	return dummy;
}


status_t
BParameterWeb::Unflatten(type_code c,
						 const void *buf,
						 ssize_t size)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}

/*************************************************************
 * private BParameterWeb
 *************************************************************/

/*
unimplemented
BParameterWeb::BParameterWeb(const BParameterWeb &clone)
BParameterWeb &BParameterWeb::operator=(const BParameterWeb &clone)
*/

status_t BParameterWeb::_Reserved_ControlWeb_0(void *) { return B_ERROR; }
status_t BParameterWeb::_Reserved_ControlWeb_1(void *) { return B_ERROR; }
status_t BParameterWeb::_Reserved_ControlWeb_2(void *) { return B_ERROR; }
status_t BParameterWeb::_Reserved_ControlWeb_3(void *) { return B_ERROR; }
status_t BParameterWeb::_Reserved_ControlWeb_4(void *) { return B_ERROR; }
status_t BParameterWeb::_Reserved_ControlWeb_5(void *) { return B_ERROR; }
status_t BParameterWeb::_Reserved_ControlWeb_6(void *) { return B_ERROR; }
status_t BParameterWeb::_Reserved_ControlWeb_7(void *) { return B_ERROR; }

void
BParameterWeb::AddRefFix(void *oldItem,
						 void *newItem)
{
	UNIMPLEMENTED();
}

/*************************************************************
 * private BParameterGroup
 *************************************************************/

BParameterGroup::BParameterGroup(BParameterWeb *web,
								 const char *name)
{
	UNIMPLEMENTED();
}


BParameterGroup::~BParameterGroup()
{
	UNIMPLEMENTED();
}

/*************************************************************
 * public BParameterGroup
 *************************************************************/

BParameterWeb *
BParameterGroup::Web() const
{
	UNIMPLEMENTED();
	return NULL;
}


const char *
BParameterGroup::Name() const
{
	UNIMPLEMENTED();
	return NULL;
}


void
BParameterGroup::SetFlags(uint32 flags)
{
	UNIMPLEMENTED();
}


uint32
BParameterGroup::Flags() const
{
	UNIMPLEMENTED();
	uint32 dummy;

	return dummy;
}


BNullParameter *
BParameterGroup::MakeNullParameter(int32 id,
								   media_type m_type,
								   const char *name,
								   const char *kind)
{
	UNIMPLEMENTED();
	return NULL;
}


BContinuousParameter *
BParameterGroup::MakeContinuousParameter(int32 id,
										 media_type m_type,
										 const char *name,
										 const char *kind,
										 const char *unit,
										 float minimum,
										 float maximum,
										 float stepping)
{
	UNIMPLEMENTED();
	return NULL;
}


BDiscreteParameter *
BParameterGroup::MakeDiscreteParameter(int32 id,
									   media_type m_type,
									   const char *name,
									   const char *kind)
{
	UNIMPLEMENTED();
	return NULL;
}


BParameterGroup *
BParameterGroup::MakeGroup(const char *name)
{
	UNIMPLEMENTED();
	return NULL;
}


int32
BParameterGroup::CountParameters()
{
	UNIMPLEMENTED();
	int32 dummy;

	return dummy;
}


BParameter *
BParameterGroup::ParameterAt(int32 index)
{
	UNIMPLEMENTED();
	return NULL;
}


int32
BParameterGroup::CountGroups()
{
	UNIMPLEMENTED();
	int32 dummy;

	return dummy;
}


BParameterGroup *
BParameterGroup::GroupAt(int32 index)
{
	UNIMPLEMENTED();
	return NULL;
}


bool
BParameterGroup::IsFixedSize() const
{
	UNIMPLEMENTED();
	bool dummy;

	return dummy;
}


type_code
BParameterGroup::TypeCode() const
{
	UNIMPLEMENTED();
	type_code dummy;

	return dummy;
}


ssize_t
BParameterGroup::FlattenedSize() const
{
	UNIMPLEMENTED();
	ssize_t dummy;

	return dummy;
}


status_t
BParameterGroup::Flatten(void *buffer,
						 ssize_t size) const
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


bool
BParameterGroup::AllowsTypeCode(type_code code) const
{
	UNIMPLEMENTED();
	bool dummy;

	return dummy;
}


status_t
BParameterGroup::Unflatten(type_code c,
						   const void *buf,
						   ssize_t size)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}

/*************************************************************
 * private BParameterGroup
 *************************************************************/

/*
// unimplemented
BParameterGroup::BParameterGroup()
BParameterGroup::BParameterGroup(const BParameterGroup &clone)
BParameterGroup &BParameterGroup::operator=(const BParameterGroup &clone)
*/

status_t BParameterGroup::_Reserved_ControlGroup_0(void *) { return B_ERROR; }
status_t BParameterGroup::_Reserved_ControlGroup_1(void *) { return B_ERROR; }
status_t BParameterGroup::_Reserved_ControlGroup_2(void *) { return B_ERROR; }
status_t BParameterGroup::_Reserved_ControlGroup_3(void *) { return B_ERROR; }
status_t BParameterGroup::_Reserved_ControlGroup_4(void *) { return B_ERROR; }
status_t BParameterGroup::_Reserved_ControlGroup_5(void *) { return B_ERROR; }
status_t BParameterGroup::_Reserved_ControlGroup_6(void *) { return B_ERROR; }
status_t BParameterGroup::_Reserved_ControlGroup_7(void *) { return B_ERROR; }


BParameter *
BParameterGroup::MakeControl(int32 type)
{
	UNIMPLEMENTED();
	return NULL;
}

/*************************************************************
 * public BParameter
 *************************************************************/

BParameter::media_parameter_type
BParameter::Type() const
{
	UNIMPLEMENTED();
	media_parameter_type dummy;

	return dummy;
}


BParameterWeb *
BParameter::Web() const
{
	UNIMPLEMENTED();
	return NULL;
}


BParameterGroup *
BParameter::Group() const
{
	UNIMPLEMENTED();
	return NULL;
}


const char *
BParameter::Name() const
{
	UNIMPLEMENTED();
	return NULL;
}


const char *
BParameter::Kind() const
{
	UNIMPLEMENTED();
	return NULL;
}


const char *
BParameter::Unit() const
{
	UNIMPLEMENTED();
	return NULL;
}


int32
BParameter::ID() const
{
	UNIMPLEMENTED();
	int32 dummy;

	return dummy;
}


void
BParameter::SetFlags(uint32 flags)
{
	UNIMPLEMENTED();
}


uint32
BParameter::Flags() const
{
	UNIMPLEMENTED();
	uint32 dummy;

	return dummy;
}


status_t
BParameter::GetValue(void *buffer,
					 size_t *ioSize,
					 bigtime_t *when)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


status_t
BParameter::SetValue(const void *buffer,
					 size_t size,
					 bigtime_t when)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


int32
BParameter::CountChannels()
{
	UNIMPLEMENTED();
	int32 dummy;

	return dummy;
}


void
BParameter::SetChannelCount(int32 channel_count)
{
	UNIMPLEMENTED();
}


media_type
BParameter::MediaType()
{
	UNIMPLEMENTED();
	media_type dummy;

	return dummy;
}


void
BParameter::SetMediaType(media_type m_type)
{
	UNIMPLEMENTED();
}


int32
BParameter::CountInputs()
{
	UNIMPLEMENTED();
	int32 dummy;

	return dummy;
}


BParameter *
BParameter::InputAt(int32 index)
{
	UNIMPLEMENTED();
	return NULL;
}


void
BParameter::AddInput(BParameter *input)
{
	UNIMPLEMENTED();
}


int32
BParameter::CountOutputs()
{
	UNIMPLEMENTED();
	int32 dummy;

	return dummy;
}


BParameter *
BParameter::OutputAt(int32 index)
{
	UNIMPLEMENTED();
	return NULL;
}


void
BParameter::AddOutput(BParameter *output)
{
	UNIMPLEMENTED();
}


bool
BParameter::IsFixedSize() const
{
	UNIMPLEMENTED();
	bool dummy;

	return dummy;
}


type_code
BParameter::TypeCode() const
{
	UNIMPLEMENTED();
	type_code dummy;

	return dummy;
}


ssize_t
BParameter::FlattenedSize() const
{
	UNIMPLEMENTED();
	ssize_t dummy;

	return dummy;
}


status_t
BParameter::Flatten(void *buffer,
					ssize_t size) const
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


bool
BParameter::AllowsTypeCode(type_code code) const
{
	UNIMPLEMENTED();
	bool dummy;

	return dummy;
}


status_t
BParameter::Unflatten(type_code c,
					  const void *buf,
					  ssize_t size)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}

/*************************************************************
 * private BParameter
 *************************************************************/


status_t BParameter::_Reserved_Control_0(void *) { return B_ERROR; }
status_t BParameter::_Reserved_Control_1(void *) { return B_ERROR; }
status_t BParameter::_Reserved_Control_2(void *) { return B_ERROR; }
status_t BParameter::_Reserved_Control_3(void *) { return B_ERROR; }
status_t BParameter::_Reserved_Control_4(void *) { return B_ERROR; }
status_t BParameter::_Reserved_Control_5(void *) { return B_ERROR; }
status_t BParameter::_Reserved_Control_6(void *) { return B_ERROR; }
status_t BParameter::_Reserved_Control_7(void *) { return B_ERROR; }


BParameter::BParameter(int32 id,
					   media_type m_type,
					   media_parameter_type type,
					   BParameterWeb *web,
					   const char *name,
					   const char *kind,
					   const char *unit)
{
	UNIMPLEMENTED();
}


BParameter::~BParameter()
{
	UNIMPLEMENTED();
}


void
BParameter::FixRefs(BList &old,
					BList &updated)
{
	UNIMPLEMENTED();
}

/*************************************************************
 * public BContinuousParameter
 *************************************************************/


type_code
BContinuousParameter::ValueType()
{
	UNIMPLEMENTED();
	type_code dummy;

	return dummy;
}


float
BContinuousParameter::MinValue()
{
	UNIMPLEMENTED();
	float dummy;

	return dummy;
}


float
BContinuousParameter::MaxValue()
{
	UNIMPLEMENTED();
	float dummy;

	return dummy;
}


float
BContinuousParameter::ValueStep()
{
	UNIMPLEMENTED();
	float dummy;

	return dummy;
}


void
BContinuousParameter::SetResponse(int resp,
								  float factor,
								  float offset)
{
	UNIMPLEMENTED();
}


void
BContinuousParameter::GetResponse(int *resp,
								  float *factor,
								  float *offset)
{
	UNIMPLEMENTED();
}


ssize_t
BContinuousParameter::FlattenedSize() const
{
	UNIMPLEMENTED();
	ssize_t dummy;

	return dummy;
}


status_t
BContinuousParameter::Flatten(void *buffer,
							  ssize_t size) const
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


status_t
BContinuousParameter::Unflatten(type_code c,
								const void *buf,
								ssize_t size)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}

/*************************************************************
 * private BContinuousParameter
 *************************************************************/

status_t BContinuousParameter::_Reserved_ContinuousParameter_0(void *) { return B_ERROR; }
status_t BContinuousParameter::_Reserved_ContinuousParameter_1(void *) { return B_ERROR; }
status_t BContinuousParameter::_Reserved_ContinuousParameter_2(void *) { return B_ERROR; }
status_t BContinuousParameter::_Reserved_ContinuousParameter_3(void *) { return B_ERROR; }
status_t BContinuousParameter::_Reserved_ContinuousParameter_4(void *) { return B_ERROR; }
status_t BContinuousParameter::_Reserved_ContinuousParameter_5(void *) { return B_ERROR; }
status_t BContinuousParameter::_Reserved_ContinuousParameter_6(void *) { return B_ERROR; }
status_t BContinuousParameter::_Reserved_ContinuousParameter_7(void *) { return B_ERROR; }


BContinuousParameter::BContinuousParameter(int32 id,
										   media_type m_type,
										   BParameterWeb *web,
										   const char *name,
										   const char *kind,
										   const char *unit,
										   float minimum,
										   float maximum,
										   float stepping)
	: BParameter(id,m_type,B_CONTINUOUS_PARAMETER,web,name,kind,unit)
{
	UNIMPLEMENTED();
}


BContinuousParameter::~BContinuousParameter()
{
	UNIMPLEMENTED();
}

/*************************************************************
 * public BDiscreteParameter
 *************************************************************/

type_code
BDiscreteParameter::ValueType()
{
	UNIMPLEMENTED();
	type_code dummy;

	return dummy;
}


int32
BDiscreteParameter::CountItems()
{
	UNIMPLEMENTED();
	int32 dummy;

	return dummy;
}


const char *
BDiscreteParameter::ItemNameAt(int32 index)
{
	UNIMPLEMENTED();
	return NULL;
}


int32
BDiscreteParameter::ItemValueAt(int32 index)
{
	UNIMPLEMENTED();
	int32 dummy;

	return dummy;
}


status_t
BDiscreteParameter::AddItem(int32 value,
							const char *name)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


status_t
BDiscreteParameter::MakeItemsFromInputs()
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


status_t
BDiscreteParameter::MakeItemsFromOutputs()
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


void
BDiscreteParameter::MakeEmpty()
{
	UNIMPLEMENTED();
}


ssize_t
BDiscreteParameter::FlattenedSize() const
{
	UNIMPLEMENTED();
	ssize_t dummy;

	return dummy;
}


status_t
BDiscreteParameter::Flatten(void *buffer,
							ssize_t size) const
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


status_t
BDiscreteParameter::Unflatten(type_code c,
							  const void *buf,
							  ssize_t size)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}

/*************************************************************
 * private BDiscreteParameter
 *************************************************************/

status_t BDiscreteParameter::_Reserved_DiscreteParameter_0(void *) { return B_ERROR; }
status_t BDiscreteParameter::_Reserved_DiscreteParameter_1(void *) { return B_ERROR; }
status_t BDiscreteParameter::_Reserved_DiscreteParameter_2(void *) { return B_ERROR; }
status_t BDiscreteParameter::_Reserved_DiscreteParameter_3(void *) { return B_ERROR; }
status_t BDiscreteParameter::_Reserved_DiscreteParameter_4(void *) { return B_ERROR; }
status_t BDiscreteParameter::_Reserved_DiscreteParameter_5(void *) { return B_ERROR; }
status_t BDiscreteParameter::_Reserved_DiscreteParameter_6(void *) { return B_ERROR; }
status_t BDiscreteParameter::_Reserved_DiscreteParameter_7(void *) { return B_ERROR; }


BDiscreteParameter::BDiscreteParameter(int32 id,
									   media_type m_type,
									   BParameterWeb *web,
									   const char *name,
									   const char *kind)
: BParameter(id,m_type,B_DISCRETE_PARAMETER,web,name,kind,"xxx fixme")
{
	UNIMPLEMENTED();
}


BDiscreteParameter::~BDiscreteParameter()
{
	UNIMPLEMENTED();
}

/*************************************************************
 * public BNullParameter
 *************************************************************/

type_code
BNullParameter::ValueType()
{
	UNIMPLEMENTED();
	type_code dummy;

	return dummy;
}


ssize_t
BNullParameter::FlattenedSize() const
{
	UNIMPLEMENTED();
	ssize_t dummy;

	return dummy;
}


status_t
BNullParameter::Flatten(void *buffer,
						ssize_t size) const
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}


status_t
BNullParameter::Unflatten(type_code c,
						  const void *buf,
						  ssize_t size)
{
	UNIMPLEMENTED();
	status_t dummy;

	return dummy;
}

/*************************************************************
 * private BNullParameter
 *************************************************************/

status_t BNullParameter::_Reserved_NullParameter_0(void *) { return B_ERROR; }
status_t BNullParameter::_Reserved_NullParameter_1(void *) { return B_ERROR; }
status_t BNullParameter::_Reserved_NullParameter_2(void *) { return B_ERROR; }
status_t BNullParameter::_Reserved_NullParameter_3(void *) { return B_ERROR; }
status_t BNullParameter::_Reserved_NullParameter_4(void *) { return B_ERROR; }
status_t BNullParameter::_Reserved_NullParameter_5(void *) { return B_ERROR; }
status_t BNullParameter::_Reserved_NullParameter_6(void *) { return B_ERROR; }
status_t BNullParameter::_Reserved_NullParameter_7(void *) { return B_ERROR; }


BNullParameter::BNullParameter(int32 id,
							   media_type m_type,
							   BParameterWeb *web,
							   const char *name,
							   const char *kind)
	: BParameter(id,m_type,B_NULL_PARAMETER,web,name,kind,"xxx fixme")
{
	UNIMPLEMENTED();
}


BNullParameter::~BNullParameter()
{
	UNIMPLEMENTED();
}


