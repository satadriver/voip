//注意MFC的库文件和WIN32的不兼容，在编译库文件时要根据这点选择编译方式
#pragma once
#include <stdio.h>
#include <windows.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <msacm.h>
#include "..\\include\\IncSpeex\\speex.h"
#include "..\\include\\IncSpeex\\speex_bits.h"
#include "..\\include\\IncSpeex\\speex_callbacks.h"
#include "..\\include\\IncSpeex\\speex_header.h"
#include "..\\include\\IncSpeex\\speex_stereo.h"
#include "..\\include\\IncSpeex\\speex_buffer.h"
#include "..\\include\\IncSpeex\\speex_types.h"
#include "..\\include\\IncSpeex\\speex_stereo.h"
#include "..\\include\\IncSpeex\\speex_resampler.h"
#include "..\\include\\IncILBC\\iLBC.h"


#include "..\\include\\IncOpus\\opus.h"
#include "..\\include\\IncOpus\\opus_defines.h."
#include "..\\include\\IncOpus\\opus_types.h"
#include "..\\include\\IncOpus\\opus_multistream.h"
//#include "..\\IncOpus\\opus_custom.h"


#pragma comment (lib,"..\\lib\\opus.lib")
#pragma comment (lib,"..\\lib\\celt.lib")
#pragma comment (lib,"..\\lib\\va_g729.lib")
#pragma comment (lib,"..\\lib\\libspeex.lib")
#pragma comment (lib,"..\\lib\\CodecG723.lib")
#pragma comment (lib,"..\\lib\\CodecGSM.lib")
#pragma comment (lib,"..\\lib\\IlbcCodec.lib")
#pragma comment (lib,"..\\lib\\AMR.lib")
#pragma comment (lib,"..\\lib\\opus.lib")
#pragma comment (lib,"winmm.lib")
#pragma comment (lib,"msacm32.lib")

//注意WIN32工程的库文件和MFC的工程文件不能兼容，必须重新创建一个工程后改变支持的环境属性后重新编译



typedef short (* iLBC_Decode_Init)( iLBC_Dec_Inst_t *iLBCdec_inst,int mode,int use_enhancer);
typedef  void (* iLBC_Decode)( float *decblock,unsigned char *bytes,iLBC_Dec_Inst_t *iLBCdec_inst,int mode);


typedef OpusDecoder *(*OpusDecoderCreate)(opus_int32 Fs,int channels,int *error);
typedef int (*OpusDecoderInit)(OpusDecoder *st,opus_int32 Fs,int channels);
typedef int (*OpusDecode)(OpusDecoder *st,const unsigned char *data,int len,opus_int16 *pcm,int frame_size,int decode_fec);
typedef int (*OpusDecoderGetSize)(int Channel);
typedef void (*OpusDecoderDestroy)(OpusDecoder *st);



typedef int (*OpusDecoderGetNBSamples)( const OpusDecoder * dec, const unsigned char packet[ ],opus_int32 len );
typedef int (*OpusPacketGetBandwidth) ( const unsigned char * data );
typedef int (*OpusPacketGetNBChannels) ( const unsigned char * data );
typedef int (*OpusPacketGetNBFrames) ( const unsigned char packet[ ], opus_int32 len );
typedef int (*OpusPacketGetNBSamples) ( const unsigned char packet[ ], opus_int32 len, opus_int32 Fs );
typedef int (*OpusPacketGetSamplesPerFrame) ( const unsigned char * data, opus_int32 Fs );
typedef int (*OpusPacketParse) ( const unsigned char * data, opus_int32 len, unsigned char * out_toc,
								const unsigned char * frames[48], opus_int16 size[48], int * payload_offset );


#pragma pack(1)

typedef struct
{
	unsigned char *		buf;
	unsigned int		len;
	unsigned int		ActualLen;
	char FileName[256];
}DATABUF,*LPDATABUF;


typedef struct _OLDVOIPFILEHEADER
{
	unsigned int Protocol;
	unsigned int UserData;
	unsigned int Reserved;
	unsigned int Unused;
}OLDVOIPFILEHEADER,*LPOLDVOIPFILEHEADER;






typedef struct _VOIPFILEHEADER
{
	unsigned char Protocol;		//注意这里跟DCE中的名字不同，但是同一个成员变量
	unsigned char Version;
	unsigned char HeaderLen;		//为了兼容实际数值为头长度减去16
	unsigned char Undefine;
	
	unsigned int UserData;
	unsigned int CallerPackCnt;
	unsigned int CalleePackCnt;
	//	unsigned char CallerPhone[16];
	//	unsigned char CalleePhone[16];
}VOIPFILEHEADER, * LPVOIPFILEHEADER;






