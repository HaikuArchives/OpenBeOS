
#include <MediaDefs.h>
#include <MediaFormats.h>

namespace BPrivate
{


/* functions parameteres similar to MediaDecoder.h if possible, returntype status_t assumed */
class Decoder
{
public:
	virtual ~Decoder(void); /* this needs to be virtual */
	Decoder(void);

	/* not sure if all three need to be virtual */
	virtual status_t SetTrack(BMediaTrack *track);
	/* what does this one do? strange parameters */
	virtual status_t Reset(long, long long, long long *, long long, long long *);
	virtual status_t GetNextChunk(void const **chunkData, size_t *chunkSize, 
								  media_header *mh, media_decode_info *mdi);

	/* these 4 functions do not appear in libmedia.so, they must be pure virtual */
	virtual status_t Decode(void *out_buffer, int64 *out_frameCount,
							media_header *out_mh, media_decode_info *info) = 0;
	virtual status_t Format(media_format *) = 0;
	virtual status_t Sniff(media_format const *, void const *in_buffer, unsigned long in_buffersize) = 0;
	virtual status_t GetCodecInfo(media_codec_info *out_mci) const = 0;

private:

	// MediaDecoder.h has 32 here, may be more or less
	//uint32 _reserved_data[32]; 
	//we also may have to add some local variables, 
	//and if the used bool, char, or uint16 it's not a multiple of 4
	//let's make a guess. 32 * uint32 + 2 bool + 2 pointers would be 138
	//figuring out the exact size/layout is left as an exercise for the reader
	uint8 reserved[42]; //but if 42 is the answer, what's the question

	virtual status_t Perform(long, void *);
	
	//probably some are already used, so they are no longer virtual
	virtual status_t _Reserved_Decoder_0(long,...);
	virtual status_t _Reserved_Decoder_1(long,...);
	virtual status_t _Reserved_Decoder_2(long,...);
	virtual status_t _Reserved_Decoder_3(long,...);
	virtual status_t _Reserved_Decoder_4(long,...);
	virtual status_t _Reserved_Decoder_5(long,...);
	virtual status_t _Reserved_Decoder_6(long,...);
	virtual status_t _Reserved_Decoder_7(long,...);
	virtual status_t _Reserved_Decoder_8(long,...);
	virtual status_t _Reserved_Decoder_9(long,...);
	virtual status_t _Reserved_Decoder_10(long,...);
	virtual status_t _Reserved_Decoder_11(long,...);
	virtual status_t _Reserved_Decoder_12(long,...);
	virtual status_t _Reserved_Decoder_13(long,...);
	virtual status_t _Reserved_Decoder_14(long,...);
	virtual status_t _Reserved_Decoder_15(long,...);
	virtual status_t _Reserved_Decoder_16(long,...);
	virtual status_t _Reserved_Decoder_17(long,...);
	virtual status_t _Reserved_Decoder_18(long,...);
	virtual status_t _Reserved_Decoder_19(long,...);
	virtual status_t _Reserved_Decoder_20(long,...);
	virtual status_t _Reserved_Decoder_21(long,...);
	virtual status_t _Reserved_Decoder_22(long,...);
	virtual status_t _Reserved_Decoder_23(long,...);
	virtual status_t _Reserved_Decoder_24(long,...);
	virtual status_t _Reserved_Decoder_25(long,...);
	virtual status_t _Reserved_Decoder_26(long,...);
	virtual status_t _Reserved_Decoder_27(long,...);
	virtual status_t _Reserved_Decoder_28(long,...);
	virtual status_t _Reserved_Decoder_29(long,...);
	virtual status_t _Reserved_Decoder_30(long,...);
	virtual status_t _Reserved_Decoder_31(long,...);
	virtual status_t _Reserved_Decoder_32(long,...);
	virtual status_t _Reserved_Decoder_33(long,...);
	virtual status_t _Reserved_Decoder_34(long,...);
	virtual status_t _Reserved_Decoder_35(long,...);
	virtual status_t _Reserved_Decoder_36(long,...);
	virtual status_t _Reserved_Decoder_37(long,...);
	virtual status_t _Reserved_Decoder_38(long,...);
	virtual status_t _Reserved_Decoder_39(long,...);
	virtual status_t _Reserved_Decoder_40(long,...);
	virtual status_t _Reserved_Decoder_41(long,...);
	virtual status_t _Reserved_Decoder_42(long,...);
	virtual status_t _Reserved_Decoder_43(long,...);
	virtual status_t _Reserved_Decoder_44(long,...);
	virtual status_t _Reserved_Decoder_45(long,...);
	virtual status_t _Reserved_Decoder_46(long,...);
	virtual status_t _Reserved_Decoder_47(long,...);
};

}

//any parameters? or none at all?
extern "C" status_t register_decoder(int32 p1,int32 p2,int32 p3,int32 p4,int32 p5,int32 p6,int32 p7,int32 p8);
extern "C" BPrivate::Decoder * instantiate_decoder(int32 p1,int32 p2,int32 p3,int32 p4,int32 p5,int32 p6,int32 p7,int32 p8);

