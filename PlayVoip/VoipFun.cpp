#pragma once
#include "VoipFun.h"
//#pragma comment(LIBC,"/NODEFAULTLIB:library")
//#pragma comment(LIBCMT,"/NODEFAULTLIB:library")
//在project-setting-link-project options中添加/NODEFAULTLIB:LIBC.lib,/NODEFAULTLIB:LIBCMT.lib

typedef int (* SpeexDecodeInt )(void *,SpeexBits *,short *);
typedef int (* SpeexDecoderCtl)(void *,int,int *);
typedef void * (* SpeexDecoderInit)(SpeexMode*);
typedef void (* SpeexBitsInit)(SpeexBits*);



#include "..\\SilkCodec\\SilkCodec.h"



BOOL __stdcall ParseZelloData(LPVOIPFILEHEADER VoipFileHdr,LPDATABUF DataBuf)
{
	int Len = *(unsigned short*)DataBuf->buf;
	if (* DataBuf->buf == 0x3c && * (DataBuf->buf + 32 ) == 0x3c)
	{
		DecodeAmr(DataBuf);
	}
	else if (Len == 0x0113 || Len == 0x0177)
	{
		if (Len >= 512)
		{
			return FALSE;
		}
		if(* (unsigned short*)(DataBuf->buf + Len + 2 ) == 0x113 || *(unsigned short*) (DataBuf->buf + Len + 2 ) == 0x177 )
		{
			DecodeSpeex(DataBuf,VoipFileHdr);
		}
	}
	else if (* (DataBuf->buf + 2) == 0x18 || * (DataBuf->buf + 2) == 0x58 )
	{
		if (Len >= 4096)
		{
			return FALSE;
		}
		if(* (DataBuf->buf + Len + 2 + 2) == 0x18 || * (DataBuf->buf + Len + 2 + 2) == 0x58 )
		{
			DecodeOpus(DataBuf,VoipFileHdr);
		}
	}
	
	return TRUE;
}






int __stdcall AudioMix(short * CallerSample,short * CalleeSample, short * DstBuf, int MixCnt)	//归一化混音
{ 
	CallerSample --;
	CalleeSample --;
	DstBuf --;
	int Tmp = 0;       
    for(int Count = 0; Count < MixCnt/2; Count ++)
    {
        Tmp = * CallerSample + * CalleeSample; 
		if(Tmp > 32767)
		{
			Tmp = 32767;
		}
		
		if(Tmp < -32768)
		{
			Tmp = -32768;
		}
		
		* DstBuf = Tmp;
		DstBuf --;
		CallerSample --;
		CalleeSample --;
    }      
	
	return true;
}





BOOL __stdcall DecodeALICALL(unsigned int * DataBuf,unsigned int BufSize)
{
	for(; BufSize >= 4; BufSize -= 4)		
	{
		*DataBuf ^= 0x57375737;		
		DataBuf ++ ;
	}

	if(BufSize)
	{
		* DataBuf ^= 0x37;
		DataBuf = (unsigned int *)((unsigned int)DataBuf +1);
		BufSize --;	
	}
	else
	{
		return true;
	}

	if(BufSize)
	{
		* DataBuf ^= 0x57;
		DataBuf = (unsigned int *)((unsigned int)DataBuf +1);
		BufSize --;
	}
	else
	{
		return true;
	}
	
	if(BufSize)
	{
		* DataBuf ^= 0x37;
		DataBuf = (unsigned int *)((unsigned int)DataBuf +1);
		BufSize --;	
	}
	else
	{
		return true;
	}
	return true;
}



BOOL __stdcall Decode97CALL(unsigned char * DataBuf,unsigned int BufSize,unsigned char MaskCode)
{
	if(MaskCode & 0x80)
	{
		MaskCode = MaskCode ^ 0x92;
	}
	else
	{
		MaskCode = MaskCode ^ 0x12;
	}

	for ( ; BufSize;BufSize --)
	{
		* DataBuf ^= MaskCode;
		DataBuf ++;
	}
	return true;
}



BOOL __stdcall DecodeDYT(unsigned char * DataBuf,unsigned int BufSize)
{
	unsigned char   MaskCode[20]		=	{
// 										0x2b,0xa9,0x3f,0xdf,
// 										0x2c,0x3f,0x4b,0xaf,
// 										0x20,0x81,0x10,0x8b,
										0xde,0x1c,0xbf,0xd5,
										0x94,0xba,0x6d,0xa5,
										0x6f,0xa0,0x2e,0x51,
										0x30,0x5e,0x50,0xa5,
										0xea,0x00,0x4d,0xfc};
// 										0xf7,0x54,0x31,0x98,	//RTCP协议异或码
// 										0xc9,0xea,0x95,0x6b,
// 										0x3f,0x53,0x86,0x1f,
// 										0x82,0xac,0x2b,0x9c,
// 										0xc9,0xa7,0xfa,0xf3,
// 										0x61,0x41,0xc1,0x68,
// 										0x05,0x33,0x07,0x51,
// 										0x77,0x92,0xf1,0x8d
	unsigned char * MaskBuf;
	int Count;
	for (BufSize = BufSize/20; BufSize;BufSize --)
	{
		MaskBuf = MaskCode;
		for (Count = 20; Count ; Count --)
		{
			* DataBuf ^= * MaskBuf;
			DataBuf ++;
			MaskBuf ++;
		}
	}
	return true;
}




BOOL __stdcall DecodeKBCALL(unsigned char * DataBuf,unsigned int BufSize)
{
	unsigned char   MaskCode[20] =	{0x2A,0x6A,0x66,0x6A,0x65,0x2A,0x28,0x26,0x34,0x2D,
									 0x2A,0x6A,0x66,0x6A,0x65,0x2A,0x28,0x26,0x34,0x2D};
	unsigned char * MaskBuf;
	int Count;
	for (BufSize = BufSize/20; BufSize;BufSize --)
	{
		MaskBuf = MaskCode;
		for (Count = 20; Count ; Count --)
		{
			* DataBuf ^= * MaskBuf;
			DataBuf ++;
			MaskBuf ++;
		}
	}
	return true;
}




BOOL __stdcall DecodeKUBAO(unsigned char * DataBuf,unsigned int BufSize)
{
	for ( ; BufSize ; BufSize --)
	{
		* DataBuf = ~( * DataBuf);
		DataBuf ++;
	}
	return true;
}



BOOL __stdcall DecodeYMCALL(unsigned char * DataBuf, int BufSize)
{
	unsigned char	MaskCode[]=	{	
// 										0x48,0xef,0xe7,0xb0,
// 										0xf2,0x62,0x4e,0xa8,
// 										0xc5,0x7f,0xbe,0x1e,
										0x2b,0x9b,0xe9,0xb7,
										0x68,0x58,0x5a,0x17,
										0x1f,0x0a,0x6f,0x40,
										0xfb,0xf1,0xdd,0x92,
										0xca,0xc6,0xbf,0xa8,
										0x10,0xa5,0x48,0x12,
										0x91,0xeb,0x98,0x12,
										0x34,0xad,0xf0,0x47,0xb8};//后面是RTCP协议的后半部分异或字段
// 										0x6a,0x23,0x75,
// 										0x8e,0x1b,0x17,0xd8,
// 										0xa5,0x99,0x1b,0xa5,
// 										0xe7,0x28,0xe7,0x7d,
// 										0x3e,0x2f,0x38,0x33};

	unsigned char * MaskBuf;
	int Count;
	for (BufSize = BufSize/33; BufSize; BufSize --)
	{
		MaskBuf = MaskCode;
		for (Count = 33; Count ; Count --)
		{
			* DataBuf ^= *MaskBuf;
			DataBuf ++;
			MaskBuf ++;
		}
	}
	return true;
}







