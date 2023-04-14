
#include <windows.h>
#include <shellapi.h>

#include "dlgProc.h"
#include "VoipFun.h"
#include "AudioPlay.h"
#include "utils.h"

#pragma comment(lib,"shell32.lib")

char* FileDataBuf = 0;			//��Ƶ�����ļ�����������Ҫ���Ķ������������������ڱ��ֲ���
int				PlayFlag = 0;				//wavOutReset��־λ
DWORD			FileSizeLow = 0;
int				SpeedFlag = 0;				//�����־
int				PlaySeqFlag = 0;			//˫�������л���־λ

HMENU hMenu;
HINSTANCE hInstance;
HMODULE hModule;
HICON hIcon;
HBITMAP hBitmap;
HWND hWnd;

char			szFileName[1024];



int __stdcall DlgMain(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == MM_WOM_DONE)	//Ϊ���������ŵ������Է�����ǰ�洦��	
	{
		if (PlayFlag == 1)		//4���ж�����˳������
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
			Ofn.lpstrTitle = "VoipPlayer������";
			int Result = GetOpenFileNameA(&Ofn);
			if (Result)
			{
				SetDlgItemText(hWnd, IDC_OPENDIRNAME, szFileName + Ofn.nFileOffset);
			}
		}
		else if ((wParam & 0xffff) == IDM_OPENFILE)
		{
			SendMessage(hWnd, WM_COMMAND, IDC_OPENFILE, 0);	//ע�ⷢ������ķ�ʽ
		}
		else if ((wParam & 0xffff) == IDM_5)		// == ��������ȼ���������� &�����������
		{
			EnableMenuItem(hMenu, IDM_10, MF_ENABLED);
			EnableMenuItem(hMenu, IDM_15, MF_ENABLED);
			EnableMenuItem(hMenu, IDM_20, MF_ENABLED);
			EnableMenuItem(hMenu, IDM_30, MF_ENABLED);
			EnableMenuItem(hMenu, IDM_5, MF_GRAYED);	//����ͬʱ�ƶ�MF_BYPOSITION,������Ч
			TIMEINTERVAL = 5;
			//MessageBoxA(hWnd,"����Ϳ��˵�ʱ�����趨Ϊ5��","����Ϳ��˵�ʱ�����趨Ϊ5��",MB_OK);
		}
		else if ((wParam & 0xffff) == IDM_10)
		{
			EnableMenuItem(hMenu, IDM_5, MF_ENABLED);
			EnableMenuItem(hMenu, IDM_15, MF_ENABLED);
			EnableMenuItem(hMenu, IDM_20, MF_ENABLED);
			EnableMenuItem(hMenu, IDM_30, MF_ENABLED);
			EnableMenuItem(hMenu, IDM_10, MF_GRAYED);
			TIMEINTERVAL = 10;
			//MessageBoxA(hWnd,"����Ϳ��˵�ʱ�����趨Ϊ10��","����Ϳ��˵�ʱ�����趨Ϊ5��",MB_OK);
		}
		else if ((wParam & 0xffff) == IDM_15)
		{
			EnableMenuItem(hMenu, IDM_10, MF_ENABLED);
			EnableMenuItem(hMenu, IDM_5, MF_ENABLED);
			EnableMenuItem(hMenu, IDM_20, MF_ENABLED);
			EnableMenuItem(hMenu, IDM_30, MF_ENABLED);
			EnableMenuItem(hMenu, IDM_15, MF_GRAYED);
			TIMEINTERVAL = 15;
			//MessageBoxA(hWnd,"����Ϳ��˵�ʱ�����趨Ϊ15��","����Ϳ��˵�ʱ�����趨Ϊ5��",MB_OK);
		}
		else if ((wParam & 0xffff) == IDM_20)
		{
			EnableMenuItem(hMenu, IDM_10, MF_ENABLED);
			EnableMenuItem(hMenu, IDM_15, MF_ENABLED);
			EnableMenuItem(hMenu, IDM_20, MF_GRAYED);
			EnableMenuItem(hMenu, IDM_30, MF_ENABLED);
			EnableMenuItem(hMenu, IDM_5, MF_ENABLED);
			TIMEINTERVAL = 20;
			//MessageBoxA(hWnd,"����Ϳ��˵�ʱ�����趨Ϊ20��","����Ϳ��˵�ʱ�����趨Ϊ5��",MB_OK);
		}
		else if ((wParam & 0xffff) == IDM_30)	// == ��������ȼ���������� &
		{
			BOOL Result = EnableMenuItem(hMenu, IDM_10, MF_ENABLED);
			Result = EnableMenuItem(hMenu, IDM_15, MF_ENABLED);
			Result = EnableMenuItem(hMenu, IDM_20, MF_ENABLED);
			Result = EnableMenuItem(hMenu, IDM_30, MF_GRAYED);
			Result = EnableMenuItem(hMenu, IDM_5, MF_ENABLED);
			TIMEINTERVAL = 30;
			//MessageBoxA(hWnd,"����Ϳ��˵�ʱ�����趨Ϊ30��","����Ϳ��˵�ʱ�����趨Ϊ5��",MB_OK);
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
				"VOIP��Ƶ������֧�ִ�������ʵ������˹涨����Ƶ��������׼,�ܹ����Ź��ڰ����ֻ����ڵĴ󲿷�VOIP�����������Ƶ������,\r\
			�ǹ���Ϊ�������VOIP��������طŲ�Ʒ֮һ";
			MessageBox(hWnd, szIntroduce, "����VOIPPLAYER", MB_OK);
		}
		else if ((wParam & 0xffff) == IDM_SOFTWARE)
		{
			char* szSoftWareSupport = \
				"�����֧�ֵ�VOIP�����:\r\n"
				"1\t����\r\n2\t����\r\n3\tUUCALL\r\n4\t����ͨ\r\n5\tͨͨ\r\n6\t����\r\n7\tKC\r\n8\t΢΢\r\n"
				"9\t4G\r\n10\t3G\r\n11\t�ƺ�\r\n12\t�첦\r\n13\t���ϱ�\r\n14\t66CALL\r\n15\t�л�ͨ\r\n16\t����\r\n"
				"17\t�ɴ�\r\n18\t�ƻ�\r\n19\t����ͨ\r\n20\tHHCALL\r\n21\tYMCALL\r\n22\t�ᱦ\r\n23\t97CALL\r\n24\t����ͨ\r\n"
				"25\tSKY\r\n26\t����ͨ\r\n27\t����\r\n28\tͼ��\r\n29\t����ͨ\r\n30\t������\r\n31\tʡǮͨ\r\n32\t΢��\r\n"
				"33\tET�й���\r\n34\t��ͨ\r\n35\t����\r\n36\tȫ��ͨ\r\n37\t�ֺ�\r\n"
				"\t�ȵ�\r\n";
			MessageBox(hWnd, szSoftWareSupport, "�����֧�ֵ�VOIP���", MB_OK);
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
		SendMessage(hWnd, WM_SETICON, ICON_BIG, (long)hIcon);			//ֻ�����������ڱ���������ͼ��,���򲻻���ʾ
		hMenu = LoadMenu(hInstance, (const char*)IDM_MAIN);			//����Ҫ��ʵ�����
		SetMenu(hWnd, hMenu);										//���ز˵�����API//GetSubMenu(hMenu,0);	
		EnableMenuItem(hMenu, IDM_5, MF_GRAYED | MF_BYCOMMAND);
		EnableMenuItem(hMenu, IDM_LOOPOFF, MF_GRAYED | MF_BYCOMMAND);	//ע��MF_BYPOSITION������
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
		SendMessage(hWnd, WM_COMMAND, IDC_STOP, 0);	//������Ƶ�豸�������ڴ��������
		EndDialog(hWnd, 0);

		VirtualFree(FileDataBuf, BUFFERSIZE, MEM_DECOMMIT);
		VirtualFree(FileDataBuf, 0, MEM_RELEASE);
	}
	else
	{
		return FALSE;		//δ�������Ϣ�����﷢�͸�WINDOWS����
	}
	return TRUE;
	//�Ի�������̷��ط�0��ʾ��Ϣ������ɣ����ڴ�����̷���0��ʾ��Ϣ�������
}



