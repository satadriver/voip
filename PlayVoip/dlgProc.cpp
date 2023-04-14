
#include <windows.h>
#include <shellapi.h>

#include "dlgProc.h"
#include "VoipFun.h"
#include "AudioPlay.h"
#include "utils.h"

#pragma comment(lib,"shell32.lib")

char* FileDataBuf = 0;			//音频编码文件缓冲区，不要随便改动，整个程序生命周期保持不变
int				PlayFlag = 0;				//wavOutReset标志位
DWORD			FileSizeLow = 0;
int				SpeedFlag = 0;				//快进标志
int				PlaySeqFlag = 0;			//双缓冲区切换标志位

HMENU hMenu;
HINSTANCE hInstance;
HMODULE hModule;
HICON hIcon;
HBITMAP hBitmap;
HWND hWnd;

char			szFileName[1024];



int __stdcall DlgMain(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == MM_WOM_DONE)	//为了声音播放的连贯性放在最前面处理	
	{
		if (PlayFlag == 1)		//4个判断条件顺序不能乱
		{
			return false;
		}
		else if (SpeedFlag)
		{
			AudioSpeed(hWnd);
			SpeedFlag = 0;
		}
		else if (RewindFlag)
		{
			AudioRewind(hWnd);
			RewindFlag = 0;
		}
		else if (AudioDataPtr + PLAYRATE < (char*)DataBuf.buf + DataBuf.len)
		{
			PlayAudioData(hWnd);
		}
		else if (AudioDataPtr + PLAYRATE >= (char*)DataBuf.buf + DataBuf.len)
		{
			if (LoopFlag)
			{
				Sleep(1000);
				AudioDataPtr = (char*)DataBuf.buf;
				PlayAudioData(hWnd);
			}
			else
			{
				SendMessage(hWnd, WM_COMMAND, IDC_STOP, 0);
			}
		}
	}
	else if (uMsg == MM_WOM_CLOSE)
	{
		return false;
	}
	else if (uMsg == MM_WOM_OPEN)
	{
		return false;
	}
	else if (uMsg == WM_TIMER)
	{
		if (wParam == TimerAudioID)
		{
			SetDlgItemInt(hWnd, IDC_CURRENTTIME, (AudioDataPtr - (char*)DataBuf.buf) / PLAYRATE, 0);
		}
		else if (wParam == TimerSecID)
		{
			PictureFlag++;
			SYSTEMTIME SysTime;
			GetLocalTime(&SysTime);
			if ((PictureFlag >= 90) && (SysTime.wMinute % 2))
			{
				PictureFlag = 0;
				SendMessage(hWnd, WM_COMMAND, IDM_PIC_BLISS, IDM_PIC_BLISS);

			}
			else if ((PictureFlag >= 90) && (SysTime.wMinute % 2 == 0))
			{
				PictureFlag = 0;
				SendMessage(hWnd, WM_COMMAND, IDM_PIC_AZUL, IDM_PIC_AZUL);
			}
		}
	}
	else if (uMsg == WM_COMMAND)
	{
		if ((wParam & 0xffff) == IDC_SPEED)
		{
			KillTimer(hWnd, TimerAudioID);
			SpeedFlag++;
		}
		else if ((wParam & 0xffff) == IDC_REWIND)
		{
			KillTimer(hWnd, TimerAudioID);
			RewindFlag++;
		}
		else if ((wParam & 0xffff) == IDC_PAUSE)
		{
			waveOutPause(hWavOut);
		}
		else if ((wParam & 0xffff) == IDC_RESUME)
		{
			waveOutRestart(hWavOut);
		}
		else if ((wParam & 0xffff) == IDC_PLAY)
		{
			OpenDecodeFile(szFileName, hWnd);
		}
		else if ((wParam & 0xffff) == IDC_STOP)
		{
			int Result = waveOutPause(hWavOut);
			PlayFlag = 1;
			waveOutReset(hWavOut);
			waveOutUnprepareHeader(hWavOut, &WavHdr_0, sizeof(WAVEHDR));
			waveOutUnprepareHeader(hWavOut, &WavHdr_1, sizeof(WAVEHDR));
			waveOutClose(hWavOut);
			SetDlgItemInt(hWnd, IDC_CURRENTTIME, 0, 0);
			KillTimer(hWnd, TimerAudioID);
			LoopFlag = 0;

			VirtualFree(DataBuf.buf, DataBuf.ActualLen, MEM_DECOMMIT);
			VirtualFree(DataBuf.buf, 0, MEM_RELEASE);
			HWND hOpenFile = GetDlgItem(hWnd, IDC_OPENFILE);
			EnableWindow(hOpenFile, true);
			hOpenFile = GetDlgItem(hWnd, IDM_OPENFILE);
			EnableWindow(hOpenFile, true);

		}
		else if ((wParam & 0xffff) == IDC_OPENFILE)
		{
			OPENFILENAME Ofn;
			memset(&Ofn, 0, sizeof(OPENFILENAME));
			memset(szFileName, 0, MAX_PATH);
			Ofn.lStructSize = sizeof(OPENFILENAME);
			Ofn.hwndOwner = hWnd;
			Ofn.lpstrFilter = "ALL FILES(*.*)\0*.*\0\0";
			Ofn.lpstrFile = szFileName;
			Ofn.nMaxFile = MAX_PATH;
			Ofn.lpstrFileTitle = szFileName;
			Ofn.nMaxFileTitle = MAX_PATH;
			Ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
			Ofn.lpstrInitialDir = 0;
			Ofn.lpstrTitle = "VoipPlayer播放器";
			int Result = GetOpenFileNameA(&Ofn);
			if (Result)
			{
				SetDlgItemText(hWnd, IDC_OPENDIRNAME, szFileName + Ofn.nFileOffset);
			}
		}
		else if ((wParam & 0xffff) == IDM_OPENFILE)
		{
			SendMessage(hWnd, WM_COMMAND, IDC_OPENFILE, 0);	//注意发送命令的方式
		}
		else if ((wParam & 0xffff) == IDM_5)		// == 运算符优先级高于运算符 &，必须加括号
		{
			EnableMenuItem(hMenu, IDM_10, MF_ENABLED);
			EnableMenuItem(hMenu, IDM_15, MF_ENABLED);
			EnableMenuItem(hMenu, IDM_20, MF_ENABLED);
			EnableMenuItem(hMenu, IDM_30, MF_ENABLED);
			EnableMenuItem(hMenu, IDM_5, MF_GRAYED);	//不能同时制定MF_BYPOSITION,否则无效
			TIMEINTERVAL = 5;
			//MessageBoxA(hWnd,"快进和快退的时间间隔设定为5秒","快进和快退的时间间隔设定为5秒",MB_OK);
		}
		else if ((wParam & 0xffff) == IDM_10)
		{
			EnableMenuItem(hMenu, IDM_5, MF_ENABLED);
			EnableMenuItem(hMenu, IDM_15, MF_ENABLED);
			EnableMenuItem(hMenu, IDM_20, MF_ENABLED);
			EnableMenuItem(hMenu, IDM_30, MF_ENABLED);
			EnableMenuItem(hMenu, IDM_10, MF_GRAYED);
			TIMEINTERVAL = 10;
			//MessageBoxA(hWnd,"快进和快退的时间间隔设定为10秒","快进和快退的时间间隔设定为5秒",MB_OK);
		}
		else if ((wParam & 0xffff) == IDM_15)
		{
			EnableMenuItem(hMenu, IDM_10, MF_ENABLED);
			EnableMenuItem(hMenu, IDM_5, MF_ENABLED);
			EnableMenuItem(hMenu, IDM_20, MF_ENABLED);
			EnableMenuItem(hMenu, IDM_30, MF_ENABLED);
			EnableMenuItem(hMenu, IDM_15, MF_GRAYED);
			TIMEINTERVAL = 15;
			//MessageBoxA(hWnd,"快进和快退的时间间隔设定为15秒","快进和快退的时间间隔设定为5秒",MB_OK);
		}
		else if ((wParam & 0xffff) == IDM_20)
		{
			EnableMenuItem(hMenu, IDM_10, MF_ENABLED);
			EnableMenuItem(hMenu, IDM_15, MF_ENABLED);
			EnableMenuItem(hMenu, IDM_20, MF_GRAYED);
			EnableMenuItem(hMenu, IDM_30, MF_ENABLED);
			EnableMenuItem(hMenu, IDM_5, MF_ENABLED);
			TIMEINTERVAL = 20;
			//MessageBoxA(hWnd,"快进和快退的时间间隔设定为20秒","快进和快退的时间间隔设定为5秒",MB_OK);
		}
		else if ((wParam & 0xffff) == IDM_30)	// == 运算符优先级高于运算符 &
		{
			BOOL Result = EnableMenuItem(hMenu, IDM_10, MF_ENABLED);
			Result = EnableMenuItem(hMenu, IDM_15, MF_ENABLED);
			Result = EnableMenuItem(hMenu, IDM_20, MF_ENABLED);
			Result = EnableMenuItem(hMenu, IDM_30, MF_GRAYED);
			Result = EnableMenuItem(hMenu, IDM_5, MF_ENABLED);
			TIMEINTERVAL = 30;
			//MessageBoxA(hWnd,"快进和快退的时间间隔设定为30秒","快进和快退的时间间隔设定为5秒",MB_OK);
		}
		else if ((wParam & 0xffff) == IDM_LOOPON)
		{
			BOOL Result = EnableMenuItem(hMenu, IDM_LOOPOFF, MF_ENABLED | MF_BYCOMMAND);
			Result = EnableMenuItem(hMenu, IDM_LOOPON, MF_GRAYED | MF_BYCOMMAND | MF_DEFAULT);
			LoopFlag = 1;
		}
		else if ((wParam & 0xffff) == IDM_LOOPOFF)
		{
			BOOL Result = EnableMenuItem(hMenu, IDM_LOOPON, MF_ENABLED | MF_BYCOMMAND | MF_DEFAULT);
			Result = EnableMenuItem(hMenu, IDM_LOOPOFF, MF_GRAYED | MF_BYCOMMAND | MF_DEFAULT);
			LoopFlag = 0;
		}

		else if ((wParam & 0xffff) == IDM_VOLUMEDOWN)
		{
			if (AudioVolume - 0x11111111 < AudioVolume)
			{
				AudioVolume -= 0x11111111;
			}
			int Result = waveOutSetVolume(hWavOut, AudioVolume);
			if (Result != MMSYSERR_NOERROR)
			{
				SendMessage(hWnd, WM_COMMAND, IDC_STOP, 0);
			}
		}
		else if ((wParam & 0xffff) == IDM_VOLUMEUP)
		{
			if (AudioVolume + 0x11111111 > AudioVolume)
			{
				AudioVolume += 0x11111111;
			}
			int Result = waveOutSetVolume(hWavOut, AudioVolume);
			if (Result != MMSYSERR_NOERROR)
			{
				SendMessage(hWnd, WM_COMMAND, IDC_STOP, 0);
			}
		}

		else if ((wParam & 0xffff) == IDM_PIC_AZUL)
		{
			BOOL Result = EnableMenuItem(hMenu, IDM_PIC_BLISS, MF_ENABLED | MF_BYCOMMAND | MF_DEFAULT);
			Result = EnableMenuItem(hMenu, IDM_PIC_AZUL, MF_GRAYED | MF_BYCOMMAND | MF_DEFAULT);
			AlterBackground(hWnd, IDB_BMPBACK_AZUL);
		}
		else if ((wParam & 0xffff) == IDM_PIC_BLISS)
		{
			BOOL Result = EnableMenuItem(hMenu, IDM_PIC_AZUL, MF_ENABLED | MF_BYCOMMAND | MF_DEFAULT);
			Result = EnableMenuItem(hMenu, IDM_PIC_BLISS, MF_GRAYED | MF_BYCOMMAND | MF_DEFAULT);
			AlterBackground(hWnd, IDB_BMPBACK_BLISS);
		}
		else if ((wParam & 0xffff) == IDM_ABOUT)
		{
			char* szIntroduce = \
				"VOIP音频播放器支持大多数国际电信联盟规定的音频编码解码标准,能够播放国内包括手机在内的大部分VOIP软件的语音音频数据流,\r\
			是国内为数不多的VOIP软件语音回放产品之一";
			MessageBox(hWnd, szIntroduce, "关于VOIPPLAYER", MB_OK);
		}
		else if ((wParam & 0xffff) == IDM_SOFTWARE)
		{
			char* szSoftWareSupport = \
				"本软件支持的VOIP软件有:\r\n"
				"1\t有信\r\n2\t飞信\r\n3\tUUCALL\r\n4\t阿里通\r\n5\t通通\r\n6\t爱聊\r\n7\tKC\r\n8\t微微\r\n"
				"9\t4G\r\n10\t3G\r\n11\t云呼\r\n12\t快拨\r\n13\t掌上宝\r\n14\t66CALL\r\n15\t中华通\r\n16\t触宝\r\n"
				"17\t可达\r\n18\t云话\r\n19\t点易通\r\n20\tHHCALL\r\n21\tYMCALL\r\n22\t酷宝\r\n23\t97CALL\r\n24\t户户通\r\n"
				"25\tSKY\r\n26\t九州通\r\n27\t飞音\r\n28\t图信\r\n29\t优络通\r\n30\t爱叮铃\r\n31\t省钱通\r\n32\t微话\r\n"
				"33\tET中国行\r\n34\t必通\r\n35\t飞语\r\n36\t全网通\r\n37\t乐呼\r\n"
				"\t等等\r\n";
			MessageBox(hWnd, szSoftWareSupport, "本软件支持的VOIP软件", MB_OK);
		}
		else if ((wParam & 0xffff) == IDM_QUIT)
		{
			SendMessage(hWnd, WM_CLOSE, 0, 0);
		}
		else if ((wParam & 0xffff) == IDC_QUIT)
		{
			SendMessage(hWnd, WM_CLOSE, 0, 0);
		}
	}
	else if (uMsg == WM_INITDIALOG)
	{
		FileDataBuf = (char*)VirtualAlloc(0, BUFFERSIZE, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		if (!FileDataBuf)
		{
			MessageBoxA(0, "VirtualAlloc error!", "VirtualAlloc error!", MB_OK);
			ExitProcess(0);
		}
		LoopFlag = 0;
		hInstance = GetModuleHandle(0);
		hIcon = LoadIcon(hInstance, (char*)ICON_MAIN);
		SendMessage(hWnd, WM_SETICON, ICON_BIG, (long)hIcon);			//只有这样才能在标题栏增加图标,否则不会显示
		hMenu = LoadMenu(hInstance, (const char*)IDM_MAIN);			//必须要有实例句柄
		SetMenu(hWnd, hMenu);										//加载菜单函数API//GetSubMenu(hMenu,0);	
		EnableMenuItem(hMenu, IDM_5, MF_GRAYED | MF_BYCOMMAND);
		EnableMenuItem(hMenu, IDM_LOOPOFF, MF_GRAYED | MF_BYCOMMAND);	//注意MF_BYPOSITION的作用
		EnableMenuItem(hMenu, IDM_PIC_AZUL, MF_GRAYED | MF_BYCOMMAND);
		LoadImage(hInstance, "back_winter.jpeg", IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);
		hBitmap = LoadBitmap(hInstance, (const char*)IDB_BMPBACK_AZUL);
		SetDlgItemInt(hWnd, IDC_CURRENTTIME, 0, 0);
		SetDlgItemInt(hWnd, IDC_AUDIOTIME, 0, 0);

		memset(szFileName, 0, MAX_PATH);
		memset(PlayBuf_0, 0, PLAYRATE);
		memset(PlayBuf_1, 0, PLAYRATE);
		memset(&DataBuf, 0, sizeof(DATABUF));
		memset(&WavHdr_0, 0, sizeof(WAVEHDR));
		memset(&WavHdr_1, 0, sizeof(WAVEHDR));
		//SetTimer(hWnd,TimerSecID,1000,0);

		SetFocus(GetDlgItem(hWnd, IDC_OPENFILE));

		DragAcceptFiles(hWnd, TRUE);
		return false;
	}
	else if (uMsg == WM_DROPFILES)
	{

		HDROP hDrop = (HDROP)wParam;
		UINT Cnt = DragQueryFile(hDrop, 0xffffffff, 0, 0);
		if ((Cnt > 1 || Cnt <= 0))
		{
			return FALSE;
		}

		//char FileNames[4096] = {0};
		for (int i = 0; i < (int)Cnt; i++)
		{
			int FileNameLen = DragQueryFileA(hDrop, i, szFileName, 4096);
			HWND hEditTxt = GetDlgItem(hWnd, IDC_OPENDIRNAME);
			int Result = SetDlgItemTextA(hWnd, IDC_OPENDIRNAME, szFileName);
		}

		DragFinish(hDrop);
	}
	else if (uMsg == WM_CLOSE)
	{
		KillTimer(hWnd, TimerSecID);
		SendMessage(hWnd, WM_COMMAND, IDC_STOP, 0);	//发送音频设备重启和内存回收命令
		EndDialog(hWnd, 0);

		VirtualFree(FileDataBuf, BUFFERSIZE, MEM_DECOMMIT);
		VirtualFree(FileDataBuf, 0, MEM_RELEASE);
	}
	else
	{
		return FALSE;		//未处理的消息从这里发送给WINDOWS处理
	}
	return TRUE;
	//对话框处理过程返回非0表示消息处理完成，窗口处理过程返回0表示消息处理完成
}