//     int i = -21417;
//     for (int j = 0; ; j++)
//     {
//       if (j >= 160)
//         return;
//       int k = j * 2;
//       paramArrayOfByte[k] = (byte)(paramArrayOfByte[k] ^ (byte)(i & 0xFF));
//       int m = 1 + j * 2;
//       paramArrayOfByte[m] = (byte)(paramArrayOfByte[m] ^ (byte)(0xFF & i >> 8));
//       i = (short)(i + 1);
//     }
BOOL __stdcall DecodeMobileALICALL(unsigned char * EncodePointer ,int EncodeDataSize, int UserData)
{
	unsigned char MaskCode;
	unsigned char Tmp_1;
	unsigned char Tmp_2;

    for (int Cnt = EncodeDataSize/20; Cnt; Cnt --)
    {
		MaskCode = 0x87;

		Tmp_1 = *(EncodePointer + 10 );
		Tmp_2 = *(EncodePointer + 11 );
		*(EncodePointer + 11 ) = Tmp_1;
		*(EncodePointer + 10 ) = Tmp_2;

		for (int Count = 0; Count < 20; Count ++ )
		{
			* EncodePointer  ^= MaskCode;
			EncodePointer ++;
			MaskCode --;
		}
    }
	return true;
}		
	




BOOL __stdcall ParseTalkBoxTcpData(LPVOIPFILEHEADER VoipFileHdr,LPDATABUF DataBuf)
{
	unsigned int ParseDataSize = DataBuf->len;
	char * ParseBuf = (char*)VirtualAlloc(0, DataBuf->len, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);		
	if(!ParseBuf)
	{
		printf("VirtualAlloc Error!\n");
		return false;
	}
	char * ParsePointer = ParseBuf;

	unsigned int CallerID = VoipFileHdr->CallerPackCnt;
	unsigned int CalleeID = VoipFileHdr->CalleePackCnt;
	char UserID[256] = {0};
	wsprintf(UserID,"主叫ID:%d\t被叫ID:%d",CallerID,CalleeID);
	MessageBox(0,UserID,"通话双方的ID",MB_OK);
	char * TalkBoxDataBuf = (char*)(DataBuf->buf);
	for (int Count = DataBuf->len/128; Count; Count --)
	{
		if( ( *(unsigned short*)(TalkBoxDataBuf + 6) == 0x2e ) &&
			( *(unsigned int* )(TalkBoxDataBuf + 50) == 0x457) )
		{
			CallerID = *(unsigned int*)(TalkBoxDataBuf + 8);
			CalleeID = *(unsigned int*)(TalkBoxDataBuf + 12);
			if( (ParsePointer - ParseBuf) % 38 )
			{
				ParsePointer += (38 - (ParsePointer - ParseBuf) % 38 );
				TalkBoxDataBuf += 128;
				continue;				//couter 依然减1
			}
			else
			{
				TalkBoxDataBuf += 128;
				continue;				//couter 依然减1
			}
		}
		else if( ( *(unsigned short*)(TalkBoxDataBuf + 8) == 0x2e ) && 
				 ( *(unsigned int* )(TalkBoxDataBuf + 52) == 0x457) )
		{
			if( (ParsePointer - ParseBuf) % 38 )
			{
				ParsePointer += (38 - (ParsePointer - ParseBuf) % 38 );
				TalkBoxDataBuf += 130;
				continue;				//couter 依然减1
			}
			else
			{
				TalkBoxDataBuf += 130;
				continue;				//couter 依然减1
			}
		}	
		
		else if( ( *(unsigned short*)(TalkBoxDataBuf + 6) == 0x78 ) && 
			     ( ( *(unsigned int*)(TalkBoxDataBuf + 8) == CallerID ) ||
			       ( *(unsigned int*)(TalkBoxDataBuf + 12) == CallerID) )  )
		{
			memmove(ParsePointer, TalkBoxDataBuf + 50, 78);
			ParsePointer += 78;	
		}
		else if( ( *(unsigned short*)(TalkBoxDataBuf + 6) == 0x78 ) && 
			     ( ( *(unsigned int*)(TalkBoxDataBuf + 8) == CalleeID ) ||
			       ( *(unsigned int*)(TalkBoxDataBuf + 12) == CalleeID) )  )
		{
			memmove(ParsePointer, TalkBoxDataBuf + 50, 78);
			ParsePointer += 78;	
		}

		else if( ( *(unsigned short*)(TalkBoxDataBuf + 6) == 0x78 ) && 
			     (*(unsigned int*)(TalkBoxDataBuf + 8) != CallerID) &&
			     (*(unsigned int*)(TalkBoxDataBuf + 12) != CallerID) )
		{
			memmove(ParsePointer, TalkBoxDataBuf + 8, 120);
			ParsePointer += 120;
		}
		else if( ( *(unsigned short*)(TalkBoxDataBuf + 6) == 0x78 ) && 
			     (*(unsigned int*)(TalkBoxDataBuf + 8) != CalleeID) &&
			     (*(unsigned int*)(TalkBoxDataBuf + 12) != CalleeID) )
		{
			memmove(ParsePointer, TalkBoxDataBuf + 8, 120);
			ParsePointer += 120;
		}

		else if( ( *(unsigned short*)(TalkBoxDataBuf + 8) == 0x78 ) && 
			     ( ( *(unsigned int*)(TalkBoxDataBuf + 10) == CallerID ) ||
			       ( *(unsigned int*)(TalkBoxDataBuf + 14) == CallerID) )  )
		{
			memmove(ParsePointer, TalkBoxDataBuf + 52, 78);
			ParsePointer += 78;	
			TalkBoxDataBuf += 2;
		}
		else if( ( *(unsigned short*)(TalkBoxDataBuf + 8) == 0x78 ) && 
				 ( ( *(unsigned int*)(TalkBoxDataBuf + 10) == CalleeID ) ||
				   ( *(unsigned int*)(TalkBoxDataBuf + 14) == CalleeID) )  )
		{
			memmove(ParsePointer, TalkBoxDataBuf + 52, 78);
			ParsePointer += 78;	
			TalkBoxDataBuf += 2;
		}

		else if( ( *(unsigned short*)(TalkBoxDataBuf + 8) == 0x78 ) && 
				 (*(unsigned int*)(TalkBoxDataBuf + 10) != CallerID) &&
				 (*(unsigned int*)(TalkBoxDataBuf + 14) != CallerID) )
		{
			memmove(ParsePointer, TalkBoxDataBuf + 10, 120);
			ParsePointer += 120;
			TalkBoxDataBuf += 2;
		}
		else if( ( *(unsigned short*)(TalkBoxDataBuf + 8) == 0x78 ) && 
			(*(unsigned int*)(TalkBoxDataBuf + 10) != CalleeID) &&
			(*(unsigned int*)(TalkBoxDataBuf + 14) != CalleeID) )
		{
			memmove(ParsePointer, TalkBoxDataBuf + 10, 120);
			ParsePointer += 120;
			TalkBoxDataBuf += 2;
		}

		else if( ( *(unsigned short*)(TalkBoxDataBuf + 6) < 0x78 ) && 
			( *(unsigned short*)(TalkBoxDataBuf + 6) > 42 ) &&
			( (*(unsigned int*)(TalkBoxDataBuf + 8) == CallerID) ||
			  (*(unsigned int*)(TalkBoxDataBuf + 12) == CallerID) ) )
		{
			int Cnt = 78 - (0x78 - *(unsigned short*)(TalkBoxDataBuf + 6) );
			memmove(ParsePointer, TalkBoxDataBuf + 50, Cnt);
			ParsePointer += Cnt;
		}

		else if( ( *(unsigned short*)(TalkBoxDataBuf + 6) == 0x2a ) &&
			( (*(unsigned int*)(TalkBoxDataBuf + 8) == CallerID) ||
			(*(unsigned int*)(TalkBoxDataBuf + 12) == CallerID) ) )
		{
			TalkBoxDataBuf += 128;
			continue;
		}
		else if( ( *(unsigned short*)(TalkBoxDataBuf + 6) == 0x2a ) &&
			( (*(unsigned int*)(TalkBoxDataBuf + 8) == CalleeID) ||
			(*(unsigned int*)(TalkBoxDataBuf + 12) == CalleeID) ) )
		{
			TalkBoxDataBuf += 128;
			continue;
		}

		else if( ( *(unsigned short*)(TalkBoxDataBuf + 8) == 0x2a ) &&
			( (*(unsigned int*)(TalkBoxDataBuf + 10) == CallerID) ||
			(*(unsigned int*)(TalkBoxDataBuf + 14) == CallerID) ) )
		{
			TalkBoxDataBuf += 130;
			continue;
		}
		else if( ( *(unsigned short*)(TalkBoxDataBuf + 8) == 0x2a ) &&
			( (*(unsigned int*)(TalkBoxDataBuf + 10) == CalleeID) ||
			(*(unsigned int*)(TalkBoxDataBuf + 14) == CalleeID) ) )
		{
			TalkBoxDataBuf += 130;
			continue;
		}

		else if(*(unsigned short*)(TalkBoxDataBuf + 6) == 0x2a ) 
		{
			TalkBoxDataBuf += 128;
			continue;
		}
		else if(*(unsigned short*)(TalkBoxDataBuf + 8) == 0x2a ) 
		{
			TalkBoxDataBuf += 130;
			continue;
		}
		else
		{
			//MessageBox(0,"TalkBox Audio Data Parse Error!\n","TalkBox Audio Data Parse Error!\n",MB_OK);
			printf("TalkBox Audio Data Parse Error!\n");
		}

		TalkBoxDataBuf += 128;
	}

	memmove(DataBuf->buf,ParseBuf,ParsePointer - ParseBuf);
	DataBuf->len = ParsePointer - ParseBuf;
	DataBuf->ActualLen = ParsePointer - ParseBuf;
	VirtualFree(ParseBuf, ParseDataSize, MEM_DECOMMIT);
	VirtualFree(ParseBuf, 0 ,MEM_RELEASE);
	return true;
}








