
#include <windows.h>
#include "dlgProc.h"

int __stdcall WinMain(IN HINSTANCE hInstance, IN HINSTANCE hPrevInstance, IN LPSTR lpCmdLine, IN int nShowCmd)
{

	DialogBoxParamA(hInstance, (CHAR*)IDD_DIALOG, 0, DlgMain, 0);

	ExitProcess(0);
	return true;
}