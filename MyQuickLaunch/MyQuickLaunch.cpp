// MyQuickLaunch.cpp : Defines the entry point for the application.
//

#define _CRT_SECURE_NO_WARNINGS
#include "stdafx.h"
#include "MyQuickLaunch.h"
#include <windowsX.h>
#include <commctrl.h>
#include <time.h>
#include <ctime>
#include <cstdlib>
#include <shellapi.h>
#include <locale>
#include <codecvt>

#pragma comment(lib, "shlwapi.lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib, "ComCtl32.lib")

//For StrCpy, StrNCat
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include <shellapi.h>
#include <msi.h>
#include <Shlobj.h>
#pragma comment(lib, "Msi.lib")


using namespace std;


// Global Variables:
HINSTANCE g_hInstance;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
HWND g_hSearchView;
HWND g_hListProgram;
HWND g_hCheckbox;
HWND g_hStatic;
HWND g_hWnd;
HWND g_hToolbarText;
HHOOK hHook = NULL;
HINSTANCE hinstLib;
HWND hToolbarText, hAnnoTextA, hAnnoTextB, hAnnoTextC, hAnnoTextD, hAnnoTextE, hAnnoTextF, hAnnoTextG;
RECT rcClient;
COLORREF colorList[] = {COLOR_A, COLOR_B, COLOR_C, COLOR_D, COLOR_E, COLOR_F, COLOR_G};
vector<Program> gAppNameList;
vector<Program> gToShowList;
vector<wstring> gFreqProgNameList;
vector<int> gFreqProgramList;
NOTIFYICONDATA notifyIcon;
WCHAR keyword[255];

int remainTime = MAXTIME; //Show statistic every 5 mins
bool isHiding = false;
bool isAutoStatistic = false;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK ScanProgramDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void CALLBACK TimeToShowStatitistic(HWND hwnd, UINT uMsg, UINT timerId, DWORD dwTime);
INT_PTR CALLBACK ViewStatitisticDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);



int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_MYQUICKLAUNCH, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return false;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MYQUICKLAUNCH));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MYQUICKLAUNCH));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_3DFACE+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_MYQUICKLAUNCH);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   g_hInstance = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindowEx(0, szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
	   CW_USEDEFAULT, CW_USEDEFAULT, DEFAULT_WIDTH, DEFAULT_HEIGHT, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return false;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return true;
}



