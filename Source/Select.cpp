#define NOMINMAX
#include "Main.h"
#include "HTTPClient.h"

#include <windows.h>

#include "rapidjson/document.h"



INT_PTR CALLBACK OBS::SelectDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:{
		LocalizeWindow(hwnd);
		int remember = AppConfig->GetInt(TEXT("Login"), TEXT("remember"));
		if (remember){
			String name = AppConfig->GetString(TEXT("Login"), TEXT("name")); 
			SendMessage(GetDlgItem(hwnd, IDC_EDIT1), WM_SETTEXT, name.Length() + 1, (LPARAM)name.Array());
			String pwd = AppConfig->GetString(TEXT("Login"), TEXT("pwd"));
			SendMessage(GetDlgItem(hwnd, IDC_EDIT3), WM_SETTEXT, pwd.Length() + 1, (LPARAM)pwd.Array());
			int autologin = AppConfig->GetInt(TEXT("Login"), TEXT("auto"));
			SendMessage(GetDlgItem(hwnd, IDC_AUTOLOGIN), BM_SETCHECK, autologin, (LPARAM)0);
			SendMessage(GetDlgItem(hwnd, IDC_REMEMBER), BM_SETCHECK, 1, (LPARAM)0);
			if (autologin){
				//login(name, pwd);
				PostMessage(hwnd, WM_COMMAND, ID_LOGIN, 0);
				break;
			}
		}
		break;
	}
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_LOGIN:{
			String account;
			account.SetLength((UINT)SendMessage(GetDlgItem(hwnd, IDC_EDIT1), WM_GETTEXTLENGTH, 0, 0));
			if (!account.Length())
			{
				OBSMessageBox(hwnd, Str("EnterName"), NULL, 0);
				break;
			}

			SendMessage(GetDlgItem(hwnd, IDC_EDIT1), WM_GETTEXT, account.Length() + 1, (LPARAM)account.Array());

			String pwd;
			pwd.SetLength((UINT)SendMessage(GetDlgItem(hwnd, IDC_EDIT3), WM_GETTEXTLENGTH, 0, 0));
			if (!pwd.Length())
			{
				OBSMessageBox(hwnd, Str("EnterName"), NULL, 0);
				break;
			}

			SendMessage(GetDlgItem(hwnd, IDC_EDIT3), WM_GETTEXT, pwd.Length() + 1, (LPARAM)pwd.Array());


			int remember = SendMessage(GetDlgItem(hwnd, IDC_REMEMBER), BM_GETSTATE, 0, (LPARAM)0);
			if (remember){
				AppConfig->SetInt(TEXT("Login"), TEXT("remember"), remember);
				AppConfig->SetString(TEXT("Login"), TEXT("name"), account);
				AppConfig->SetString(TEXT("Login"), TEXT("pwd"), pwd);
				int autologin = SendMessage(GetDlgItem(hwnd, IDC_AUTOLOGIN), BM_GETSTATE, 0, (LPARAM)0);
				AppConfig->SetInt(TEXT("Login"), TEXT("auto"), autologin);
			}

			EndDialog(hwnd, LOWORD(0));
			break;
		}
		case IDC_REMEMBER:
			break;
		case IDC_AUTOLOGIN:
			break;
		case IDCANCEL:
			EndDialog(hwnd, LOWORD(wParam));
			break;
		}
		break;
	}
	return FALSE;
}