//必须先灰化再使能才能更新控件
int __stdcall AlterBackground(HWND hWnd, int BitmapID)
{
	HWND hWndDir = GetDlgItem(hWnd, IDC_OPENDIRNAME);
	HWND hWndFileDir = GetDlgItem(hWnd, IDC_FILEDIR);

	HWND hWndPLAYAUDIOTIME = GetDlgItem(hWnd, IDC_PLAYAUDIOTIME);
	HWND hWndPLAYCURRENTTIME = GetDlgItem(hWnd, IDC_PLAYCURRENTTIME);
	HWND hWndAudioTime = GetDlgItem(hWnd, IDC_AUDIOTIME);
	HWND hWndCurrentTime = GetDlgItem(hWnd, IDC_CURRENTTIME);

	HWND hWndClose = GetDlgItem(hWnd, IDC_QUIT);
	HWND hWndResume = GetDlgItem(hWnd, IDC_RESUME);
	HWND hWndPause = GetDlgItem(hWnd, IDC_PAUSE);
	HWND hWndOpenFile = GetDlgItem(hWnd, IDC_OPENFILE);
	HWND hWndPlay = GetDlgItem(hWnd, IDC_PLAY);
	HWND hWndStop = GetDlgItem(hWnd, IDC_STOP);
	HWND hWndSpeed = GetDlgItem(hWnd, IDC_SPEED);
	HWND hWndRewind = GetDlgItem(hWnd, IDC_REWIND);

	HWND hWndPLAYCALLERTIME = GetDlgItem(hWnd, IDC_PLAYCALLERTIME);
	HWND hWndPLAYCALLEETIME = GetDlgItem(hWnd, IDC_PLAYCALLEETIME);
	HWND hWndCALLERTime = GetDlgItem(hWnd, IDC_CALLERTIME);
	HWND hWndCALLEETime = GetDlgItem(hWnd, IDC_CALLEETIME);

	EnableWindow(hWndFileDir, 0);
	EnableWindow(hWndDir, 0);
	EnableWindow(hWndPLAYAUDIOTIME, 0);
	EnableWindow(hWndPLAYCURRENTTIME, 0);
	EnableWindow(hWndAudioTime, 0);
	EnableWindow(hWndCurrentTime, 0);
	EnableWindow(hWndClose, 0);
	EnableWindow(hWndResume, 0);
	EnableWindow(hWndPause, 0);
	EnableWindow(hWndOpenFile, 0);
	EnableWindow(hWndPlay, 0);
	EnableWindow(hWndStop, 0);
	EnableWindow(hWndSpeed, 0);
	EnableWindow(hWndRewind, 0);

	EnableWindow(hWndPLAYCALLERTIME, 0);
	EnableWindow(hWndPLAYCALLEETIME, 0);
	EnableWindow(hWndCALLERTime, 0);
	EnableWindow(hWndCALLEETime, 0);

	HBITMAP hBmp = LoadBitmap(hInstance, (const char*)BitmapID);
	SendDlgItemMessage(hWnd, IDC_BMPFRAME, STM_SETIMAGE, IMAGE_BITMAP, (long)hBmp);
	EnableWindow(hWndFileDir, 1);
	EnableWindow(hWndPLAYAUDIOTIME, 1);
	EnableWindow(hWndPLAYCURRENTTIME, 1);
	EnableWindow(hWndClose, 1);
	EnableWindow(hWndResume, 1);
	EnableWindow(hWndPause, 1);
	EnableWindow(hWndOpenFile, 1);
	EnableWindow(hWndPlay, 1);
	EnableWindow(hWndStop, 1);
	EnableWindow(hWndSpeed, 1);
	EnableWindow(hWndRewind, 1);
	EnableWindow(hWndDir, 1);
	EnableWindow(hWndAudioTime, 1);
	EnableWindow(hWndCurrentTime, 1);

	EnableWindow(hWndPLAYCALLERTIME, 1);
	EnableWindow(hWndPLAYCALLEETIME, 1);
	EnableWindow(hWndCALLERTime, 1);
	EnableWindow(hWndCALLEETime, 1);
	return true;
}