//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	g_hWnd = hWnd;
	//Get current program rectangle
	GetClientRect(hWnd, &rcClient);
	int wndWidth = rcClient.right - rcClient.left;
	int wndHeight = rcClient.bottom - rcClient.top;

	//Swich program messages
	switch (message)
	{
	case WM_CREATE:
	{
					  //start to hook
					  _doInstallHook(hWnd);
					  InitCommonControls();
					  SetupViews(hWnd, wndWidth, wndHeight);

					  RegistryEnumeration();

					  ScanProgramFileX86();
					  ScanProgramFile();

					  SortList(ASCENDING);
					  ReadProgramFrequency(FREQUENCY_FILE_PATH);
					  InsertFrequencyToDb();
					  
					  //Load data
					  gToShowList = gAppNameList;
					  LoadDataToListView(gAppNameList);
	}
		break;


	case WM_KEYDOWN:
	{
					   _doInstallHook(hWnd);

					   if (isHiding) {
						   //bat su kien hook o day
						   // 							ShowWindow(hWnd, SW_SHOWNORMAL);
						   // 							MessageBox(hWnd, L"OK", NULL, 0);
					   }
	}
		break;
	case WM_MYMESSAGE:
	{

						 switch (lParam)
						 {
						 case WM_RBUTTONDOWN:
						 {
												//Get menu item information
												MENUITEMINFO separatorBtn = { 0 };
												separatorBtn.cbSize = sizeof(MENUITEMINFO);
												separatorBtn.fMask = MIIM_FTYPE;
												separatorBtn.fType = MFT_SEPARATOR;

												HMENU hMenu = CreatePopupMenu();

												if (hMenu) {
													InsertMenu(hMenu, -1, MF_BYPOSITION, EXIT, L"Exit");
													InsertMenuItem(hMenu, -1, false, &separatorBtn);
													InsertMenu(hMenu, -1, MF_BYPOSITION, VIEW_STATITISTIC, L"View statitistics");
													InsertMenuItem(hMenu, -1, false, &separatorBtn);
													InsertMenu(hMenu, -1, MF_BYPOSITION, SCAN, L"Scan to build database");
													InsertMenuItem(hMenu, -1, false, &separatorBtn);
													POINT pt;
													GetCursorPos(&pt);
													SetForegroundWindow(hWnd);
													UINT clicked = TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, pt.x, pt.y, 0, hWnd, NULL);


													if (clicked == VIEW_STATITISTIC){
														DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_STATISTIC),
															hWnd, ViewStatitisticDialog);

													}
													else if (clicked == SCAN)
													{
														//scan here
														ScanDatabase();
														LoadDataToListView(gAppNameList);
														gToShowList = gAppNameList;
													}
													else if (clicked == EXIT)
													{
														DestroyWindow(hWnd);
													}
													PostMessage(hWnd, WM_NULL, 0, 0);
													DestroyMenu(hMenu);
												}

						 }
						 default:
							 break;
						 }
	}


	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);

		// Parse the menu selections:
		if ((HWND)lParam == g_hSearchView) {
			if (wmEvent == EN_CHANGE) {
				//triggered on keypress
				GetWindowText(g_hSearchView, keyword, 255);
				SearchList(keyword);
				LoadDataToListView(gToShowList);
			}
		}



		switch (wmId)
		{
		case BN_CLICKED:
			if ((HWND)lParam == g_hCheckbox)
			{
				ClearData();
				ScanDatabase();

				SearchList(keyword);
				LoadDataToListView(gToShowList);

			}
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;

		case IDM_STATISTIC:
		{
							  DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_STATISTIC),
								  hWnd, ViewStatitisticDialog);

		}
			break;

		case ID_AUTO_STATISTIC:
		{
								   if (!isAutoStatistic)
								   {
									   remainTime = MAXTIME;
									   CheckMenuItem(GetMenu(hWnd), ID_AUTO_STATISTIC, MF_BYCOMMAND |
										   MF_CHECKED);
									   //set time tim create dialog
									   SetTimer(hWnd, IDT_TIME_RUN, 1000, (TIMERPROC)TimeToShowStatitistic);
									   isAutoStatistic = true;
								   }
								   else
								   {
									   CheckMenuItem(GetMenu(hWnd), ID_AUTO_STATISTIC, MF_BYCOMMAND |
										   MF_UNCHECKED);
									   KillTimer(hWnd, IDT_TIME_RUN);
									   isAutoStatistic = false;
								   }
		}

		case ID_SCAN:
			//scan here
			ScanDatabase();
			LoadDataToListView(gAppNameList);
			gToShowList = gAppNameList;
			break;

		case IDM_ABOUT:
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_NOTIFY:
	{
					  int nCurSelIndex;

					  //The program has started and loaded all necessary component

					  NMHDR* notifyMess = (NMHDR*)lParam; //Notification Message
					  LPNMTREEVIEW lpnmTree = (LPNMTREEVIEW)notifyMess; //Contains information about a tree-view notification message
					  HTREEITEM currSelected;

					  switch (notifyMess->code)
					  {
					  case NM_DBLCLK:
						  //Get hwndFrom for window handle to the control sending the message
						  //To check whether this event fire by Listview
						  if (notifyMess->hwndFrom == g_hListProgram)
						  {
							  int i = ListView_GetSelectionMark(g_hListProgram);

							  RunProgram(i);
						  }

						  break;
					  case NM_RETURN:
						  if (notifyMess->hwndFrom == g_hListProgram)
						  {

							  int i = ListView_GetSelectionMark(g_hListProgram);

							  RunProgram(i);
						  }
						  break;
					  }
	}
		break;


	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		//Draw elevation
		//Draw elevation
		fillRectangle(hdc, 30, 0, wndWidth - 20, wndHeight - 20, COLOR_WHITE);
		//Draw toolbar
		fillRectangle(hdc, 0, 0, wndWidth, TOOLBAR_HEIGHT, COLOR_TOOLBAR);
		EndPaint(hWnd, &ps);
		break;
	case WM_CTLCOLORSTATIC:
	{
							  HDC hdcStatic = (HDC)wParam;
							  DWORD CtrlID = GetDlgCtrlID((HWND)lParam); //Window Control ID

							  //Set color for static control
							  //Set text and background
							  if ((HWND)lParam == hToolbarText || (HWND)lParam == g_hToolbarText)
							  {
								  SetTextColor(hdcStatic, COLOR_WHITE);
								  SetBkColor(hdcStatic, COLOR_TOOLBAR);
							  }
							  else
							  {
								  SetTextColor(hdcStatic, COLOR_BLACK);
								  SetBkColor(hdcStatic, COLOR_WHITE);
							  }


							  return (BOOL)GetSysColorBrush(COLOR_WINDOW);
	}
		break;
	case WM_CLOSE:
		ShowWindow(hWnd, SW_HIDE);
		break;
	case WM_DESTROY:
		WriteProgramFrequency(FREQUENCY_FILE_PATH);
		_doRemoveHook(hWnd);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)true;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)true;
		}
		break;
	}
	return (INT_PTR)false;
}


HWND CreateListView(long lExtStyle, HWND parentWnd, long ID, HINSTANCE hParentInst, int x, int y, int nWidth, int nHeight, long lStyle)
{
	//Create
	HWND m_hListView = CreateWindowEx(lExtStyle, WC_LISTVIEW, _T("List View"),
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP | lStyle,
		x, y, nWidth, nHeight, parentWnd, (HMENU)ID, hParentInst, NULL);


	//Init 2 columns
	LVCOLUMN lvCol;

	//Let the LVCOLUMN know that we will set the format, header text and width of it
	lvCol.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
	lvCol.fmt = LVCFMT_LEFT;

	lvCol.cx = 350;
	lvCol.pszText = _T("Name");
	ListView_InsertColumn(m_hListView, 0, &lvCol);

	lvCol.fmt = LVCFMT_LEFT;
	lvCol.pszText = _T("Version");
	lvCol.cx = 75;
	ListView_InsertColumn(m_hListView, 1, &lvCol);

	return m_hListView;
}



