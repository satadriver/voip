
#include <windows.h>
#include "VoipFun.h"
#include "dlgProc.h"


#pragma once



#define BUFFERSIZE			0x4000000	//����δ����ǰ��������ݿ鳤��Ϊ16M��16*1024*1024/1000 >16000��
#define PLAYRATE			16000
#define	TimerAudioID				-1
#define TimerSecID					-2	//ÿ�����ڿ����ж����ʱ��������ÿ����ʱ����IDҪ���ֿ���




int PlayAudioData(HWND hWnd);
int AudioRewind(HWND hWnd);
int AudioSpeed(HWND hWnd);
int __stdcall PlayAudio(HWND hWnd);




extern HWAVEOUT			hWavOut;			//���ڻص��е���LPARAMô?
extern DATABUF			DataBuf;
extern WAVEHDR			WavHdr_0;
extern WAVEHDR			WavHdr_1;
extern char 			PlayBuf_0[PLAYRATE];
extern char 			PlayBuf_1[PLAYRATE];

extern char* AudioDataPtr;		//��������Ƶ���ݻ��������������Ҫ�ͷ�

extern int				TIMEINTERVAL;

extern int				PictureFlag;
extern unsigned int		AudioVolume;
extern VOIPFILEHEADER	VoipFileHdr;

extern int				LoopFlag;
extern int				RewindFlag;				//���˱�־
extern int				PlayFlag;				//wavOutReset��־λ
extern int				SpeedFlag;				//�����־
extern int				PlaySeqFlag;			//˫�������л���־λ