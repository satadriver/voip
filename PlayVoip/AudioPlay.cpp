#pragma once
#include "VoipFun.h"
#include "AudioPlay.h"






HWAVEOUT		hWavOut = 0;			//窗口回调中等于LPARAM么?
DATABUF			DataBuf;
WAVEHDR			WavHdr_0;
WAVEHDR			WavHdr_1;
char 			PlayBuf_0[PLAYRATE];
char 			PlayBuf_1[PLAYRATE];

char* AudioDataPtr = 0;		//解码后的音频数据缓冲区，播放完后要释放

int				TIMEINTERVAL = 5;

int				RewindFlag = 0;				//后退标志
int				PictureFlag = 0;
int				LoopFlag = 0;
unsigned int	AudioVolume = 0xffffffff;
VOIPFILEHEADER	VoipFileHdr;












int PlayAudioData(HWND hWnd)
{
	try
	{
		if (PlaySeqFlag == 0)
		{
			if (waveOutWrite(hWavOut, &WavHdr_0, sizeof(WAVEHDR)))
			{
				SendMessage(hWnd, WM_COMMAND, IDC_STOP, 0);
				MessageBoxA(hWnd, "waveOutWrite error!", "waveOutWrite error!", MB_OK);
				return false;
			}
			memmove(PlayBuf_1, AudioDataPtr, PLAYRATE);
			AudioDataPtr += PLAYRATE;
			PlaySeqFlag = 1;
			return true;
		}
		else if (PlaySeqFlag == 1)
		{
			if (waveOutWrite(hWavOut, &WavHdr_1, sizeof(WAVEHDR)))
			{
				SendMessage(hWnd, WM_COMMAND, IDC_STOP, 0);
				MessageBoxA(hWnd, "waveOutWrite error!", "waveOutWrite error!", MB_OK);
				return false;
			}
			memmove(PlayBuf_0, AudioDataPtr, PLAYRATE);
			AudioDataPtr += PLAYRATE;
			PlaySeqFlag = 0;
			return true;
		}
		else
		{
			return false;
		}
		return true;
	}
	catch (...)
	{
		SendMessage(hWnd, WM_COMMAND, IDC_STOP, 0);
		return false;
	}
}




//音频数据块未播放完成时不能中断或者解除锁定
//通过循环改变WAVEHDR结构中的数据地址的方式来实现快进或者快退
int AudioRewind(HWND hWnd)
{
	try
	{
		if ((int)(AudioDataPtr - PLAYRATE * TIMEINTERVAL * RewindFlag) <= (int)DataBuf.buf)
		{
			AudioDataPtr = (char*)DataBuf.buf;
		}
		else
		{
			AudioDataPtr -= PLAYRATE * TIMEINTERVAL * RewindFlag;
		}
		SetDlgItemInt(hWnd, IDC_CURRENTTIME, (AudioDataPtr - (char*)DataBuf.buf) / PLAYRATE, 0);
		SetTimer(hWnd, TimerAudioID, 1000, 0);

		int Result;
		Result = waveOutPause(hWavOut);
		if (Result != MMSYSERR_NOERROR)
		{
			SendMessage(hWnd, WM_COMMAND, IDC_STOP, 0);
			MessageBoxA(hWnd, "waveOutPause error!", "waveOutPause error!", MB_OK);
			return false;
		}

		PlayFlag = 1;
		Result = waveOutReset(hWavOut);
		if (Result != MMSYSERR_NOERROR)
		{
			SendMessage(hWnd, WM_COMMAND, IDC_STOP, 0);
			//MessageBoxA(hWnd, "waveOutReset error!" ,"waveOutReset error!" ,MB_OK);
			return false;
		}

		if (AudioDataPtr + PLAYRATE > (char*)DataBuf.buf + DataBuf.len)
		{
			SendMessage(hWnd, WM_COMMAND, IDC_STOP, 0);
			//MessageBoxA(hWnd, "wave data size low!" ,"wave data size low!" ,MB_OK);
			return false;
		}
		PlayFlag = 0;
		PlaySeqFlag = 0;
		memmove(PlayBuf_0, AudioDataPtr, PLAYRATE);
		AudioDataPtr += PLAYRATE;
		PlayAudioData(hWnd);
		return true;
	}
	catch (...)
	{
		SendMessage(hWnd, WM_COMMAND, IDC_STOP, 0);
		return false;
	}
}