void LoadDataToListView(vector<Program> list)
{
	ListView_DeleteAllItems(g_hListProgram);
	for (int i = 0; i < list.size(); i++)
	{
		//Add to ListView
		LV_ITEM lv;

		lv.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;

		//Insert type to first column
		lv.iItem = i;
		lv.iSubItem = 0;
		lv.pszText = (LPWSTR)list.at(i).displayName.c_str();
		//lv.iImage
		//lv.lParam
		ListView_InsertItem(g_hListProgram, &lv);

		if (!list.at(i).displayVersion.empty())
		{
			//Load (Type, Size, Free Space)
			lv.mask = LVIF_TEXT;

			//Load Drives's Type to second column
			lv.iSubItem = 1;
			lv.pszText = (LPWSTR)list.at(i).displayVersion.c_str();
			ListView_SetItem(g_hListProgram, &lv); //Sets some or all of a list - view item's attributes.
		}	
	}
}

void ClearData()
{
	gToShowList.clear();
	gAppNameList.clear();
}

wstring fixExecutablePath(wstring path)
{
	int index = path.find(L"exe");
	return path.substr(0, index + 3);
}

void setChartAnnotation(HWND staticX, int i, vector<Program>sort, int sum){
	ShowWindow(staticX, SW_SHOWNOACTIVATE);
	wstring temp = to_wstring(100 * (sort[i].frequency) / sum) + L"\%" + L": " + sort[i].displayName;
	SetWindowText(staticX, temp.c_str());
}