typedef struct  
{
	unsigned char			RIFF[4];
	unsigned int			WaveSize;
	unsigned char			WAVE[4];
	unsigned char			FMT[4];
	unsigned int			FormatInfoSize;
	unsigned short			PCM;
	unsigned short			ChannelCnt;
	unsigned int			SamplesPerSec;
	unsigned int			AvgBytesPerSec;
	unsigned short			BlockAlign;
	unsigned short			BitsPerSample;
	unsigned char			DATA[4];
	unsigned int			DataSize;
}WAVEHEADER,*LPWAVEHEADER;


typedef struct 
{
	short			dp0[ 280 ];
	short			z1;				/* preprocessing.c, Offset_com.			*/

	long			L_z2;			/*                  Offset_com.			*/
	int				mp;				/*                  Preemphasis			*/

	short			u[8];			/* short_term_aly_filter.c				*/
	short			LARpp[2][8]; 	/*										*/
	short			j;				/*										*/
	short            ltp_cut;       /* long_term.c, LTP crosscorr.			*/
	short			nrp;			/* 40 */	/* long_term.c, synthesis	*/
	short			v[9];			/* short_term.c, synthesis				*/
	short			msr;			/* decoder.c,	Postprocessing			*/

	char			verbose;		/* only used if !NDEBUG					*/
	char			fast;			/* only used if FAST					*/
	char			wav_fmt;		/* only used if WAV49 defined			*/

	unsigned char	frame_index;	/*            odd/even chaining			*/
	unsigned char	frame_chain;	/*   half-byte to carry forward			*/
}GSMstate,*LPGSMstate;
#pragma pack()

extern "C"	GSMstate *			gsm_create(void);
extern "C"	int					gsm_option(GSMstate * ,int ,int *);
extern "C"	int					gsm_decode(GSMstate * ,unsigned char * ,short *);
extern "C"	void				gsm_destroy(GSMstate * );
extern "C"	void				va_g729a_init_decoder(void);
extern "C"	void				va_g729a_decoder(unsigned char *bitstream, short *synth_short, int bfi);
extern "C"  int					DecodeG723_24(short * PCMaudioPointer , int * G723audioPointer , int DataSize);
extern "C"  int					DecodeG721(short * PCMaudioPointer , int * G723audioPointer , int DataSize);
extern "C"  int					DecodeG723_40(short * PCMaudioPointer , int * G723audioPointer , int DataSize);

extern "C"	void				Decoder_Interface_Decode( void *st,unsigned char *bits,short *synth, int bfi );
extern "C"	void *				Decoder_Interface_init( void );
extern "C"	void				Decoder_Interface_exit( void *state );



BOOL __stdcall ParseZelloData(LPVOIPFILEHEADER VoipFileHdr,LPDATABUF DataBuf);
BOOL __stdcall ParseTalkBoxTcpData(LPVOIPFILEHEADER VoipFileHdr,LPDATABUF DataBuf);
BOOL __stdcall DecodeMobileALICALL(unsigned char * DataBuf,int BufSize, int UserData);
BOOL __stdcall DecodeALICALL(unsigned int * DataBuf,unsigned int BufSize);
BOOL __stdcall Decode97CALL(unsigned char * DataBuf,unsigned int BufSize,unsigned char MaskCode);
BOOL __stdcall DecodeDYT(unsigned char * DataBuf,unsigned int BufSize);
BOOL __stdcall DecodeKBCALL(unsigned char * DataBuf,unsigned int BufSize);
BOOL __stdcall DecodeKUBAO(unsigned char * DataBuf,unsigned int BufSize);
BOOL __stdcall DecodeYMCALL(unsigned char * DataBuf, int BufSize);

BOOL __stdcall DecodeSpeex(LPDATABUF DataBuf, LPVOIPFILEHEADER VoipFileHdr);
BOOL __stdcall msG723Decode(LPDATABUF DataBuf);
BOOL __stdcall CodecGSM( LPDATABUF DataBuf);
BOOL __stdcall G721ACodec( LPDATABUF DataBuf);
BOOL __stdcall G723ACodec( LPDATABUF DataBuf ,int Flag );
BOOL __stdcall DecodePCMU( LPDATABUF DataBuf);
BOOL __stdcall DecodePCMA(LPDATABUF DataBuf);
BOOL __stdcall DecodeG729A(LPDATABUF DataBuf);
BOOL __stdcall CodecILBC(LPDATABUF DataBuf,LPVOIPFILEHEADER VoipHdr);
BOOL __stdcall DecodeOpus(LPDATABUF DataBuf,LPVOIPFILEHEADER VoipHdr);
BOOL __stdcall DecodeAmr(LPDATABUF DataBuf);
BOOL __stdcall VoipDataParse(LPVOIPFILEHEADER VoipFileHdr,LPDATABUF DataBuf);
BOOL __stdcall AudioMix(short * CallerSample,short * CalleeSample, short * DstBuf, int MixCnt);	//归一化混音