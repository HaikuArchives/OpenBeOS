#include <MediaDefs.h>
#include <MediaFormats.h>

namespace BPrivate
{

/* functions parameteres similar to MediaEncoder.h if possible, returntype status_t assumed */
class Encoder
{
public:
	virtual ~Encoder(void); /* this needs to be virtual */
	Encoder(void);

	/* not sure if all these need to be virtual */
	virtual status_t Flush(void);
	virtual status_t StartEncoder(void);
	virtual status_t SetEncodeParameters(encode_parameters *);
	virtual status_t GetEncodeParameters(encode_parameters *) const;
	virtual status_t GetParameterView(void);
	virtual status_t SetParameterValue(long, void const *, unsigned long);
	virtual status_t GetParameterValue(long, void *, unsigned long *);
	virtual status_t WriteChunk(void const *, unsigned long, media_encode_info *);
	virtual status_t AddTrackInfo(unsigned long, char const *, unsigned long);
	virtual status_t AddCopyright(char const *);
	virtual status_t Web(void);
	virtual status_t AttachedToTrack(void);
	virtual status_t SetTrack(BMediaTrack *);
	
	/* these 3 functions do not appear in libmedia.so, they must be pure virtual */
	virtual status_t GetCodecInfo(media_codec_info *) const = 0;
	virtual status_t Encode(void const *, long long, media_encode_info *) = 0;
	virtual status_t SetFormat(media_file_format *, media_format *, media_format *) = 0;

private:
	// MediaEncoder.h has 32 here, may be more or less
	//uint32 _reserved_data[32]; 
	//we also may have to add some local variables, 
	//and if the used bool, char, or uint16 it's not a multiple of 4
	//let's make a guess. 32 * uint32 + 2 bool + 2 pointers would be 138
	//figuring out the exact size/layout is left as an exercise for the reader
	uint8 reserved[42]; //but if 42 is the answer, what's the question
	
	virtual status_t Perform(long, void *);

	//probably some are already used, so they are no longer virtual
	virtual status_t _Reserved_Encoder_0(long,...);
	virtual status_t _Reserved_Encoder_1(long,...);
	virtual status_t _Reserved_Encoder_2(long,...);
	virtual status_t _Reserved_Encoder_3(long,...);
	virtual status_t _Reserved_Encoder_4(long,...);
	virtual status_t _Reserved_Encoder_5(long,...);
	virtual status_t _Reserved_Encoder_6(long,...);
	virtual status_t _Reserved_Encoder_7(long,...);
	virtual status_t _Reserved_Encoder_8(long,...);
	virtual status_t _Reserved_Encoder_9(long,...);
	virtual status_t _Reserved_Encoder_10(long,...);
	virtual status_t _Reserved_Encoder_11(long,...);
	virtual status_t _Reserved_Encoder_12(long,...);
	virtual status_t _Reserved_Encoder_13(long,...);
	virtual status_t _Reserved_Encoder_14(long,...);
	virtual status_t _Reserved_Encoder_15(long,...);
	virtual status_t _Reserved_Encoder_16(long,...);
	virtual status_t _Reserved_Encoder_17(long,...);
	virtual status_t _Reserved_Encoder_18(long,...);
	virtual status_t _Reserved_Encoder_19(long,...);
	virtual status_t _Reserved_Encoder_20(long,...);
	virtual status_t _Reserved_Encoder_21(long,...);
	virtual status_t _Reserved_Encoder_22(long,...);
	virtual status_t _Reserved_Encoder_23(long,...);
	virtual status_t _Reserved_Encoder_24(long,...);
	virtual status_t _Reserved_Encoder_25(long,...);
	virtual status_t _Reserved_Encoder_26(long,...);
	virtual status_t _Reserved_Encoder_27(long,...);
	virtual status_t _Reserved_Encoder_28(long,...);
	virtual status_t _Reserved_Encoder_29(long,...);
	virtual status_t _Reserved_Encoder_30(long,...);
	virtual status_t _Reserved_Encoder_31(long,...);
	virtual status_t _Reserved_Encoder_32(long,...);
	virtual status_t _Reserved_Encoder_33(long,...);
	virtual status_t _Reserved_Encoder_34(long,...);
	virtual status_t _Reserved_Encoder_35(long,...);
	virtual status_t _Reserved_Encoder_36(long,...);
	virtual status_t _Reserved_Encoder_37(long,...);
	virtual status_t _Reserved_Encoder_38(long,...);
	virtual status_t _Reserved_Encoder_39(long,...);
	virtual status_t _Reserved_Encoder_40(long,...);
	virtual status_t _Reserved_Encoder_41(long,...);
	virtual status_t _Reserved_Encoder_42(long,...);
	virtual status_t _Reserved_Encoder_43(long,...);
	virtual status_t _Reserved_Encoder_44(long,...);
	virtual status_t _Reserved_Encoder_45(long,...);
	virtual status_t _Reserved_Encoder_46(long,...);
	virtual status_t _Reserved_Encoder_47(long,...);
};

}

// any parameters? or none at all?
// and what does _nth_encoder refer to? why nth?
extern "C" BPrivate::Encoder *instantiate_nth_encoder(int32 p1,int32 p2,int32 p3,int32 p4,int32 p5);
extern "C" status_t register_encoder(int32 p1,int32 p2,int32 p3,int32 p4,int32 p5,int32 p6,int32 p7,int32 p8);
extern "C" BPrivate::Encoder *instantiate_encoder(int32 p1,int32 p2,int32 p3,int32 p4,int32 p5,int32 p6,int32 p7,int32 p8);