int AudioSpeed(HWND hWnd)
{
	try
	{
		if ((unsigned int)AudioDataPtr + PLAYRATE * TIMEINTERVAL * SpeedFlag >= (unsigned int)(DataBuf.buf + DataBuf.len))
		{
			AudioDataPtr = (char*)DataBuf.buf;
			SendMessage(hWnd, WM_COMMAND, IDC_STOP, 0);
		}
		else
		{
			AudioDataPtr += PLAYRATE * TIMEINTERVAL * SpeedFlag;
		}
		SetDlgItemInt(hWnd, IDC_CURRENTTIME, (AudioDataPtr - (char*)DataBuf.buf) / PLAYRATE, 0);
		SetTimer(hWnd, TimerAudioID, 1000, 0);

		int Result;
		Result = waveOutPause(hWavOut);
		if (Result != MMSYSERR_NOERROR)
		{
			SendMessage(hWnd, WM_COMMAND, IDC_STOP, 0);
			//MessageBoxA(hWnd, "waveOutPause error!" ,"waveOutPause error!" ,MB_OK);
			return false;
		}
		PlayFlag = 1;
		Result = waveOutReset(hWavOut);
		if (Result != MMSYSERR_NOERROR)
		{
			SendMessage(hWnd, WM_COMMAND, IDC_STOP, 0);
			//MessageBoxA(hWnd, "waveOutReset error!" ,"waveOutReset error!" ,MB_OK);
			return false;
		}

		if (AudioDataPtr + PLAYRATE > (char*)DataBuf.buf + DataBuf.len)
		{
			SendMessage(hWnd, WM_COMMAND, IDC_STOP, 0);
			//MessageBoxA(hWnd, "wave data size low!" ,"wave data size low!" ,MB_OK);
			return false;
		}
		PlayFlag = 0;
		PlaySeqFlag = 0;
		memmove(PlayBuf_0, AudioDataPtr, PLAYRATE);
		AudioDataPtr += PLAYRATE;
		PlayAudioData(hWnd);
		return true;

	}
	catch (...)
	{
		SendMessage(hWnd, WM_COMMAND, IDC_STOP, 0);
		return false;
	}
}