INT_PTR CALLBACK ViewStatitisticDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	static HBRUSH dlgTextBackground;

	GetClientRect(hDlg, &rcClient);
	int dlgWidth = rcClient.right - rcClient.left;
	int dlgHeight = rcClient.bottom - rcClient.top;
	HFONT hFont = hFont = CreateFont(25, 0, 0, 0, FW_SEMIBOLD, false, false, false, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"San serif");

	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		WriteProgramFrequency(FREQUENCY_FILE_PATH);
		ReadProgramFrequency(FREQUENCY_FILE_PATH);

		hToolbarText = CreateWindowEx(0, L"STATIC", L"Statistic", WS_CHILD | WS_VISIBLE, 20, 10, 75, 24, hDlg, NULL, NULL, NULL);
		SendMessage(hToolbarText, WM_SETFONT, (WPARAM)hFont, true);

		hFont = CreateFont(24, 7, 0, 0, FW_EXTRALIGHT, true, false, false, ANSI_CHARSET,
			OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
			DEFAULT_PITCH | FF_DONTCARE, TEXT("Segoe UI"));
		hAnnoTextA = CreateWindowEx(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE, dlgWidth * 9 / 15, dlgHeight / 10 + TOOLBAR_HEIGHT, 200, 20, hDlg, NULL, NULL, NULL);
		SendMessage(hAnnoTextA, WM_SETFONT, (WPARAM)hFont, true);
		hAnnoTextB = CreateWindowEx(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE, dlgWidth * 9 / 15, dlgHeight / 5 + TOOLBAR_HEIGHT, 200, 20, hDlg, NULL, NULL, NULL);
		SendMessage(hAnnoTextB, WM_SETFONT, (WPARAM)hFont, true);
		hAnnoTextC = CreateWindowEx(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE, dlgWidth * 9 / 15, dlgHeight * 3 / 10 + TOOLBAR_HEIGHT, 200, 20, hDlg, NULL, NULL, NULL);
		SendMessage(hAnnoTextC, WM_SETFONT, (WPARAM)hFont, true);
		hAnnoTextD = CreateWindowEx(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE, dlgWidth * 9 / 15, dlgHeight * 4 / 10 + TOOLBAR_HEIGHT, 200, 20, hDlg, NULL, NULL, NULL);
		SendMessage(hAnnoTextD, WM_SETFONT, (WPARAM)hFont, true);
		hAnnoTextE = CreateWindowEx(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE, dlgWidth * 9 / 15, dlgHeight / 2 + TOOLBAR_HEIGHT, 200, 20, hDlg, NULL, NULL, NULL);
		SendMessage(hAnnoTextE, WM_SETFONT, (WPARAM)hFont, true);
		hAnnoTextF = CreateWindowEx(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE, dlgWidth * 9 / 15, dlgHeight * 6 / 10 + TOOLBAR_HEIGHT, 200, 20, hDlg, NULL, NULL, NULL);
		SendMessage(hAnnoTextF, WM_SETFONT, (WPARAM)hFont, true);
		hAnnoTextG = CreateWindowEx(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE, dlgWidth * 9 / 15, dlgHeight * 7 / 10 + TOOLBAR_HEIGHT, 200, 20, hDlg, NULL, NULL, NULL);
		SendMessage(hAnnoTextG, WM_SETFONT, (WPARAM)hFont, true);

		//ShowWindow(static1, SW_HIDE);
		ShowWindow(hAnnoTextB, SW_HIDE);
		ShowWindow(hAnnoTextC, SW_HIDE);
		ShowWindow(hAnnoTextD, SW_HIDE);
		ShowWindow(hAnnoTextE, SW_HIDE);
		ShowWindow(hAnnoTextF, SW_HIDE);
		ShowWindow(hAnnoTextG, SW_HIDE);

		dlgTextBackground = CreateSolidBrush(COLOR_BACKGROUND);

		break;
		//return (INT_PTR)true;

	case WM_CTLCOLORSTATIC:
	{
							  HDC hdcStatic = (HDC)wParam;
							  DWORD CtrlID = GetDlgCtrlID((HWND)lParam); //Window Control ID

							  if ((HWND)lParam == hToolbarText || (HWND)lParam == g_hToolbarText)
							  {
								  SetTextColor(hdcStatic, COLOR_WHITE);
								  SetBkColor(hdcStatic, COLOR_TOOLBAR);
							  }
							  else
							  {
								  SetTextColor(hdcStatic, COLOR_BLACK);
								  SetBkColor(hdcStatic, COLOR_WHITE);
							  }


							  return (BOOL)GetSysColorBrush(COLOR_WINDOW);
	}
		break;
	case WM_CTLCOLORDLG:
		return (INT_PTR)dlgTextBackground;
		break;
	case WM_COMMAND:
	{
					   int id = LOWORD(wParam);
					   switch (id)
					   {
					   case IDOK:
						   EndDialog(hDlg, false);
						   return (INT_PTR)true;
					   case IDCANCEL:
						   EndDialog(hDlg, false);
						   return (INT_PTR)true;
					   default:
						   EndDialog(hDlg, false);
						   return (INT_PTR)true;
						   break;
					   }
	}
		break;


	case WM_PAINT:
	{
					 hdc = BeginPaint(hDlg, &ps);

					 //Draw elevation
					 fillRectangle(hdc, 30, 0, dlgWidth - 20, dlgHeight - 20, COLOR_WHITE);

					 //Draw toolbar
					 fillRectangle(hdc, 0, 0, dlgWidth, TOOLBAR_HEIGHT, COLOR_TOOLBAR);

					 if (gAppNameList.size() > 0)
					 {
						 DWORD dwRadius = 100; //Pie chart radius
						 float startAngle = 90;
						 float sweepAngle = 0;

						 int nX = dlgWidth / 4;
						 int nY = dlgHeight / 3 + TOOLBAR_HEIGHT;

						 int sum = 0;
						 vector<Program> sortedList;

						 hdc = GetDC(hDlg);
						 BeginPath(hdc);

						 //Calculate total program in list
						 for (int i = 0; i < gAppNameList.size(); i++)
						 {
							 sum += gAppNameList[i].frequency;
						 }

						 
						 //Push the index of the list in the sorted list
						 for (int i = 0; i < gAppNameList.size(); i++)
						 {
							 sortedList.push_back(gAppNameList[i]);
						 }

						 if (gAppNameList.size())
						 {
							 //Sort order
							 for (int i = 0; i < sortedList.size() - 1; i++)
							 {
								 for (int j = i + 1; j < sortedList.size(); j++)
								 {
									 if (sortedList[i].frequency < sortedList[j].frequency)
									 {
										 Program temp = sortedList[i];
										 sortedList[i] = sortedList[j];
										 sortedList[j] = temp;
									 }
								 }
							 }
						 }

						 //start to paint				
						 SelectObject(hdc, GetStockObject(DC_BRUSH));

						 //Draw the pie chart
						 for (int i = 0; i < sortedList.size(); i++)
						 {
							 if (i < 6 && sortedList[i].frequency>0)
							 {
								 //calculate sweep angle
								 sweepAngle = 360 * sortedList[i].frequency / sum;
								 BeginPath(hdc);
								 if (i == 0)
								 {
									 if (i == sortedList.size() - 1)
									 {
										 sweepAngle = 270 + startAngle;
									 }

									 SetDCBrushColor(hdc, colorList[i]);
									 setChartAnnotation(hAnnoTextA, i, sortedList, sum);
								 }
								 if (i == 1)
								 {
									 if (i == sortedList.size() - 1)
									 {
										 sweepAngle = 270 + startAngle;
									 }
									 SetDCBrushColor(hdc, colorList[i]);
									 setChartAnnotation(hAnnoTextB, i, sortedList, sum);
								 }
								 if (i == 2)
								 {
									 if (i == sortedList.size() - 1)
									 {
										 sweepAngle = 270 + startAngle;
									 }
									 SetDCBrushColor(hdc, colorList[i]);
									 setChartAnnotation(hAnnoTextC, i, sortedList, sum);
								 }
								 if (i == 3)
								 {
									 if (i == sortedList.size() - 1)
									 {
										 sweepAngle = 270 + startAngle;
									 }
									 SetDCBrushColor(hdc, colorList[i]);
									 setChartAnnotation(hAnnoTextD, i, sortedList, sum);
								 }
								 if (i == 4)
								 {
									 if (i == sortedList.size() - 1)
									 {
										 sweepAngle = 270 + startAngle;
									 }
									 SetDCBrushColor(hdc, colorList[i]);
									 setChartAnnotation(hAnnoTextE, i, sortedList, sum);
								 }
								 if (i == 5)
								 {
									 if (i == sortedList.size() - 1)
									 {
										 sweepAngle = 270 + startAngle;
									 }
									 SetDCBrushColor(hdc, colorList[i]);
									 setChartAnnotation(hAnnoTextF, i, sortedList, sum);
								 }
							 }
							 else
							 {
								 if (sortedList[i].frequency < 1 && i < 6)
								 {
									 break;
								 }

								 //Last angle  = 360 - all others
								 sweepAngle = 270 + startAngle;

								 BeginPath(hdc);
								 SetDCBrushColor(hdc, colorList[6]);
								 Draw(hDlg, hdc, nX, nY, dwRadius, startAngle, -sweepAngle);

								 //Show chart annotations
								 fillRectangle(hdc, dlgWidth / 2, dlgHeight * (i + 1) / 10 + TOOLBAR_HEIGHT, dlgWidth * 4 / 7, dlgHeight * (i + 1) / 10 + dlgHeight / 20 + TOOLBAR_HEIGHT, colorList[6]);
								 ShowWindow(hAnnoTextG, SW_SHOWNOACTIVATE);
								 wstring temp = to_wstring((int)(100 * sweepAngle / 360)) + L"\%: " + L"Others: ";
								 SetWindowText(hAnnoTextG, temp.c_str());
								 break;

							 }

							 Draw(hDlg, hdc, nX, nY, dwRadius, startAngle, -sweepAngle);
							 fillRectangle(hdc, dlgWidth / 2, dlgHeight * (i + 1) / 10 + TOOLBAR_HEIGHT, dlgWidth * 4 / 7, dlgHeight * (i + 1) / 10 + dlgHeight / 20 + TOOLBAR_HEIGHT, colorList[i]);

							 //Move the line between core and bound base on angle
							 startAngle -= sweepAngle;
						 }

					 }

	}
		break;
	}
	return (INT_PTR)false;
}