int __stdcall OpenDecodeFile(char* szFileName, HWND hWnd)
{
	// 	if (!strstr(szFileName,".media.voip_"))
	// 	{
	// 		MessageBoxA(hWnd,"请检查您要打开的文件是否是VOIP原始数据流文件!","文件格式错误",MB_OK);
	// 		return false;
	// 	}

	HANDLE hFileAudio = CreateFile(szFileName, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
		0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFileAudio == INVALID_HANDLE_VALUE)
	{
		MessageBoxA(hWnd, "无法打开文件，请检查文件是否正在使用中!", "打开文件失败!", MB_OK);
		return false;
	}

	int Result;
	DWORD	FileSizeHigh;
	Result = FileSizeLow = GetFileSize(hFileAudio, &FileSizeHigh);
	if ((Result == 0) || (FileSizeLow == 0) || (FileSizeHigh) || (FileSizeLow > BUFFERSIZE))
	{
		CloseHandle(hFileAudio);
		MessageBoxA(hWnd, "请不要打开超过4G的文件!", "文件太大或者太小!", MB_OK);
		return false;
	}

	Result = SetFilePointer(hFileAudio, 0, 0, FILE_BEGIN);
	if (Result == -1)
	{
		CloseHandle(hFileAudio);
		MessageBoxA(hWnd, "SetFilePointer error!", "SetFilePointer Error!", MB_OK);
		return false;
	}

	DWORD Counter = 0;
	Result = ReadFile(hFileAudio, FileDataBuf, FileSizeLow, &Counter, 0);
	if ((Counter != FileSizeLow) || (Result == 0))
	{
		CloseHandle(hFileAudio);
		char szErrorCode[0x100] = { 0 };
		wsprintf(szErrorCode, "读取文件错误，错误码:%d", GetLastError());
		MessageBoxA(hWnd, szErrorCode, "读取文件错误!", MB_OK);
		return false;
	}
	CloseHandle(hFileAudio);

	LPVOIPFILEHEADER TmpVoipFileHdr = (LPVOIPFILEHEADER)FileDataBuf;

	DataBuf.buf = (unsigned char*)(FileDataBuf + TmpVoipFileHdr->HeaderLen + 16);
	DataBuf.len = FileSizeLow - (TmpVoipFileHdr->HeaderLen + 16);

	memmove(DataBuf.FileName, szFileName, strlen(szFileName));
	Result = VoipDataParse((LPVOIPFILEHEADER)FileDataBuf, &DataBuf);
	if (Result == 0)
	{
		VirtualFree(DataBuf.buf, DataBuf.ActualLen, MEM_DECOMMIT);
		VirtualFree(DataBuf.buf, 0, MEM_RELEASE);
		MessageBoxA(hWnd, "请检查您要打开的文件是否是VOIP原始数据流文件!", "文件数据流解码错误", MB_OK);
		return false;
	}

	memmove((char*)&VoipFileHdr, FileDataBuf, TmpVoipFileHdr->HeaderLen + 16);
	PlayAudio(hWnd);
	return true;
}