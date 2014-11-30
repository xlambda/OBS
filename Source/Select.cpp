#define NOMINMAX
#include "Main.h"
#include "HTTPClient.h"

#include <windows.h>
#include <vector>

#include "rapidjson/document.h"


enum {
	ID_DUMMY = 6000,
	ID_COURSES,
	ID_CURRICULUMS,
};

struct Curriculum;
struct Course {
	String courseId;
	String title;
	String subtitle;
	std::vector<Curriculum> curriculums;
};

struct Curriculum {
	String curriculumId;
	String title;
	String startTime;
	String endTime;
	String fmsUrl;
	String stream;
	String playUrl;
};
static std::vector<Course> *pCourseList = NULL;


void setCurriculum(HWND hwnd, std::vector<Curriculum>& curriculums)
{
	SendMessage(hwnd, LB_RESETCONTENT, 0, 0);
	for (int i = 0; i < curriculums.size(); i++){
		SendMessage(hwnd, LB_ADDSTRING, 0, (LPARAM)FormattedString(TEXT("%s"), curriculums[i].curriculumId).Array());
	}
	SendMessage(hwnd, LB_SETCURSEL, 0, 0);
}

INT_PTR CALLBACK OBS::SelectDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:{
		LocalizeWindow(hwnd);

		if (!pCourseList){
			pCourseList = new std::vector < Course > ;
			String courses = AppConfig->GetString(TEXT("Courses"), TEXT("json"));
			rapidjson::Document d;
			int len = tstr_to_utf8_datalen(courses);
			char* p = new char[len];
			tstr_to_utf8(courses, p, len);
			if (d.ParseInsitu(p).HasParseError()){
				MessageBox(hwnd, TEXT("parse error"), TEXT("error"), MB_OK);
				return false;
			}
			if (!d.IsArray()){
				MessageBox(hwnd, TEXT("parse error"), TEXT("error"), MB_OK);
				return false;
			}

			for (rapidjson::Value::ConstValueIterator it = d.Begin(); it != d.End(); it++){
				Course c;
				c.courseId = utf8_createTstr(it->operator[]("courseId").GetString());
				c.title = utf8_createTstr(it->operator[]("title").GetString());
				c.subtitle = utf8_createTstr(it->operator[]("subtitle").GetString());
				const rapidjson::Value& cc = it->operator[]("curriculums");
				if (!cc.IsArray())
					continue;
				for (rapidjson::Value::ConstValueIterator it2 = cc.Begin(); it2 != cc.End(); it2++){
					Curriculum t;
					t.curriculumId = utf8_createTstr(it2->operator[]("curriculumId").GetString());
					t.title = utf8_createTstr(it2->operator[]("title").GetString());
					t.startTime = utf8_createTstr(it2->operator[]("startTime").GetString());
					t.endTime = utf8_createTstr(it2->operator[]("endTime").GetString());
					t.fmsUrl = utf8_createTstr(it2->operator[]("fmsUrl").GetString());
					t.stream = utf8_createTstr(it2->operator[]("stream").GetString());
					t.playUrl = utf8_createTstr(it2->operator[]("playUrl").GetString());
					c.curriculums.push_back(t);
				}
				pCourseList->push_back(c);
			}
			delete[] p;
		}

		HWND hwndTemp;
		hwndTemp = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("LISTBOX"), NULL,
			WS_CHILDWINDOW|WS_VISIBLE|WS_TABSTOP|LBS_HASSTRINGS|WS_VSCROLL|LBS_NOTIFY|LBS_NOINTEGRALHEIGHT|WS_CLIPSIBLINGS,
			0, 0, 0, 0, hwnd, (HMENU)ID_COURSES, 0, 0);
		SendMessage(hwndTemp, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
		DWORD flags = SWP_NOOWNERZORDER|SWP_SHOWWINDOW;
		SetWindowPos(GetDlgItem(hwnd, ID_COURSES), NULL, 40, 55, 170, 190, flags);
		for (int i = 0; i < pCourseList->size(); i++){
			SendMessage(hwndTemp, LB_ADDSTRING, 0, (LPARAM)FormattedString(TEXT("%s"), pCourseList->operator[](i).courseId).Array());
		}
		SendMessage(hwndTemp, LB_SETCURSEL, 0, 0);

		hwndTemp = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("LISTBOX"), NULL,
			WS_CHILDWINDOW|WS_VISIBLE|WS_TABSTOP|LBS_HASSTRINGS|WS_VSCROLL|LBS_NOTIFY|LBS_NOINTEGRALHEIGHT|WS_CLIPSIBLINGS,
			0, 0, 0, 0, hwnd, (HMENU)ID_CURRICULUMS, 0, 0);
		SendMessage(hwndTemp, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
		SetWindowPos(GetDlgItem(hwnd, ID_CURRICULUMS), NULL, 250, 55, 170, 190, flags);
		setCurriculum(hwndTemp, pCourseList->operator[](0).curriculums);
		break;
	}
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_COURSES:{
			if (HIWORD(wParam) == LBN_SELCHANGE){
				HWND hwndCourse = (HWND)lParam;
				UINT id = (UINT)SendMessage(hwndCourse, LB_GETCURSEL, 0, 0);
				if (id < 0 || id > pCourseList->size())
					break;
				Course c = pCourseList->operator[](id);
				setCurriculum(GetDlgItem(hwnd, ID_CURRICULUMS), c.curriculums);
			}
			break;
		}
		case ID_CURRICULUMS:{
			break;
		}
		case IDOK:{
			UINT cid = (UINT)SendMessage(GetDlgItem(hwnd, ID_COURSES), LB_GETCURSEL, 0, 0);
			UINT cuid = (UINT)SendMessage(GetDlgItem(hwnd, ID_CURRICULUMS), LB_GETCURSEL, 0, 0);
			if (cid < 0 || cid > pCourseList->size())
				break;
			Course c = pCourseList->operator[](cid);
			if (cuid < 0 || cuid > c.curriculums.size())
				break;
			Curriculum cu = c.curriculums[cuid];
			AppConfig->SetString(TEXT("Publish"), TEXT("URL"), cu.fmsUrl);
			AppConfig->SetString(TEXT("Publish"), TEXT("PlayPath"), cu.stream);
			AppConfig->SetString(TEXT("Publish"), TEXT("Preview"), cu.playUrl);
			EndDialog(hwnd, 0);
			break;
		}
		case IDCANCEL:
			EndDialog(hwnd, LOWORD(wParam));
			delete pCourseList;
			pCourseList = NULL;
			break;
		default:{
			break;
		}
		}
		break;
	}
	return FALSE;
}
