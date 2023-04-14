
#include <windows.h>
#include "VoipFun.h"
#include "dlgProc.h"


#pragma once



#define BUFFERSIZE			0x4000000	//假设未解码前的最大数据块长度为16M，16*1024*1024/1000 >16000秒
#define PLAYRATE			16000
#define	TimerAudioID				-1
#define TimerSecID					-2	//每个窗口可以有多个计时器，但是每个计时器的ID要区分开来




int PlayAudioData(HWND hWnd);
int AudioRewind(HWND hWnd);
int AudioSpeed(HWND hWnd);
int __stdcall PlayAudio(HWND hWnd);




extern HWAVEOUT			hWavOut;			//窗口回调中等于LPARAM么?
extern DATABUF			DataBuf;
extern WAVEHDR			WavHdr_0;
extern WAVEHDR			WavHdr_1;
extern char 			PlayBuf_0[PLAYRATE];
extern char 			PlayBuf_1[PLAYRATE];

extern char* AudioDataPtr;		//解码后的音频数据缓冲区，播放完后要释放

extern int				TIMEINTERVAL;

extern int				PictureFlag;
extern unsigned int		AudioVolume;
extern VOIPFILEHEADER	VoipFileHdr;

extern int				LoopFlag;
extern int				RewindFlag;				//后退标志
extern int				PlayFlag;				//wavOutReset标志位
extern int				SpeedFlag;				//快进标志
extern int				PlaySeqFlag;			//双缓冲区切换标志位