//�����Ȼһ���ʹ�ܲ��ܸ��¿ؼ�
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
	// 		MessageBoxA(hWnd,"������Ҫ�򿪵��ļ��Ƿ���VOIPԭʼ�������ļ�!","�ļ���ʽ����",MB_OK);
	// 		return false;
	// 	}

	HANDLE hFileAudio = CreateFile(szFileName, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
		0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFileAudio == INVALID_HANDLE_VALUE)
	{
		MessageBoxA(hWnd, "�޷����ļ��������ļ��Ƿ�����ʹ����!", "���ļ�ʧ��!", MB_OK);
		return false;
	}

	int Result;
	DWORD	FileSizeHigh;
	Result = FileSizeLow = GetFileSize(hFileAudio, &FileSizeHigh);
	if ((Result == 0) || (FileSizeLow == 0) || (FileSizeHigh) || (FileSizeLow > BUFFERSIZE))
	{
		CloseHandle(hFileAudio);
		MessageBoxA(hWnd, "�벻Ҫ�򿪳���4G���ļ�!", "�ļ�̫�����̫С!", MB_OK);
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
		wsprintf(szErrorCode, "��ȡ�ļ����󣬴�����:%d", GetLastError());
		MessageBoxA(hWnd, szErrorCode, "��ȡ�ļ�����!", MB_OK);
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
		MessageBoxA(hWnd, "������Ҫ�򿪵��ļ��Ƿ���VOIPԭʼ�������ļ�!", "�ļ��������������", MB_OK);
		return false;
	}

	memmove((char*)&VoipFileHdr, FileDataBuf, TmpVoipFileHdr->HeaderLen + 16);
	PlayAudio(hWnd);
	return true;
}