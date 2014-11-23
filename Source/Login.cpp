
#define NOMINMAX
#include "Main.h"
#include "HTTPClient.h"

#include <windows.h>
#include "Wincrypt.h"

#include "rapidjson/document.h"

String md5(String s){
	HCRYPTPROV hProv = NULL;
	if (CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT) == FALSE)
	{
		return TEXT("");
	}
	HCRYPTPROV hHash = NULL;
	if (CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash) == FALSE)
	{
		return TEXT("");
	}
	int len = tstr_to_utf8_datalen(s);
	char* p = new char[len];
	tstr_to_utf8(s, p, len);
	if (CryptHashData(hHash, (const BYTE *)p, len - 1, 0) == FALSE) 
	{
		return TEXT("");
	}
	delete p;

	DWORD result_len;
	if (CryptGetHashParam(hHash, HP_HASHVAL, NULL, &result_len, 0) == FALSE){
		return TEXT("");
	}
	BYTE* r = new BYTE[result_len + 1];
	if (CryptGetHashParam(hHash, HP_HASHVAL, r, &result_len, 0) == FALSE){
		return TEXT("");
	}

	String result = TEXT("");
	for (unsigned int i = 0; i < result_len; i++){
		result += FormattedString(TEXT("%02x"), r[i]);
	}

	if (CryptDestroyHash(hHash) == FALSE)          //Ïú»Ùhash¶ÔÏó  
	{
		return TEXT("");
	}
	if (CryptReleaseContext(hProv, 0) == FALSE)
	{
		return TEXT("");
	}
	return result;
}

bool login(String account, String pwd, HWND hwnd=NULL)
{
	int responseCode;
	//String timestamp = FormattedString(TEXT("%u"), OSGetTime());
	String timestamp = TEXT("1413519583909");
	String sign = md5(String(TEXT("live.polyv.net")) + account + pwd + timestamp);
	String url;
	url = String(TEXT("http://live.polyv.net/api/v1/teacher_login.json"));
	String params = String(TEXT("number=")) + account + TEXT("&passwd=")
		+ pwd + TEXT("&timestamp=") + timestamp + TEXT("&sign=") + sign;
	//url += "?number=";
	String extraheaders = FormattedString(TEXT("Content-Type: application/x-www-form-urlencoded\r\nContent-Length:%d"),
		params.Length());
	url = url + TEXT("?") + params;
	String s = HTTPGetString(url, NULL, &responseCode, TEXT("GET"));
	rapidjson::Document d;
	int len = tstr_to_utf8_datalen(s);
	char* p = new char[len];
	tstr_to_utf8(s, p, len);
	if (d.ParseInsitu(p).HasParseError()){
		MessageBox(hwnd, TEXT("parse error"), TEXT("error"), MB_OK);
		return false;
	}
	if (strcmp(d["status"].GetString(), "ok") != 0){
		MessageBox(hwnd, utf8_createTstr(d["msg"].GetString()), utf8_createTstr(d["code"].GetString()), MB_OK);
		return false;
	}
	AppConfig->SetString(TEXT("Publish"), TEXT("URL"), utf8_createTstr(d["url"].GetString()));
	AppConfig->SetString(TEXT("Publish"), TEXT("PlayPath"), utf8_createTstr(d["stream"].GetString()));
	AppConfig->SetString(TEXT("Publish"), TEXT("Preview"), utf8_createTstr(d["preview"].GetString()));
	return true;
}

INT_PTR CALLBACK OBS::LoginDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
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

			if (pwd.Length() != 32)
				pwd = md5(pwd);
			if (!login(account, pwd, hwnd))
				break;

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