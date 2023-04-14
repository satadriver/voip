#pragma once
#include <windows.h>


#define  IDD_DIALOG			0X2000
#define  IDC_OPENFILE		0X2001
#define  IDC_PLAY			0X2002
#define	 IDC_PAUSE			0X2003
#define  IDC_RESUME			0x2004
#define  IDC_STOP			0x2005
#define  IDC_QUIT			0x2006
#define	 IDC_SPEED			0x2007
#define  IDC_REWIND			0x2008
#define  IDC_OPENDIRNAME	0X2009
#define  IDC_FILEDIR		0x200a

#define  IDC_PLAYCURRENTTIME		0x2015
#define  IDC_CURRENTTIME			0x2016
#define  IDC_PLAYAUDIOTIME			0x2017
#define  IDC_AUDIOTIME				0x2018

#define  IDC_PLAYCALLERTIME			0X2019
#define  IDC_CALLERTIME				0X201A
#define  IDC_PLAYCALLEETIME			0X201B	
#define  IDC_CALLEETIME				0X201C

#define  IDC_BMPFRAME				0X201D


#define	IDM_MAIN			0X2020
#define  IDM_5				0x2021
#define  IDM_10				0x2022
#define  IDM_15				0x2023
#define  IDM_20				0x2024
#define  IDM_30				0x2025
#define  IDM_PIC_BLISS		0x2028
#define  IDM_PIC_AZUL		0X2029
#define  IDM_OPENFILE		0X202A
#define	 IDM_QUIT			0X202B 
#define  IDM_ABOUT			0x202c
#define	 IDM_LOOPON			0X202E
#define  IDM_LOOPOFF		0X202F
#define  IDM_VOLUMEUP		0X2030
#define  IDM_VOLUMEDOWN		0X2031
#define	 IDM_SOFTWARE		0X2032



#define  IDB_BMPBACK_BLISS		0X20A0
#define  IDB_BMPBACK_AZUL		0X20A1
#define	 ICON_MAIN				0X2080

int __stdcall DlgMain(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int __stdcall AlterBackground(HWND hWnd, int BitmapID);

int __stdcall OpenDecodeFile(char* szFileName, HWND hWnd);