BOOL __stdcall CodecGSM( LPDATABUF DataBuf)
{
	int GSMbufSize = DataBuf->len;
	if(GSMbufSize < 3000)
	{
		return false;
	}
	unsigned char * GSMbuffer = DataBuf->buf;

	GSMstate * GsmInfo = gsm_create();
	if (!GsmInfo ) 
	{
		printf("gsm_create error!\n");
		return false;
	}
	int GSM_OPT_VERBOSE = 0;
	int GSM_OPT_FAST = 0;
	int Result = gsm_option(GsmInfo, 2 , &GSM_OPT_FAST);
	Result = gsm_option(GsmInfo, 1 , &GSM_OPT_VERBOSE);

	short* PCMbuffer =(short*)VirtualAlloc(0, GSMbufSize * 10,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
	if(!PCMbuffer)
	{
		printf("VirtualAlloc Error!\n");
		return false;
	}
	DataBuf->ActualLen = GSMbufSize * 10;
	short * PcmBuf = PCMbuffer;

	for( ; GSMbufSize > 0 ; GSMbufSize -= 33)
	{
		if (gsm_decode(GsmInfo, GSMbuffer, PCMbuffer)) 
		{
			printf("gsm_decode error!\n");
			gsm_destroy(GsmInfo);
			VirtualFree(PCMbuffer,GSMbufSize * 10,MEM_DECOMMIT);
			VirtualFree(PCMbuffer,0,MEM_RELEASE);
			return false;
		}
		GSMbuffer += 33;
		PCMbuffer +=160;
	}
	gsm_destroy(GsmInfo);

	DataBuf->buf = (unsigned char *)PcmBuf;
	DataBuf->len = (unsigned int)PCMbuffer - (unsigned int)PcmBuf;
	return true;
}







BOOL __stdcall FormatEnumProc (HACMDRIVERID acmDrvID, LPACMFORMATDETAILS pacmFormatDetail,
							   DWORD Instance, DWORD DrvSupport) 
{ 
 	printf("%4.4lXH, %s\n", pacmFormatDetail->dwFormatTag, pacmFormatDetail->szFormat); 
	//长整形大写16进制数字，输出格式4.4，后面跟H字符
	return TRUE; 
} 

//#define unsigned __int3264  ULONG_PTR 
//__stdcall是微软编译器的用法，_stdcall是C++语法的用法，在MS编译器中两者完全一样，在非MS编译器中__stdcall报错
BOOL __stdcall CodecsEnumProc(HACMDRIVERID acmDrvID, unsigned long acmDrvIDpointer, DWORD DrvSupport) 
{ 
	if (!(DrvSupport & ACMDRIVERDETAILS_SUPPORTF_CODEC ) )	//ACMDRIVERDETAILS_SUPPORTF_CODEC = 1
	{
		return TRUE;
	}

	ACMDRIVERDETAILS acmDrvDetail; 
	RtlZeroMemory(&acmDrvDetail,sizeof(ACMDRIVERDETAILS));
	acmDrvDetail.cbStruct = sizeof(ACMDRIVERDETAILS); 
	MMRESULT mmr = acmDriverDetails(acmDrvID, &acmDrvDetail, 0); 
	if (mmr)
	{
		printf("acmDriverDetails error,error code :%d\n",GetLastError()) ; 
		return TRUE;	
		//必须返回TRUE,WINDOWS才会继续枚举下一个驱动直到枚举全部完成 .否则WINDOWS返回,驱动枚举会中断
	}
	printf("acm驱动全称: %s\n", acmDrvDetail.szLongName); 
 	printf("acm驱动描述: %s\n", acmDrvDetail.szFeatures); 

	HACMDRIVER acmDrv = NULL; 
	mmr = acmDriverOpen(&acmDrv, acmDrvID, 0); 
	if (mmr)
	{
		printf("acmDriverOpen error,error code :%d\n",GetLastError()) ; 
		return TRUE;
	}

	DWORD MaxSize = 0; 
	mmr = acmMetrics(0, ACM_METRIC_MAX_SIZE_FORMAT, &MaxSize); 
	if (mmr)
	{
		acmDriverClose(acmDrv, 0); 
		printf("acmMetrics error,error code :%d\n",GetLastError()) ; 
		return TRUE;
	}

	LPWAVEFORMATEX WaveFormatEx = (LPWAVEFORMATEX)malloc(MaxSize);
	memset(WaveFormatEx , 0, MaxSize); 
	WaveFormatEx->cbSize		= LOWORD(MaxSize) - sizeof(WAVEFORMATEX); 
	WaveFormatEx->wFormatTag	= WAVE_FORMAT_UNKNOWN; 

	ACMFORMATDETAILS FormatDetail; 
	memset(&FormatDetail, 0, sizeof(ACMFORMATDETAILS)); 
	FormatDetail.cbStruct		= sizeof(FormatDetail); 
	FormatDetail.pwfx			= WaveFormatEx; 
	FormatDetail.cbwfx			= MaxSize; 
	FormatDetail.dwFormatTag	= WAVE_FORMAT_UNKNOWN; 
	mmr = acmFormatEnum(acmDrv, &FormatDetail, FormatEnumProc, 0, 0);  
	//FormatDetail.fdwSupport跟上面的DrvSupport含义一样
	if (mmr)
	{
		printf("acmFormatEnum error,error code :%d\n",GetLastError()) ; 
	}

	if (FormatDetail.dwFormatTag == 0x42)
	{
		*(LPHACMDRIVERID)acmDrvIDpointer = acmDrvID;
	}

	free(WaveFormatEx); 
	acmDriverClose(acmDrv, 0); 
	return TRUE; 
}


//	WAVE_FORMAT_G7xx_ADPCM
// 	WAVE_FORMAT_G7xx_CELP 
// 	WAVE_FORMAT_Gxx
//  msg723编码时WAVEFORMATEX中的extra字段: char [18] = {0x42,0,1,0,0x40,0x1f,0,0,0x20,3,0,0,0x18,0,0,0,0xa,0}
//  msg723解码时WAVEFORMATEX中的extra字段: char [10] = {2,0,0xce,0x9a,0x32,0xf7,0xa2,0xae,0xde,0xac};
BOOL __stdcall msG723Decode(LPDATABUF DataBuf)
{
	if(DataBuf->len < 3000)
	{
		return false;
	}

	HACMDRIVERID acmDrvID = 0;
	MMRESULT mmr = acmDriverEnum(CodecsEnumProc, (unsigned long)&acmDrvID, 0);
	if (!acmDrvID)
	{
		printf("acmDriverEnum error,error code :%d\n",GetLastError());
		return false;
	}
	HACMDRIVER acmDrv = 0;
	mmr = acmDriverOpen(&acmDrv, acmDrvID, 0);
	if (!acmDrv)
	{
		printf("acmDriverOpen error,error code :%d\n",GetLastError());
		return false;
	}

    DWORD maxSize = 0;
    mmr = acmMetrics(NULL, ACM_METRIC_MAX_SIZE_FORMAT, &maxSize);
    if(mmr != MMSYSERR_NOERROR)
	{
		acmDriverClose(acmDrv,0);
		printf("acmMetrics error,error code :%d\n",GetLastError());
		return false;
	}

    LPWAVEFORMATEX sourceFormat = (LPWAVEFORMATEX) new char [maxSize];
    LPWAVEFORMATEX destFormat   = (LPWAVEFORMATEX) new char [maxSize];
    memset(sourceFormat, 0, maxSize);
    memset(destFormat, 0, maxSize);

	//注意头的填充格式要严格要求，必须严格按照SDK的说明填写信息,msg723转换为PCM时的参数是固定的
    sourceFormat->wFormatTag		= 0x42;				//MSG723的数据格式标志,msg723 = 0x42
    sourceFormat->nChannels			= 1;
    sourceFormat->nSamplesPerSec	= 8000;
    sourceFormat->nAvgBytesPerSec	= 800;				//MSG723的数据格式标志,DSP TRUESPEECH压缩比10:1
    sourceFormat->nBlockAlign		= 24;
    sourceFormat->wBitsPerSample	= 0;				//MSG723的数据格式标志,采样位数除以8
    sourceFormat->cbSize			= 10;
	unsigned char msG723extra[10] = {0x02,0x00,0xce,0x9a,0x32,0xf7,0xa2,0xae,0xde,0xac};
	char * SrcFormatExtraData = (char * )((unsigned int)sourceFormat + sizeof(WAVEFORMATEX));
	memmove( SrcFormatExtraData, msG723extra, 10);

    destFormat->wFormatTag			= WAVE_FORMAT_PCM;
    destFormat->nChannels			= 1;
    destFormat->nSamplesPerSec		= 8000;
    destFormat->nAvgBytesPerSec		= 16000;
    destFormat->nBlockAlign			= 2;  
	//压缩比20:1,24字节转换后为480字节,留出足够的空间够WINDOWS解压计算使用
    destFormat->wBitsPerSample		= 16;
    destFormat->cbSize				= 0;

	HACMSTREAM mStreamHandler;
	//为了提高解码声音的质量不要指定ACM_STREAMOPENF_NONREALTIME
    mmr = acmStreamOpen(&mStreamHandler,acmDrv, sourceFormat, destFormat, NULL, 0, 0,0); 
    if(mmr != MMSYSERR_NOERROR)
	{
		delete[] sourceFormat;
		delete[] destFormat;
		acmDriverClose(acmDrv,0);
		printf("acmStreamOpen error,error code :%d\n",GetLastError());
		return false;
	}

    DWORD suggestedDestSize = 0;
    mmr = acmStreamSize(mStreamHandler, DataBuf->len, &suggestedDestSize, ACM_STREAMSIZEF_SOURCE);
	if ( (mmr!= MMSYSERR_NOERROR) || (suggestedDestSize < DataBuf->len * 20)  )		 
	{
		delete[] sourceFormat;
		delete[] destFormat;
		acmStreamClose(mStreamHandler, 0);
		acmDriverClose(acmDrv,0);
		printf("acmStreamSize error,error code :%d\n",GetLastError());
		return false;
	}

	unsigned char * PCMbuffer =(unsigned char*)VirtualAlloc(0, suggestedDestSize,
		MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);	
	if(!PCMbuffer)
	{
		delete[] sourceFormat;
		delete[] destFormat;
		acmStreamClose(mStreamHandler, 0);
		acmDriverClose(acmDrv,0);
		printf("VirtualAlloc Error,error code :%d\n",GetLastError());
		return false;
	}
	DataBuf->ActualLen = suggestedDestSize;

	ACMSTREAMHEADER mAcmheader ;
    memset(&mAcmheader, 0, sizeof(ACMSTREAMHEADER) );
    mAcmheader.cbStruct    = sizeof(ACMSTREAMHEADER);
    mAcmheader.cbSrcLength = DataBuf->len;
    mAcmheader.pbSrc       = DataBuf->buf;				//音频格式转换过程中,源数据缓冲地址不能改变
    mAcmheader.cbDstLength = suggestedDestSize;
    mAcmheader.pbDst       = (unsigned char *)PCMbuffer;

    mmr = acmStreamPrepareHeader(mStreamHandler, &mAcmheader, 0);
    if (mmr != MMSYSERR_NOERROR)
    {
		delete[] sourceFormat;
		delete[] destFormat;
		acmStreamClose(mStreamHandler, 0);
		acmDriverClose(acmDrv,0);
		VirtualFree(PCMbuffer,suggestedDestSize ,MEM_DECOMMIT);
		VirtualFree(PCMbuffer,0,MEM_RELEASE);
		printf("acmStreamPrepareHeader error,error code :%d\n",GetLastError());
		return false;
	}

	//所有音频源数据的数据格式的一次性的转换完成，不需要向网络电话软件那样每一个包转换一次
	mmr  = acmStreamConvert(mStreamHandler, &mAcmheader, ACM_STREAMCONVERTF_BLOCKALIGN);
	if( (mmr != MMSYSERR_NOERROR) || (mAcmheader.cbDstLengthUsed != suggestedDestSize) )
	{
		printf("acmStreamConvert error,error code :%d\n",GetLastError());
		return false;
	}

	acmStreamUnprepareHeader(mStreamHandler, &mAcmheader, 0);
	delete[] sourceFormat;
	delete[] destFormat;
	acmStreamClose(mStreamHandler, 0);
	acmDriverClose(acmDrv,0);

	DataBuf->buf = (unsigned char *)PCMbuffer;
	DataBuf->len = suggestedDestSize;
	return true;
}


 






BOOL __stdcall G721ACodec( LPDATABUF DataBuf)
{
	unsigned int G721BufSize = DataBuf->len;
	if(G721BufSize < 3000)
	{
		return true;
	}
	unsigned char * G721Buffer = (unsigned char *)DataBuf->buf;	//3b to 2B

	short* PCMbuffer =(short*)VirtualAlloc(0, G721BufSize << 2,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);	
	if(!PCMbuffer)
	{
		printf("VirtualAlloc Error!\n");
		return false;
	}
	DataBuf->ActualLen = G721BufSize << 2;
	short * PcmBuf = PCMbuffer;

	int Counter = DecodeG721(PCMbuffer, (int*)G721Buffer, G721BufSize);
	if (Counter == 0)
	{
		VirtualFree(PCMbuffer,G721BufSize << 2,MEM_DECOMMIT);
		VirtualFree(PCMbuffer,0,MEM_RELEASE);
		printf("G721 conversion error !\n");
		return false;
	}

	DataBuf->buf = (unsigned char *)PcmBuf;
	DataBuf->len = Counter;
	return true;
}









BOOL __stdcall G723ACodec( LPDATABUF DataBuf ,int Flag )
{
	unsigned int G723BufSize = DataBuf->len;
	if(G723BufSize < 3000)
	{
		return false;
	}
	unsigned char * G723Buffer =(unsigned char *)DataBuf->buf;	//3b to 2B

	short* PCMbuffer =(short*)VirtualAlloc(0, G723BufSize * 6,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);	
	if(!PCMbuffer)
	{
		printf("VirtualAlloc Error!\n");
		return false;
	}
	DataBuf->ActualLen = G723BufSize * 6;
	short * PcmBuf = PCMbuffer;

	int Counter;
	if (Flag )
	{
		Counter = DecodeG723_24(PCMbuffer, (int*)G723Buffer, G723BufSize);
	}
	else
	{
		Counter = DecodeG723_40(PCMbuffer, (int*)G723Buffer, G723BufSize);
	}

	if (Counter == 0)
	{
		VirtualFree(PCMbuffer,G723BufSize * 6,MEM_DECOMMIT);
		VirtualFree(PCMbuffer,0,MEM_RELEASE);
		printf("G723 conversion error !\n");
		return false;
	}

	DataBuf->buf = (unsigned char *)PcmBuf;
	DataBuf->len = Counter;
	return true;
}












//将8位无符号PCMU编码转换为16位的PCM编码
BOOL __stdcall DecodePCMU( LPDATABUF DataBuf)
{
	unsigned int PCMUdataSize = DataBuf->len;
	if(PCMUdataSize < 3000)
	{
		return true;
	}

	unsigned char * PCMUbuffer = DataBuf->buf;
	short* FlatBuffer =	(short*)VirtualAlloc(0, PCMUdataSize<<1, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);	
	if(!FlatBuffer)
	{
		printf("VirtualAlloc Error!\n");
		return false;
	}
	DataBuf->ActualLen = PCMUdataSize<<1;
	short * FlatBuf = FlatBuffer;

	unsigned char	PCMUvar;
	int				PCMvar;
	unsigned int	ConvertSize = PCMUdataSize;

	for(; ConvertSize;ConvertSize --)
	{
 		PCMUvar		= ~(*PCMUbuffer);
		PCMvar = ( (PCMUvar & 0xf) << 3) + 0x84;  //QUANT_MASK = 0xf,BIAS = 0x84,SEG_MASK = 0x70,SEG_SHIFT =4
		PCMvar <<= (PCMUvar & 0x70) >> 4;
	
		* FlatBuffer = ( (PCMUvar & 0x80) ? (0x84 - PCMvar) : (PCMvar - 0x84) );
		FlatBuffer ++;
		PCMUbuffer ++;
	}

	DataBuf->buf = (unsigned char *)FlatBuf;
	DataBuf->len = (unsigned int)FlatBuffer - (unsigned int)FlatBuf;	
	//这么写才是正确的
	return true;
}











//将8位无符号PCMA编码转换为16位的PCM编码
BOOL __stdcall DecodePCMA(LPDATABUF DataBuf)
{
	unsigned int PCMAdataSize = DataBuf->len;
	if(PCMAdataSize < 3000)
	{
		return true;
	}
	unsigned char * PCMAbuffer = DataBuf->buf;

	short* FlatBuffer =(short*)VirtualAlloc(0,PCMAdataSize<<1, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);	
	if(!FlatBuffer)
	{
		printf("VirtualAlloc Error!\n");
		return false;
	}
	DataBuf->ActualLen = PCMAdataSize<<1;
	short * FlatBuf = FlatBuffer;

	unsigned char	PCMAvar;
	signed short	PCM16var;
	unsigned int	Segvar;
	unsigned int	ConvertSize = PCMAdataSize;

	for(; ConvertSize;ConvertSize --)
	{
 		PCMAvar		= (*PCMAbuffer) ^ 0x55;
		PCM16var	= (PCMAvar & 0xf) <<4;
		Segvar		= (PCMAvar & 0x70)>>4;
		switch(Segvar)
		{
		case 0:
			PCM16var +=8;
			break;

		case 1:
			PCM16var +=0x108;
			break;

		default:
			PCM16var += 0x108;
			PCM16var <<= Segvar -1;
		}
		
		* FlatBuffer = (PCMAvar & 0x80) ? PCM16var : -PCM16var ;
		FlatBuffer ++;
		PCMAbuffer ++;
	}

	DataBuf->buf = (unsigned char *)FlatBuf;
	DataBuf->len = (unsigned int)FlatBuffer - (unsigned int)FlatBuf;
	//为什么要这么写？指针相减到底意味着什么？
	return true;
}






//Flags = 0 to indicate that we have a correct frame,
//else we lost the current frame,
//then decoder will perform subsitution with the last frame
BOOL __stdcall DecodeG729A(LPDATABUF DataBuf)
{
	int EncodeDataSize = DataBuf->len;
	if(EncodeDataSize < 3000)
	{
		return false;
	}
	unsigned char * EncodeBuf = DataBuf->buf;

	short* DecodeBuf = (short*)VirtualAlloc(0,EncodeDataSize<<6,MEM_COMMIT | MEM_RESERVE,PAGE_READWRITE);		
	if(!DecodeBuf)
	{
		printf("VirtualAlloc Error!\n");
		return false;
	}
	DataBuf->ActualLen = EncodeDataSize<<6;
	short* PcmBuf = DecodeBuf;

	va_g729a_init_decoder();

	for( ; EncodeDataSize  >0 ; )
	{	
		va_g729a_decoder(EncodeBuf , DecodeBuf, 0);
		EncodeBuf += 10;
		DecodeBuf += 80;
		EncodeDataSize -= 10;
	}

	DataBuf->len = (unsigned int)DecodeBuf - (unsigned int)PcmBuf;
	//为什么要这么写？指针相减到底意味着什么？
	DataBuf->buf = (unsigned char *)PcmBuf;
	return true;
}










BOOL __stdcall DecodeSpeex(LPDATABUF DataBuf,LPVOIPFILEHEADER VoipFileHdr)
{
	int EncodeDataSize = DataBuf->len;
	if(EncodeDataSize < 3000)
	{
		return false;
	}
	unsigned char * EncodeBuf = DataBuf->buf;

	int Result = EncodeDataSize<<4;
	short * DecodeBuf = (short*)VirtualAlloc(0,Result,MEM_COMMIT | MEM_RESERVE,PAGE_READWRITE);		
	if(!DecodeBuf)
	{
		return false;
	}
	DataBuf->ActualLen = Result;
	short* PcmBuf = DecodeBuf;	
		
	void *  DecodeState = speex_decoder_init(&speex_nb_mode);
	if (!DecodeState)
	{
		MessageBoxA(0,"speex_decoder_init error","speex_decoder_init ERROR",MB_OK);
		return false;
	}

	unsigned int DecodeFrameSize = 0;
	Result = speex_decoder_ctl(DecodeState, SPEEX_GET_FRAME_SIZE, &DecodeFrameSize);
	if (Result)
	{
		MessageBoxA(0,"speex_decoder_ctl","speex_decoder_ctl",MB_OK);
		return false;
	}

	int PerceptualPostFilter = 1;
	Result = speex_decoder_ctl(DecodeState, SPEEX_SET_PF, &PerceptualPostFilter);
	if (Result)
	{
		MessageBoxA(0,"speex_decoder_ctl","speex_decoder_ctl",MB_OK);
		return false;
	}

	unsigned int CallerFrameCnt = 0;
	unsigned int CalleeFrameCnt = 0;
	SpeexBits SpxBit;
	speex_bits_init(&SpxBit);

	unsigned int EncodeFrameSize ;
	float DecodeArray[1024];
	while (EncodeBuf < (DataBuf->buf + DataBuf->len) )
	{
		speex_bits_reset(&SpxBit);
		for(int Counter = 0; Counter <1024 ; Counter ++ )
		{
			DecodeArray[Counter] = 0.0;
		}


		EncodeFrameSize = *(unsigned short*)EncodeBuf;
		EncodeBuf += 2;

		speex_bits_read_from(&SpxBit, (char *)EncodeBuf ,EncodeFrameSize );
		EncodeBuf += EncodeFrameSize;
		
		int Remain = 0;
		do 
		{
			Result = speex_decode(DecodeState, &SpxBit, (float*)DecodeArray);
			if( (Result == -1) || (Result == -2) )
			{
				break;
			}
			
			for(int Counter = 0; Counter <160 ; Counter ++ )
			{
				DecodeBuf[Counter] = (SHORT)DecodeArray[Counter];
			}
		
			
			DecodeBuf += DecodeFrameSize;	
			CallerFrameCnt ++;
			
			Remain = speex_bits_remaining(&SpxBit);
		} 
		while (Remain != -1 && Remain != -2);
	
	}

	speex_bits_destroy(&SpxBit);
	speex_decoder_destroy(DecodeState);
	DataBuf->buf = (unsigned char *)PcmBuf;
	DataBuf->len = (unsigned int)DecodeBuf - (unsigned int)PcmBuf;

	VoipFileHdr->CallerPackCnt = CallerFrameCnt;
	VoipFileHdr->CalleePackCnt = 0;
	


	/*
	WAVEHEADER wavHeader;
	memmove(wavHeader.RIFF,"RIFF",4);
	memmove(wavHeader.WAVE,"WAVE",4);
	memmove(wavHeader.FMT,"fmt ",4);
	wavHeader.FormatInfoSize = 0x10;
	wavHeader.PCM	= 1;
	wavHeader.ChannelCnt = 1;
	wavHeader.BlockAlign = 2;
	wavHeader.SamplesPerSec = 8000;
	wavHeader.AvgBytesPerSec = 16000;
	wavHeader.BitsPerSample = 16;
	memmove(wavHeader.DATA , "data", 4);
	wavHeader.DataSize = DataBuf->len;
	wavHeader.WaveSize = DataBuf->len + sizeof(WAVEHEADER ) - 8;

	static int i = 0;
	i ++;
	char fname[256] = {0};
	wsprintf(fname,"c:\\%d.wav", i);
	FILE * fp = fopen(fname,"w");
	if (fp == 0)
	{
		return false;
	}
	fwrite((char*)&wavHeader, sizeof(WAVEHEADER),1,fp);
	fwrite(PcmBuf, DataBuf->len, 1 , fp);
	fclose(fp);
	*/

	return true;
}








//注意动态加载时的步骤
//1: typedef int (*Function)(int ，char *);注意括号里的指针标记意思是函数指针
//2: Function Func = LoadLibrary(hDll,"ABC.dll");
//3: int Num = Func(int ,char *);
//4  必须将动态链接库文件放到exe文件同一目录
//5  def文件格式:LIBRARY ABC EXPORTS EFG HIJ，注意关键字必须大写
BOOL __stdcall CodecILBC(LPDATABUF DataBuf,LPVOIPFILEHEADER VoipHdr)
{
	unsigned int CallerID = VoipHdr->CallerPackCnt;
	unsigned int CalleeID = VoipHdr->CalleePackCnt;
	char UserID[256] = {0};
	wsprintf(UserID,"主叫ID:%d\t被叫ID:%d",CallerID,CalleeID);
	MessageBox(0,UserID,"通话双方的ID",MB_OK);

	int EncodeFrameSize;
	if (VoipHdr->UserData == 20)
	{
		EncodeFrameSize = 38;
	}
	else if (VoipHdr->UserData == 30)
	{
		EncodeFrameSize = 50;
	}
	else
	{
		return false;
	}

	int EncodeDataSize = DataBuf->len;
	if(EncodeDataSize < 3000)
	{
		return false;
	}
	unsigned char * EncodeBuf = DataBuf->buf;


	HMODULE hDll = LoadLibrary("CodecILBC.dll");
	if (hDll == 0)
	{
		return false;
	}
	iLBC_Decode_Init IlbcDecInit = (iLBC_Decode_Init)GetProcAddress(hDll,"initDecode");
	if (IlbcDecInit == 0)
	{
		return false;
	}
	iLBC_Decode IlbcDecode = (iLBC_Decode)GetProcAddress(hDll,"iLBC_decode");
	if (IlbcDecode == 0)
	{
		return false;
	}

	short* DecodeBuf = (short*)VirtualAlloc(0,EncodeDataSize << 6,MEM_COMMIT | MEM_RESERVE,PAGE_READWRITE);		
	if(!DecodeBuf)
	{
		FreeLibrary(hDll);
		printf("VirtualAlloc Error!\n");
		return false;
	}
	DataBuf->ActualLen = EncodeDataSize << 6;
	short* DecodePointer = DecodeBuf;
	
	int Enhance = 1;
	iLBC_Dec_Inst_t IlbcDecInst;
	int DecodeFrameSize = 0;
	DecodeFrameSize = IlbcDecInit(&IlbcDecInst,VoipHdr->UserData,Enhance);
	
// 	int EncodeDeltaOffset = EncodeDataSize % EncodeFrameSize;
// 	if (EncodeDeltaOffset)
// 	{
// 		EncodeBuf += EncodeDeltaOffset;
// 		EncodeDataSize -= EncodeDeltaOffset;
// 	}
	
	int CallerPackCnt = 0;
	float DecodeArray[4096];
	int Counter;
	int DecodeFlag = 1;	//DecodeFlag = 1 is normal decode,DecodeFlag = 0 is noise decode
	for( ; EncodeDataSize >0 ; )
	{	
		IlbcDecode(DecodeArray,EncodeBuf, &IlbcDecInst,DecodeFlag);
		for(Counter = 0; Counter < DecodeFrameSize ; Counter ++)
		{
			DecodeBuf[Counter] = (short)(DecodeArray[Counter]);
		}
		EncodeBuf += EncodeFrameSize;

		DecodeBuf += DecodeFrameSize;

		EncodeDataSize -= EncodeFrameSize;

		CallerPackCnt ++;
	}
	
	FreeLibrary(hDll);
	DataBuf->len = (unsigned int)DecodeBuf - (unsigned int)DecodePointer;
	//为什么要这么写？指针相减到底意味着什么？
	DataBuf->buf = (unsigned char *)DecodePointer;

	VoipHdr->CallerPackCnt = CallerPackCnt;
	VoipHdr->CalleePackCnt	= 0;
	return true;
}














int __stdcall DecodeAmr(LPDATABUF DataBuf)
{
	unsigned int AmrBufSize = DataBuf->len;
	if(AmrBufSize < 3000)
	{
		return true;
	}
	unsigned char * AmrBuffer = (unsigned char *)DataBuf->buf;	

	short* PCMbuffer =(short*)VirtualAlloc(0, AmrBufSize << 6,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);	
	if(!PCMbuffer)
	{
		printf("VirtualAlloc Error!\n");
		return false;
	}
	DataBuf->ActualLen = AmrBufSize <<6;
	short * PcmBuf = PCMbuffer;

	int * destate = (int*)Decoder_Interface_init();

	int FrameCnt = 0;
	int FrameSize = 0;
	while (AmrBuffer < DataBuf->buf + DataBuf->len)
	{
		if (*AmrBuffer == 0x3c)
		{
			FrameSize = 32;
		}
		else if (*AmrBuffer == 4)
		{
			FrameSize = 13;
		}
		else if (*AmrBuffer == 0x0c)
		{
			FrameSize = 14;
		}
		else if (*AmrBuffer == 0x14)
		{
			FrameSize = 16;
		}
		else if (*AmrBuffer == 0x1c)
		{
			FrameSize = 18;
		}
		else if (*AmrBuffer == 0x24)
		{
			FrameSize = 20;
		}
		else if (*AmrBuffer == 0x2c)
		{
			FrameSize = 21;
		}
		else if (*AmrBuffer == 0x34)
		{
			FrameSize = 27;
		}
		else
		{
			AmrBuffer += 1;
			continue;
		}

		Decoder_Interface_Decode(destate, AmrBuffer, PCMbuffer, 0);
		AmrBuffer += FrameSize;
		PCMbuffer += 160;

		FrameCnt ++;
	}

	Decoder_Interface_exit(destate);
	DataBuf->buf = (unsigned char *)PcmBuf;
	DataBuf->len = PCMbuffer - PcmBuf;
	return TRUE;
}





int __stdcall DecodeOpus(LPDATABUF DataBuf,LPVOIPFILEHEADER VoipFileHdr)
{
	unsigned int OpusBufSize = DataBuf->len;
	if(OpusBufSize < 3000)
	{
		return true;
	}
	unsigned char * OpusBuffer = (unsigned char *)DataBuf->buf;	
	
	int PCMBufferSize = OpusBufSize << 6 + 960 * 16;
	short* PCMbuffer =(short*)VirtualAlloc(0, PCMBufferSize,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);	
	if(!PCMbuffer)
	{
		printf("VirtualAlloc Error!\n");
		return false;
	}
	DataBuf->ActualLen = PCMBufferSize;
	short * PcmBuf = PCMbuffer;

	HINSTANCE hDll = LoadLibrary("..\\lib\\libopus-0.dll");
	if (hDll == 0)
	{
		return FALSE;
	}

	OpusDecoderCreate  OpusDecCreate = (OpusDecoderCreate )GetProcAddress(hDll,"opus_decoder_create");
	if (OpusDecCreate == 0)
	{
		return FALSE;
	}
	OpusDecoderInit  OpusDecInit  = (OpusDecoderInit)GetProcAddress(hDll,"opus_decoder_init");
	if (OpusDecInit == 0)
	{
		return FALSE;
	}
	OpusDecoderGetSize  OpusDecGetSize  = (OpusDecoderGetSize)GetProcAddress(hDll,"opus_decoder_get_size");
	if (OpusDecGetSize == 0)
	{
		return FALSE;
	}
	OpusDecode  OpusDec  = (OpusDecode)GetProcAddress(hDll,"opus_decode");
	if (OpusDec == 0)
	{
		return FALSE;
	}
	OpusDecoderDestroy  OpusDecDestroy  = (OpusDecoderDestroy)GetProcAddress(hDll,"opus_decoder_destroy");
	if (OpusDecDestroy == 0)
	{
		return FALSE;
	}

	OpusPacketGetNBFrames OpusPackGetNBFrames = (OpusPacketGetNBFrames)GetProcAddress(hDll,"opus_packet_get_nb_frames");
	if (OpusPackGetNBFrames == 0)
	{
		return FALSE;
	}

	OpusPacketGetBandwidth OpusPackGetBandwidth = (OpusPacketGetBandwidth)GetProcAddress(hDll,"opus_packet_get_bandwidth");
	if (OpusPackGetBandwidth == 0)
	{
		return FALSE;
	}

	
	OpusPacketGetNBChannels OpusPackGetNBChannels = (OpusPacketGetNBChannels)GetProcAddress(hDll,"opus_packet_get_nb_channels");
	if (OpusPackGetNBChannels == 0)
	{
		return FALSE;
	}

	/*
	typedef int (*OpusDecoderGetNBSamples)( const OpusDecoder * dec, const unsigned char packet[ ],opus_int32 len );
	typedef int (*OpusPacketGetBandwidth) ( const unsigned char * data );
	typedef int (*OpusPacketGetNBChannels) ( const unsigned char * data );
	typedef int (*OpusPacketGetNBFrames) ( const unsigned char packet[ ], opus_int32 len );
	typedef int (*OpusPacketGetNBSamples) ( const unsigned char packet[ ], opus_int32 len, opus_int32 Fs );
	typedef int (*OpusPacketGetSamplesPerFrame) ( const unsigned char * data, opus_int32 Fs );
	typedef int (*OpusPacketParse) ( const unsigned char * data, opus_int32 len, unsigned char * out_toc,
								const unsigned char * frames[48], opus_int16 size[48], int * payload_offset );
	*/


	char tmpdata[8192];
	int Channel = 1;
	OpusDecoder * OpusDecState = 0;
	int Error;
	OpusDecState = OpusDecCreate(8000,Channel,&Error);
	if (OpusDecState < 0)
	{
		return FALSE;
	}

	int DecSize = 0;
	int EncodeSize;
	int DecodeSize;
	int FrameCnt = 0;

	try
	{


	while (OpusBuffer < DataBuf->buf + DataBuf->len)
	{
		EncodeSize = *(unsigned short*)OpusBuffer;
		OpusBuffer += 2;

//		memset(tmpdata, 0, 4096);
//		memmove(tmpdata, OpusBuffer, EncodeSize);
//		int i = OpusPackGetNBFrames((unsigned char *)tmpdata, EncodeSize);

		DecodeSize = OpusDec(OpusDecState, OpusBuffer,EncodeSize, (__int16 *)PCMbuffer, 960*6,0);		//960*6 
		if (DecodeSize < 0)
		{		
			OpusBuffer += EncodeSize;
			break;
		}
		OpusBuffer += EncodeSize;

		PCMbuffer += DecodeSize;

		FrameCnt ++;
	}

	}
	catch (...)
	{
		int myint = 0;
	}

	OpusDecDestroy(OpusDecState);
	FreeLibrary(hDll);

	DataBuf->buf = (unsigned char *)PcmBuf;
	DataBuf->len = ((unsigned int)PCMbuffer - (unsigned int)PcmBuf);
	VoipFileHdr->CallerPackCnt = FrameCnt;
	VoipFileHdr->CalleePackCnt = 0;
	return TRUE;
}








int __stdcall VoipDataParse(LPVOIPFILEHEADER VoipFileHdr,LPDATABUF DataBuf)
{
	if ( VoipFileHdr->Protocol  == 1)
	{
		DecodeYMCALL(DataBuf->buf,DataBuf->len);
		CodecGSM( DataBuf);
	}
	else if( ( VoipFileHdr->Protocol  == 2) || ( VoipFileHdr->Protocol  == 3) )
	{
		Decode97CALL((unsigned char *)DataBuf->buf,DataBuf->len,(unsigned char )VoipFileHdr->UserData);
		DecodeG729A( DataBuf);
	}
	else if ( VoipFileHdr->Protocol  == 4)
	{
		DecodeKBCALL(DataBuf->buf,DataBuf->len);
		DecodeG729A( DataBuf);
	}
	else if ( VoipFileHdr->Protocol  == 5)
	{
		DecodeKUBAO(DataBuf->buf,DataBuf->len);
		DecodePCMU( DataBuf);
	}
	else if ( VoipFileHdr->Protocol  == 6 ) 
	{
		DecodeALICALL((unsigned int *)DataBuf->buf,DataBuf->len);
		DecodeG729A( DataBuf);
	}
	else if ( VoipFileHdr->Protocol  == 7 )
	{
		DecodeDYT(DataBuf->buf,DataBuf->len);
		DecodeG729A( DataBuf);
	}
	else if ( VoipFileHdr->Protocol  == 8 )
	{
		DecodeMobileALICALL(DataBuf->buf,DataBuf->len,VoipFileHdr->UserData);
		DecodeG729A( DataBuf);
	}
	else if ( VoipFileHdr->Protocol  == 9 )
	{
		ParseZelloData(VoipFileHdr,DataBuf);
	}
	else if( (VoipFileHdr->Protocol  >= 0x10 ) && ( VoipFileHdr->Protocol  < 0x20 ) ) 
	{
		DecodeG729A(DataBuf);
	}
	else if( ( VoipFileHdr->Protocol  >= 0x20 ) && ( VoipFileHdr->Protocol  < 0x30 ) ) 
	{
		msG723Decode(DataBuf);
	}
	else if( ( VoipFileHdr->Protocol  >= 0x30 ) && ( VoipFileHdr->Protocol  < 0x40 ) ) 
	{
		DecodePCMA(DataBuf);
	}
	else if( ( VoipFileHdr->Protocol  >= 0x40 ) && ( VoipFileHdr->Protocol  < 0x50 ) ) 
	{
		DecodePCMU(DataBuf);
	}
	else if( ( VoipFileHdr->Protocol  >= 0x50 ) && ( VoipFileHdr->Protocol  < 0x60 ) ) 
	{
		CodecGSM( DataBuf);
	}
	else if( ( VoipFileHdr->Protocol  >= 0x60 ) && ( VoipFileHdr->Protocol  < 0x70 ) ) 
	{
//		ParseTalkBoxTcpData(VoipFileHdr,DataBuf);
		CodecILBC(DataBuf,VoipFileHdr);
	}
	else if( ( VoipFileHdr->Protocol  >= 0x70 ) && ( VoipFileHdr->Protocol  < 0x80 ) ) 
	{
		G723ACodec(DataBuf,VoipFileHdr->UserData);
	}
	else if( ( VoipFileHdr->Protocol  >= 0x80 ) && ( VoipFileHdr->Protocol  < 0x90 ) ) 
	{
		G721ACodec(DataBuf);
	}
	else if( ( VoipFileHdr->Protocol  >= 0x90 ) && ( VoipFileHdr->Protocol  < 0xa0 ) ) 
	{
		DecodeAmr(DataBuf);	
	}
	else if( ( VoipFileHdr->Protocol  >= 0xa0 ) && ( VoipFileHdr->Protocol  < 0xb0 ) ) 
	{
		DecodeSpeex(DataBuf, VoipFileHdr);	
	}
	else if( ( VoipFileHdr->Protocol  >= 0xb0 ) && ( VoipFileHdr->Protocol  < 0xc0 ) ) 
	{
		DecodeOpus(DataBuf, VoipFileHdr);	
	}
	else if( ( VoipFileHdr->Protocol  >= 0xc0 ) && ( VoipFileHdr->Protocol  < 0xd0 ) ) 
	{

	}
	else if( ( VoipFileHdr->Protocol  >= 0xd0 ) && ( VoipFileHdr->Protocol  < 0xe0 ) ) 
	{

	}
	else if( ( VoipFileHdr->Protocol  >= 0xe0 ) && ( VoipFileHdr->Protocol  < 0xf0 ) ) 
	{
		//DecodeSpeex(DataBuf, VoipFileHdr);	
	}
	else if( ( VoipFileHdr->Protocol  >= 0xf0 ) && ( VoipFileHdr->Protocol  < 0x100 ) ) 
	{
		char * argv[4];
		argv[0] = "..\\PlayVOIP.exe";
		argv[1] = DataBuf->FileName;
		argv[2] = "silktmpoutputfile";
		argv[3] = "-Fs_API 8000";
		SilkCodec(3,argv,DataBuf,VoipFileHdr);	
	}
	else
	{
		return false;
	}

	return true;
}