void Draw(HWND hDlg, HDC hdc, int nX, int nY, DWORD dwRadius, float fStartAngle, float fSweepAngle){
	MoveToEx(hdc, nX, nY, (LPPOINT)NULL);
	AngleArc(hdc, nX, nY, dwRadius, fStartAngle, fSweepAngle);
	LineTo(hdc, nX, nY);
	EndPath(hdc);
	FillPath(hdc);
	//ReleaseDC(hDlg, hdc);
}
 
INT_PTR CALLBACK ScanProgramDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam){
	GetClientRect(hDlg, &rcClient);
	int width = rcClient.right - rcClient.left;
	int height = rcClient.bottom - rcClient.top;
	HFONT hFont = CreateFont(17, 7, 0, 0, FW_EXTRALIGHT, true, false, false, ANSI_CHARSET,
		OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, TEXT("Times New Roman"));	GetClientRect(hDlg, &rcClient);
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
	{
						  InitCommonControls();

						  g_hSearchView = CreateWindowEx(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 10, 10, DEFAULT_WIDTH - 35, 20, hDlg, (HMENU)123, g_hInstance, NULL);
						  g_hListProgram = CreateListView(WS_EX_CLIENTEDGE, hDlg, IDL_LISTVIEW, g_hInstance, 10, 40, DEFAULT_WIDTH - 35, 180, LVS_REPORT | LVS_ICON | LVS_EDITLABELS | LVS_SHOWSELALWAYS);


						  //EnumInstalledSoftware();
						  RegistryEnumeration();

						  ScanProgramFileX86();

						  SortList(ASCENDING);

						  ReadProgramFrequency(FREQUENCY_FILE_PATH);

						  LoadDataToListView(gToShowList);
	}
		break;
	case WM_CTLCOLORSTATIC:
	{
							  DWORD CtrlID = GetDlgCtrlID((HWND)lParam); //Window Control ID
							  HDC hdcStatic = (HDC)wParam;
							  SetTextColor(hdcStatic, COLOR_BLACK);
							  SetBkColor(hdcStatic, COLOR_WHITE);
							  return (BOOL)GetSysColorBrush(COLOR_WINDOW);
	}
		break;
	case WM_COMMAND:
	{
					   int id = LOWORD(wParam);
					   switch (id)
					   {
					   case IDOK:
						   EndDialog(hDlg, false);
						   return (INT_PTR)true;
					   case IDCANCEL:
						   EndDialog(hDlg, false);
						   return (INT_PTR)true;
					   default:
						   EndDialog(hDlg, false);
						   return (INT_PTR)true;
						   break;
					   }
	}break;

	}
}

void CALLBACK TimeToShowStatitistic(HWND hwnd, UINT uMsg, UINT timerId, DWORD dwTime)
{
	remainTime--;
	if (remainTime == -1)
	{
		DialogBox(NULL, MAKEINTRESOURCE(IDD_STATISTIC),
			hwnd, ViewStatitisticDialog);
		remainTime = MAXTIME;
	}

	return;
}

bool KeyPressed(int a)
{
	if (GetAsyncKeyState(a))
	{
		return true;
	}
	else
	{
		return false;
	}
}