int __stdcall PlayAudio(HWND hWnd)
{
	try
	{
		SetDlgItemInt(hWnd, IDC_CURRENTTIME, 0, 0);
		SetDlgItemInt(hWnd, IDC_AUDIOTIME, DataBuf.len / PLAYRATE, 0);

		SetDlgItemInt(hWnd, IDC_CALLERTIME,
			(DataBuf.len / PLAYRATE) * VoipFileHdr.CallerPackCnt / (VoipFileHdr.CallerPackCnt + VoipFileHdr.CalleePackCnt), 0);
		SetDlgItemInt(hWnd, IDC_CALLEETIME,
			(DataBuf.len / PLAYRATE) * VoipFileHdr.CalleePackCnt / (VoipFileHdr.CallerPackCnt + VoipFileHdr.CalleePackCnt), 0);
		WAVEFORMATEX				WavFmtEx;
		WavFmtEx.wFormatTag = WAVE_FORMAT_PCM;
		WavFmtEx.nChannels = 1;
		WavFmtEx.wBitsPerSample = 16;
		WavFmtEx.nAvgBytesPerSec = PLAYRATE;
		WavFmtEx.nBlockAlign = WavFmtEx.wBitsPerSample / 8;
		WavFmtEx.nSamplesPerSec = PLAYRATE / WavFmtEx.nBlockAlign;
		WavFmtEx.cbSize = 0;

		int Result = waveOutOpen(&hWavOut, WAVE_MAPPER, &WavFmtEx, (unsigned long)hWnd, 0, CALLBACK_WINDOW);
		if (Result != MMSYSERR_NOERROR)
		{

			VirtualFree(DataBuf.buf, DataBuf.ActualLen, MEM_DECOMMIT);
			VirtualFree(DataBuf.buf, 0, MEM_RELEASE);
			MessageBoxA(hWnd, "wavOutOpen error!", "wavOutOpen error!", MB_OK);
			return false;
		}

		WavHdr_0.dwBufferLength = PLAYRATE;
		WavHdr_0.dwBytesRecorded = 0;
		WavHdr_0.dwFlags = 0;
		WavHdr_0.dwLoops = 0;
		WavHdr_0.dwUser = 0;
		WavHdr_0.lpData = PlayBuf_0;
		WavHdr_0.lpNext = 0;
		WavHdr_0.reserved = 0;
		Result = waveOutPrepareHeader(hWavOut, &WavHdr_0, sizeof(WAVEHDR));
		if (Result != MMSYSERR_NOERROR)
		{
			waveOutClose(hWavOut);
			VirtualFree(DataBuf.buf, DataBuf.ActualLen, MEM_DECOMMIT);
			VirtualFree(DataBuf.buf, 0, MEM_RELEASE);
			MessageBoxA(hWnd, "waveOutPrepareHeader error!", "waveOutPrepareHeader error!", MB_OK);
			return false;
		}
		WavHdr_1.dwBufferLength = PLAYRATE;
		WavHdr_1.dwBytesRecorded = 0;
		WavHdr_1.dwFlags = 0;
		WavHdr_1.dwLoops = 0;
		WavHdr_1.dwUser = 0;
		WavHdr_1.lpData = PlayBuf_1;
		WavHdr_1.lpNext = 0;
		WavHdr_1.reserved = 0;
		Result = waveOutPrepareHeader(hWavOut, &WavHdr_1, sizeof(WAVEHDR));
		if (Result != MMSYSERR_NOERROR)
		{
			Result = waveOutPrepareHeader(hWavOut, &WavHdr_0, sizeof(WAVEHDR));
			waveOutClose(hWavOut);
			VirtualFree(DataBuf.buf, DataBuf.ActualLen, MEM_DECOMMIT);
			VirtualFree(DataBuf.buf, 0, MEM_RELEASE);
			MessageBoxA(hWnd, "waveOutPrepareHeader error!", "waveOutPrepareHeader error!", MB_OK);
			return false;
		}

		Result = waveOutSetVolume(hWavOut, AudioVolume);
		if (Result != MMSYSERR_NOERROR)
		{
			waveOutUnprepareHeader(hWavOut, &WavHdr_0, sizeof(WAVEHDR));
			waveOutUnprepareHeader(hWavOut, &WavHdr_1, sizeof(WAVEHDR));
			waveOutClose(hWavOut);
			VirtualFree(DataBuf.buf, DataBuf.len, MEM_DECOMMIT);
			VirtualFree(DataBuf.buf, 0, MEM_RELEASE);
			MessageBoxA(hWnd, "waveOutSetVolume error!", "waveOutSetVolume error!", MB_OK);
			return false;
		}

		AudioDataPtr = (char*)DataBuf.buf;
		if (AudioDataPtr + PLAYRATE > (char*)DataBuf.buf + DataBuf.len)
		{
			SendMessage(hWnd, WM_COMMAND, IDC_STOP, 0);
			MessageBoxA(hWnd, "wave data size low!", "wave data size low!", MB_OK);
			return false;
		}
		SpeedFlag = 0;
		RewindFlag = 0;
		PlayFlag = 0;
		PlaySeqFlag = 0;
		memmove(PlayBuf_0, AudioDataPtr, PLAYRATE);
		AudioDataPtr += PLAYRATE;
		PlayAudioData(hWnd);

		SetTimer(hWnd, TimerAudioID, 1000, 0);
		HWND hOpenFile = GetDlgItem(hWnd, IDC_OPENFILE);
		EnableWindow(hOpenFile, FALSE);
		hOpenFile = GetDlgItem(hWnd, IDM_OPENFILE);
		EnableWindow(hOpenFile, FALSE);
		return true;
	}
	catch (...)
	{
		SendMessage(hWnd, WM_COMMAND, IDC_STOP, 0);
		return false;
	}
}