LRESULT CALLBACK MyHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{

	if (nCode < 0)
	{
		return CallNextHookEx(hHook, nCode, wParam, lParam);
	}

	if ((GetAsyncKeyState(VK_LWIN) < 0) && (GetAsyncKeyState('K') < 0))
	{
		if (isHiding)
		{
			ShowWindow(g_hWnd, SW_SHOWDEFAULT);
			isHiding = false;
		}
		else
		{
			isHiding = true;
			ShowWindow(g_hWnd, SW_MINIMIZE);

		}
	}

	return CallNextHookEx(hHook, nCode, wParam, lParam);

}

void _doInstallHook(HWND hWnd)
{
	if (hHook != NULL) return;
	hHook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)MyHookProc, hinstLib, 0);

}

void _doRemoveHook(HWND hWnd)
{
	if (hHook == NULL) return;
	UnhookWindowsHookEx(hHook);
	hHook = NULL;
	//MessageBox(hWnd, L"Remove hook successfully", L"Result", MB_OK);
}

void WriteProgramFrequency(wstring filePath)
{
	wofstream f(filePath);

	for (int i = 0; i < gAppNameList.size(); i++)
	{
		if (gAppNameList.at(i).frequency > 0)
		{
			f << gAppNameList.at(i).displayName << endl;
			f << gAppNameList.at(i).frequency << endl;
		}
	}

	//Close file
	f.close();

}

void ReadProgramFrequency(wstring filePath)
{
	wfstream f;
	f.open(filePath, ios::in);


	wstring buffer;
	if (f.is_open())
	{
		//Get items
		while (getline(f, buffer))
		{
			gFreqProgNameList.push_back(buffer);
			getline(f, buffer);
			gFreqProgramList.push_back(_wtoi64(buffer.c_str()));
		}
	}

	//Close file
	f.close();
}

void InsertFrequencyToDb()
{
	for (int i = 0; i < gAppNameList.size(); i++)
	{
		gAppNameList.at(i).index = i;
		for (int j = 0; j < gFreqProgNameList.size(); j++)
		{
			if (gAppNameList.at(i).displayName == gFreqProgNameList.at(j))
			{
				gAppNameList.at(i).frequency = gFreqProgramList.at(j);
				break;
			}
		}
	}
}

bool isExecutableFile(WCHAR file[])
{
	if (wcsstr(file, L".exe") != nullptr)
	{
		//Contain .exe
		return true;
	}

	return false;
}

void ScanProgramFileX86() 
{
	//Init find_data struct
	WIN32_FIND_DATA  wfd; // Contains information about the file that is found by Find first file and Find next file
	TCHAR* programFileRoot = new TCHAR[wcslen(PROGRAM_FILE_PATH_X86) + 2];
	StrCpy(programFileRoot, PROGRAM_FILE_PATH_X86);
	StrCat(programFileRoot, _T("\\*"));

	HANDLE hFile = FindFirstFileW(programFileRoot, &wfd);
	bool found = true;

	//If the function fails or fails to locate files from the search string
	if (hFile == INVALID_HANDLE_VALUE) {
		found = false;
	}
	while (found) 
	{
		TCHAR* applicationPath;
		if ((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) //Get only directory and folder
			&& (StrCmp(wfd.cFileName, _T(".")) != 0) && (StrCmp(wfd.cFileName, _T("..")) != 0)) 
		{
			applicationPath = new TCHAR[wcslen(PROGRAM_FILE_PATH_X86) + wcslen(wfd.cFileName) + 2];
			StrCpy(applicationPath, PROGRAM_FILE_PATH_X86);
			StrCat(applicationPath, L"\\");
			StrCat(applicationPath, wfd.cFileName);
			deepPathSearch(applicationPath, 0);
		}
		found = FindNextFileW(hFile, &wfd);
	}
}

void ScanProgramFile() 
{
	//Init find_data struct
	WIN32_FIND_DATA  wfd; //Contains information about the file that is found by Find first file and Find next file
	TCHAR* programFileRoot = new TCHAR[wcslen(PROGRAM_FILE_PATH) + 2];
	StrCpy(programFileRoot, PROGRAM_FILE_PATH);
	StrCat(programFileRoot, _T("\\*"));

	HANDLE hFile = FindFirstFileW(programFileRoot, &wfd);
	bool found = true;

	//If the function fails or fails to locate files from the search string
	if (hFile == INVALID_HANDLE_VALUE) {
		found = false;
	}

	while (found) 
	{
		TCHAR* applicationPath;
		if ((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) //Get only directory and folder
			&& (StrCmp(wfd.cFileName, _T(".")) != 0) && (StrCmp(wfd.cFileName, _T("..")) != 0)) 
		{
			applicationPath = new TCHAR[wcslen(PROGRAM_FILE_PATH) + wcslen(wfd.cFileName) + 2];
			StrCpy(applicationPath, PROGRAM_FILE_PATH);
			StrCat(applicationPath, L"\\");
			StrCat(applicationPath, wfd.cFileName);

			deepPathSearch(applicationPath, 0);
		}
		found = FindNextFileW(hFile, &wfd);
	}
}

void deepPathSearch(TCHAR* applicationPath, int depthLevel) 
{
	//Determine depth search level
	if (depthLevel >= 2)
		return;
	if (applicationPath == NULL || wcslen(applicationPath) == 0) {
		return;
	}

	TCHAR* appContent = new TCHAR[wcslen(applicationPath) + 2];
	StrCpy(appContent, applicationPath);
	StrCat(appContent, L"\\*");

	WIN32_FIND_DATA wfd; // Contains information about the file that is found by Find first file and Find next file
	HANDLE hFile = FindFirstFileW(appContent, &wfd);
	bool isFound = true;
	if (hFile == INVALID_HANDLE_VALUE) {
		isFound = false;
	}
	while (isFound) {
		if (depthLevel == 1) {
			if ((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) //Get only directory and folder
			{
				isFound = FindNextFileW(hFile, &wfd);
				continue;
			}
		}
		if ((StrCmp(wfd.cFileName, _T(".")) != 0) && (StrCmp(wfd.cFileName, _T("..")) != 0)) {
			if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) //Get only directory and folder
			{
				//Recursion to dive to next depth level
				TCHAR* childDir;
				childDir = new TCHAR[wcslen(applicationPath) + wcslen(wfd.cFileName) + 2];
				StrCpy(childDir, applicationPath);
				StrCat(childDir, L"\\");
				StrCat(childDir, wfd.cFileName);
				deepPathSearch(childDir, depthLevel + 1);

				//Make GC happy
				delete[] childDir;
			}
			else {
				if (isExecutableFile(wfd.cFileName)) 
				{
					//Gotcha!
					TCHAR* childFile;
					childFile = new TCHAR[wcslen(applicationPath) + wcslen(wfd.cFileName) + 2];
					StrCpy(childFile, applicationPath);
					StrCat(childFile, L"\\");
					StrCat(childFile, wfd.cFileName);

					//Save data
					Program p;
					p.installLocation = applicationPath;
					p.displayIcon = childFile;
					p.displayName = wfd.cFileName;
					p.displayVersion = L"";
					gAppNameList.push_back(p);
				}
			}
		}
		isFound = FindNextFileW(hFile, &wfd);
	}
}

void SortList(int mode)
{
	for (int i = 0; i < gAppNameList.size(); i++)
	{
		for (int j = i + 1; j < gAppNameList.size(); j++)
		{
			if (mode == ASCENDING)
			{
				if (gAppNameList.at(i).displayName.compare(gAppNameList.at(j).displayName) > 0)
				{
					Program temp = gAppNameList.at(i);
					gAppNameList.at(i) = gAppNameList.at(j);
					gAppNameList.at(j) = temp;
				}
			}
			else
			{
				if (gAppNameList.at(i).displayName.compare(gAppNameList.at(j).displayName) < 0)
				{
					Program temp = gAppNameList.at(i);
					gAppNameList.at(i) = gAppNameList.at(j);
					gAppNameList.at(j) = temp;
				}
			}
		}
	}
}

void SearchList(wstring keyword)
{
	gToShowList.clear();
	for (int i = 0; i < gAppNameList.size(); i++)
	{
		if (gAppNameList.at(i).displayName.find(keyword) != -1)
		{
			gToShowList.push_back(gAppNameList.at(i));
		}
	}
	/*
	else
	{
		for (int i = 0; i < gToShowList.size(); i++)
	{
		if (gToShowList.at(i).find(keyword) == -1)
		{
		gToShowList.erase(gToShowList.begin() + i);
		}
	}
	}
	*/
}

tstring QueryValueData(HKEY hKey,
	LPCTSTR szName)
{
	tstring value;

	DWORD dwType;
	DWORD dwSize = 0;

	if (::RegQueryValueEx(
		hKey,                   // key handle
		szName,                 // item name
		NULL,                   // reserved
		&dwType,                // type of data stored
		NULL,                   // no data buffer
		&dwSize                 // required buffer size
		) == ERROR_SUCCESS && dwSize > 0)
	{
		value.resize(dwSize);

		::RegQueryValueEx(
			hKey,                   // key handle
			szName,                 // item name
			NULL,                   // reserved
			&dwType,                // type of data stored
			(LPBYTE)&value[0],      // data buffer
			&dwSize                 // available buffer size
			);
	}

	return value;
}

void RegistryEnumeration()
{
	HKEY hKey;
	LONG ret = ::RegOpenKeyEx(
		HKEY_LOCAL_MACHINE,     // local machine hive
		_T("Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall"), // uninstall key
		0,                      // reserved
		KEY_READ,               // desired access
		&hKey                   // handle to the open key
		);

	if (ret != ERROR_SUCCESS)
		return;

	DWORD dwIndex = 0;
	DWORD cbName = 1024;
	TCHAR szSubKeyName[1024];

	while ((ret = ::RegEnumKeyEx(
		hKey,
		dwIndex,
		szSubKeyName,
		&cbName,
		NULL,
		NULL,
		NULL,
		NULL)) != ERROR_NO_MORE_ITEMS)
	{
		if (ret == ERROR_SUCCESS)
		{
			HKEY hItem;
			if (::RegOpenKeyEx(hKey, szSubKeyName, 0, KEY_READ, &hItem) != ERROR_SUCCESS)
				continue;

			tstring name = QueryValueData(hItem, _T("DisplayName"));
			tstring publisher = QueryValueData(hItem, _T("Publisher"));
			tstring version = QueryValueData(hItem, _T("DisplayVersion"));
			tstring location = QueryValueData(hItem, _T("InstallLocation"));
			tstring icon = QueryValueData(hItem, _T("DisplayIcon"));

			if (!name.empty())
			{
				Program p;
				p.displayIcon = icon;
				p.displayName = name;
				p.displayVersion = version;
				p.installLocation = location;
				gAppNameList.push_back(p);
			}

			::RegCloseKey(hItem);
		}
		dwIndex++;
		cbName = 1024;
	}
	::RegCloseKey(hKey);
}

void AddNotificationIcon(HWND hWnd)
{
	//add icon to notification area 
	ZeroMemory(&notifyIcon, sizeof(NOTIFYICONDATA));
	notifyIcon.cbSize = sizeof(NOTIFYICONDATA);
	notifyIcon.uID = IDI_ICON1;
	notifyIcon.hBalloonIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_ICON1));
	notifyIcon.hIcon = (HICON)LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_ICON1));
	notifyIcon.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	notifyIcon.hWnd = hWnd;
	notifyIcon.uCallbackMessage = WM_MYMESSAGE;
	wcscpy_s(notifyIcon.szTip, L"My QuickLaunch");
	Shell_NotifyIcon(NIM_MODIFY, &notifyIcon); //modify name in szTip
	Shell_NotifyIcon(NIM_ADD, &notifyIcon); //Add to notification area
}

void ScanDatabase()
{
	int state = SendMessage(g_hCheckbox, BM_GETCHECK, 0, 0);
	if (state == BST_CHECKED)
	{
		//Include
		RegistryEnumeration();
	}
	ScanProgramFile();
	ScanProgramFileX86();
	InsertFrequencyToDb();
}

void fillRectangle(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color)
{
	//Create rectangle
	RECT* rect = new RECT;
	rect->left = x1;
	rect->top = y1;
	rect->right = x2;
	rect->bottom = y2;

	//Create new brush
	HBRUSH hbrush = CreateSolidBrush(color);

	//Fill
	FillRect(hdc, rect, hbrush);
}

void RunProgram(int index)
{
	wstring exeString = gToShowList.at(index).displayIcon;
	if (!exeString.empty())
	{
		exeString = fixExecutablePath(exeString);
	}
	else
	{
		exeString = gToShowList.at(index).installLocation;
	}

	//Increase frequency
	gAppNameList.at(gToShowList.at(index).index).frequency++;

	ShellExecute(NULL, _T("open"), exeString.c_str(), NULL, NULL, SW_SHOWNORMAL);
}

void SetupViews(HWND hWnd, int wndWidth, int wndHeight)
{
	//Create font
	HFONT hFont;
	hFont = CreateFont(24, 0, 0, 0, FW_BOLD, false, false, false, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"San serif");

	//add icon to notification area 
	AddNotificationIcon(hWnd);

	g_hToolbarText = CreateWindowEx(0, L"STATIC", L"My QuickLaunch", WS_CHILD | WS_VISIBLE, 30, 10, 160, 23, hWnd, (HMENU)NULL, g_hInstance, NULL);
	SendMessage(g_hToolbarText, WM_SETFONT, WPARAM(hFont), true);

	hFont = CreateFont(16, 0, 0, 0, FW_SEMIBOLD, false, false, false, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

	g_hSearchView = CreateWindowEx(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 40, 40 + TOOLBAR_HEIGHT, wndWidth - 70, 30, hWnd, (HMENU)NULL, g_hInstance, NULL);
	g_hListProgram = CreateListView(WS_EX_CLIENTEDGE, hWnd, IDL_LISTVIEW, g_hInstance, 40, 80 + TOOLBAR_HEIGHT, wndWidth - 70, wndHeight - 110 - TOOLBAR_HEIGHT, LVS_REPORT | LVS_ICON | LVS_EDITLABELS | LVS_SHOWSELALWAYS);

	g_hStatic = CreateWindowEx(0, L"STATIC", L"Search Everywhere: ", WS_CHILD | WS_VISIBLE, 40, 10 + TOOLBAR_HEIGHT, wndWidth - 130, 30, hWnd, (HMENU)NULL, g_hInstance, NULL);
	SendMessage(g_hStatic, WM_SETFONT, WPARAM(hFont), true);

	g_hCheckbox = CreateWindowEx(0, L"BUTTON", L"Include installed program in Registry (HKLM)", BS_AUTOCHECKBOX | WS_CHILD | WS_VISIBLE, 190, 10 + TOOLBAR_HEIGHT, 310, 20, hWnd, (HMENU)NULL, g_hInstance, NULL);
	SendMessage(g_hCheckbox, WM_SETFONT, WPARAM(hFont), true);
	SendMessage(g_hCheckbox, BM_SETCHECK, BST_CHECKED, NULL);
}


