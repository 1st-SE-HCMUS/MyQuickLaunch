// MyQuickLaunch.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "MyQuickLaunch.h"
#include <commctrl.h>
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib, "ComCtl32.lib")
//For StrCpy, StrNCat
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include <shellapi.h>
#include <vector>
#include <string>
#include <iostream>
#include <msi.h>
#include <Shlobj.h>
#pragma comment(lib, "Msi.lib")
#include <fstream>

#ifdef _UNICODE
#define tcout       wcout
#define tstring     wstring
#else
#define tcout       cout
#define tstring     string
#endif

using namespace std;

class Program
{
public:
	int index;
	wstring displayName;
	wstring displayIcon;
	wstring displayVersion;
	wstring installLocation;
	int frequency = 0;
};



#define MAX_LOADSTRING 100
#define DEFAULT_WIDTH 360
#define DEFAULT_HEIGHT 360

#define ASCENDING 1
#define DESCENDING 0

#define PROGRAM_FILE_PATH L"C:\\Program Files"
#define PROGRAM_FILE_PATH_X86 L"C:\\Program Files (x86)"
#define FREQUENCY_FILE_PATH L"program_frequency.txt"

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

// Global Variables:
HINSTANCE g_hInstance;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
HWND hInput;
HWND hListProgram;
vector<Program> gListAppName;
vector<Program> gToShowList;
vector<wstring> gFrequentProgramName;
vector<int> gFrequentProgram;


// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
HWND createListView(long lExtStyle, HWND parentWnd, long ID, HINSTANCE hParentInst, int x, int y, int nWidth, int nHeight, long lStyle);
bool EnumInstalledSoftware(void);
void QueryKey(HKEY hKey);
void loadDataToListView(vector<Program> list);
wstring fixExecutablePath(wstring path);
void ScanAppPath(TCHAR* appPath, int level);

void writeProgramFrequency(wstring filePath)
{
	wofstream f(filePath);

	for (int i = 0; i < gListAppName.size(); i++)
	{
		if (gListAppName.at(i).frequency > 0)
		{
			f << gListAppName.at(i).displayName << endl;
			f << gListAppName.at(i).frequency << endl;
		}
	}

	//Close file
	f.close();

}

void readProgramFrequency(wstring filePath)
{
	wfstream f;
	f.open(filePath, ios::in);


	wstring buffer;
	if (f.is_open())
	{
		//Get items
		while (getline(f, buffer))
		{
			gFrequentProgramName.push_back(buffer);
			getline(f, buffer);
			gFrequentProgram.push_back(_wtoi64(buffer.c_str()));
		}
	}

	//Close file
	f.close();

	for (int i = 0; i < gListAppName.size(); i++)
	{
		gListAppName.at(i).index = i;
		for (int j = 0; j < gFrequentProgramName.size(); j++)
		{
			if (gListAppName.at(i).displayName == gFrequentProgramName.at(j))
			{
				gListAppName.at(i).frequency = gFrequentProgram.at(j);
				break;
			}
		}
	}
}

bool IsExefile(WCHAR file[])
{
	if (wcsstr(file, L".exe") != nullptr)
	{
		//Contain .exe
		return true;
	}

	return false;
}

void Scanx86Database() {
	WIN32_FIND_DATA  fd;
	TCHAR* programPathx86Content = new TCHAR[wcslen(PROGRAM_FILE_PATH_X86) + 2];
	StrCpy(programPathx86Content, PROGRAM_FILE_PATH_X86);
	StrCat(programPathx86Content, _T("\\*"));
	HANDLE hFile = FindFirstFileW(programPathx86Content, &fd);
	BOOL isFound = TRUE;
	if (hFile == INVALID_HANDLE_VALUE) {
		isFound = FALSE;
		OutputDebugString(L"Can't found x86: ");
		OutputDebugString(PROGRAM_FILE_PATH_X86);
		OutputDebugString(L"\n");
	}
	while (isFound) {
		TCHAR* appPath;
		if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			&& (StrCmp(fd.cFileName, _T(".")) != 0) && (StrCmp(fd.cFileName, _T("..")) != 0)
			) {
			appPath = new TCHAR[wcslen(PROGRAM_FILE_PATH_X86) + wcslen(fd.cFileName) + 2];
			StrCpy(appPath, PROGRAM_FILE_PATH_X86);
			StrCat(appPath, L"\\");
			StrCat(appPath, fd.cFileName);
			OutputDebugString(L"Scan app path x86: ");
			OutputDebugString(appPath);
			OutputDebugString(L"\n");
			ScanAppPath(appPath, 0);
		}
		isFound = FindNextFileW(hFile, &fd);
	}
}
void Scanx64Database() {
	WIN32_FIND_DATA  fd;
	TCHAR* programPathContent = new TCHAR[wcslen(PROGRAM_FILE_PATH) + 2];
	StrCpy(programPathContent, PROGRAM_FILE_PATH);
	StrCat(programPathContent, _T("\\*"));

	HANDLE hFile = FindFirstFileW(programPathContent, &fd);
	BOOL isFound = TRUE;
	if (hFile == INVALID_HANDLE_VALUE) {
		isFound = FALSE;
		OutputDebugString(L"Can't found x64 : ");
		OutputDebugString(PROGRAM_FILE_PATH);
		OutputDebugString(L"\n");
	}
	while (isFound) {
		TCHAR* appPath;
		if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			&& (StrCmp(fd.cFileName, _T(".")) != 0) && (StrCmp(fd.cFileName, _T("..")) != 0)
			) {
			appPath = new TCHAR[wcslen(PROGRAM_FILE_PATH) + wcslen(fd.cFileName) + 2];
			StrCpy(appPath, PROGRAM_FILE_PATH);
			StrCat(appPath, L"\\");
			StrCat(appPath, fd.cFileName);
			OutputDebugString(L"Scan app path x64: ");
			OutputDebugString(appPath);
			OutputDebugString(L"\n");
			ScanAppPath(appPath, 0);
		}
		isFound = FindNextFileW(hFile, &fd);
	}
}
void ScanAppPath(TCHAR* appPath, int level) {
	if (level >= 2)
		return;
	if (appPath == NULL || wcslen(appPath) == 0) {
		return;
	}
	TCHAR* appPathContent = new TCHAR[wcslen(appPath) + 2];
	StrCpy(appPathContent, appPath);
	StrCat(appPathContent, L"\\*");
	WIN32_FIND_DATA fd;
	HANDLE hFile = FindFirstFileW(appPathContent, &fd);
	BOOL isFound = TRUE;
	if (hFile == INVALID_HANDLE_VALUE) {
		isFound = FALSE;
		OutputDebugString(L"Can't found when scan app: ");
		OutputDebugString(appPath);
		OutputDebugString(L"\n");
	}
	while (isFound) {
		if (level == 1) {
			if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				isFound = FindNextFileW(hFile, &fd);
				continue;
			}
		}
		if ((StrCmp(fd.cFileName, _T(".")) != 0) && (StrCmp(fd.cFileName, _T("..")) != 0)) {
			if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				// is directory -> recur
				TCHAR* childDirectory;
				childDirectory = new TCHAR[wcslen(appPath) + wcslen(fd.cFileName) + 2];
				StrCpy(childDirectory, appPath);
				StrCat(childDirectory, L"\\");
				StrCat(childDirectory, fd.cFileName);
				ScanAppPath(childDirectory, level + 1);
				OutputDebugString(L"Scan child: ");
				OutputDebugString(childDirectory);
				OutputDebugString(L"\n");
				delete[] childDirectory;
			}
			else {
				if (IsExefile(fd.cFileName)) {
					TCHAR* childFile;
					childFile = new TCHAR[wcslen(appPath) + wcslen(fd.cFileName) + 2];
					StrCpy(childFile, appPath);
					StrCat(childFile, L"\\");
					StrCat(childFile, fd.cFileName);

					Program p;
					p.installLocation = appPath;
					p.displayIcon = childFile;
					p.displayName = fd.cFileName;
					p.displayVersion = L"";
					gListAppName.push_back(p);

					OutputDebugString(L"\t\t File: ");
					OutputDebugString(fd.cFileName);
					OutputDebugString(L"\n");
					// Save config ini
					//WritePrivateProfileString(L"app", fd.cFileName, childFile, dataPath);
				}
			}
		}
		isFound = FindNextFileW(hFile, &fd);
	}
}

void sortList(int mode)
{
	for (int i = 0; i < gListAppName.size(); i++)
	{
		for (int j = i + 1; j < gListAppName.size(); j++)
		{
			if (mode == ASCENDING)
			{
				if (gListAppName.at(i).displayName.compare(gListAppName.at(j).displayName) > 0)
				{
					Program temp = gListAppName.at(i);
					gListAppName.at(i) = gListAppName.at(j);
					gListAppName.at(j) = temp;
				}
			}
			else
			{
				if (gListAppName.at(i).displayName.compare(gListAppName.at(j).displayName) < 0)
				{
					Program temp = gListAppName.at(i);
					gListAppName.at(i) = gListAppName.at(j);
					gListAppName.at(j) = temp;
				}
			}
		}
	}
}

void searchList(wstring keyword)
{
	gToShowList.clear();
		for (int i = 0; i < gListAppName.size(); i++)
		{
			if (gListAppName.at(i).displayName.find(keyword) != -1)
			{
				gToShowList.push_back(gListAppName.at(i));
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

tstring MsiQueryProperty(LPCTSTR szProductCode,
	LPCTSTR szUserSid,
	MSIINSTALLCONTEXT dwContext,
	LPCTSTR szProperty)
{
	tstring value;

	DWORD cchValue = 0;
	UINT ret2 = ::MsiGetProductInfoEx(
		szProductCode,
		szUserSid,
		dwContext,
		szProperty,
		NULL,
		&cchValue);

	if (ret2 == ERROR_SUCCESS)
	{
		cchValue++;
		value.resize(cchValue);

		ret2 = ::MsiGetProductInfoEx(
			szProductCode,
			szUserSid,
			dwContext,
			szProperty,
			(LPTSTR)&value[0],
			&cchValue);
	}

	return value;
}

void MsiEnum()
{
	UINT ret = 0;
	DWORD dwIndex = 0;
	TCHAR szInstalledProductCode[39] = { 0 };
	TCHAR szSid[128] = { 0 };
	DWORD cchSid;
	MSIINSTALLCONTEXT dwInstalledContext;

	do
	{
		memset(szInstalledProductCode, 0, sizeof(szInstalledProductCode));
		cchSid = sizeof(szSid) / sizeof(szSid[0]);

		ret = ::MsiEnumProductsEx(
			NULL,           // all the products in the context
			_T("s-1-1-0"),  // i.e.Everyone, all users in the system
			MSIINSTALLCONTEXT_USERMANAGED | MSIINSTALLCONTEXT_USERUNMANAGED | MSIINSTALLCONTEXT_MACHINE,
			dwIndex,
			szInstalledProductCode,
			&dwInstalledContext,
			szSid,
			&cchSid);

		if (ret != ERROR_SUCCESS)
		{
			tstring name = MsiQueryProperty(
				szInstalledProductCode,
				cchSid == 0 ? NULL : szSid,
				dwInstalledContext,
				INSTALLPROPERTY_INSTALLEDPRODUCTNAME);

			tstring publisher = MsiQueryProperty(
				szInstalledProductCode,
				cchSid == 0 ? NULL : szSid,
				dwInstalledContext,
				INSTALLPROPERTY_PUBLISHER);

			tstring version = MsiQueryProperty(
				szInstalledProductCode,
				cchSid == 0 ? NULL : szSid,
				dwInstalledContext,
				INSTALLPROPERTY_VERSIONSTRING);

			tstring location = MsiQueryProperty(
				szInstalledProductCode,
				cchSid == 0 ? NULL : szSid,
				dwInstalledContext,
				INSTALLPROPERTY_INSTALLLOCATION);

			tcout << name << endl;
			tcout << "  - " << publisher << endl;
			tcout << "  - " << version << endl;
			tcout << "  - " << location << endl;
			tcout << endl;

			dwIndex++;
		}

	} while (ret == ERROR_SUCCESS);
}


tstring RegistryQueryValue(HKEY hKey,
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

void RegistryEnum()
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

			tstring name = RegistryQueryValue(hItem, _T("DisplayName"));
			tstring publisher = RegistryQueryValue(hItem, _T("Publisher"));
			tstring version = RegistryQueryValue(hItem, _T("DisplayVersion"));
			tstring location = RegistryQueryValue(hItem, _T("InstallLocation"));
			tstring icon = RegistryQueryValue(hItem, _T("DisplayIcon"));

			if (!name.empty())
			{
				Program p;
				p.displayIcon = icon;
				p.displayName = name;
				p.displayVersion = version;
				p.installLocation = location;
				gListAppName.push_back(p);
			}

			::RegCloseKey(hItem);
		}
		dwIndex++;
		cbName = 1024;
	}
	::RegCloseKey(hKey);
}



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
		return FALSE;
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
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
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

   hWnd = CreateWindowEx(WS_EX_PALETTEWINDOW, szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
	   CW_USEDEFAULT, CW_USEDEFAULT, DEFAULT_WIDTH, DEFAULT_HEIGHT, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
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

	switch (message)
	{
	case WM_CREATE:
	{
					  InitCommonControls();

					  hInput = CreateWindowEx(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 10, 10, DEFAULT_WIDTH - 35, 20, hWnd, (HMENU)123, g_hInstance, NULL);
					  hListProgram = createListView(WS_EX_CLIENTEDGE, hWnd, IDL_LISTVIEW, g_hInstance, 10, 40, DEFAULT_WIDTH - 35, 180, LVS_REPORT | LVS_ICON | LVS_EDITLABELS | LVS_SHOWSELALWAYS);


					  //EnumInstalledSoftware();
					  RegistryEnum();

					  Scanx86Database();

					  sortList(ASCENDING);

					  readProgramFrequency(FREQUENCY_FILE_PATH);

					  loadDataToListView(gToShowList);

					 
	}
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:

		if ((HWND)lParam == hInput) {
			if (wmEvent == EN_CHANGE ) {
				//triggered on keypress
				WCHAR keyword[255];
				GetWindowText(hInput, keyword, 255);
				searchList(keyword);
				loadDataToListView(gToShowList);
			}
		}

		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
			/*
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
			*/
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
						  if (notifyMess->hwndFrom == hListProgram)
						  {
							  
							  int i = ListView_GetSelectionMark(hListProgram);

							  wstring exeString = gToShowList.at(i).displayIcon;
							  if (!exeString.empty())
							  {
								  exeString = fixExecutablePath(exeString);
							  }
							  else
							  {
								  exeString = gToShowList.at(i).installLocation;
							  }

							  //Increase frequency
							  gListAppName.at(gToShowList.at(i).index).frequency++;

							 ShellExecute(NULL, _T("open"), exeString.c_str(), NULL, NULL, SW_SHOWNORMAL);
						  }
							  
						  break;
					  }
	}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		writeProgramFrequency(FREQUENCY_FILE_PATH);
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
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}


HWND createListView(long lExtStyle, HWND parentWnd, long ID, HINSTANCE hParentInst, int x, int y, int nWidth, int nHeight, long lStyle)
{
	//Create
	HWND m_hListView = CreateWindowEx(lExtStyle, WC_LISTVIEW, _T("List View"),
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP | lStyle,
		x, y, nWidth, nHeight, parentWnd, (HMENU)ID, hParentInst, NULL);


	//Init 5 columns
	LVCOLUMN lvCol;

	//Let the LVCOLUMN know that we will set the format, header text and width of it
	lvCol.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
	lvCol.fmt = LVCFMT_LEFT;

	lvCol.cx = 150;
	lvCol.pszText = _T("Name");
	ListView_InsertColumn(m_hListView, 0, &lvCol);

	lvCol.fmt = LVCFMT_LEFT;
	lvCol.pszText = _T("Type");
	lvCol.cx = 125;
	ListView_InsertColumn(m_hListView, 1, &lvCol);

	return m_hListView;
}


bool EnumInstalledSoftware(void)
{
	HKEY hUninstKey = NULL;
	HKEY hAppKey = NULL;
	WCHAR sAppKeyName[1024];
	WCHAR sSubKey[1024];
	WCHAR sDisplayName[1024];
	WCHAR *sRoot = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall");
	long lResult = ERROR_SUCCESS;
	DWORD dwType = KEY_ALL_ACCESS;
	DWORD dwBufferSize = 0;

	//Open the "Uninstall" key.
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, sRoot, 0, KEY_READ, &hUninstKey) != ERROR_SUCCESS)
	{
		return false;
	}

	for (DWORD dwIndex = 0; lResult == ERROR_SUCCESS; dwIndex++)
	{
		//Enumerate all sub keys...
		dwBufferSize = sizeof(sAppKeyName);
		if ((lResult = RegEnumKeyEx(hUninstKey, dwIndex, sAppKeyName,
			&dwBufferSize, NULL, NULL, NULL, NULL)) == ERROR_SUCCESS)
		{
			//Open the sub key.
			wsprintf(sSubKey, L"%s\\%s", sRoot, sAppKeyName);
			if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, sSubKey, 0, KEY_READ, &hAppKey) != ERROR_SUCCESS)
			{
				RegCloseKey(hAppKey);
				RegCloseKey(hUninstKey);
				return false;
			}

			//Get the display name value from the application's sub key.
			dwBufferSize = sizeof(sDisplayName);
			if (RegQueryValueEx(hAppKey, L"DisplayName", NULL,
				&dwType, (unsigned char*)sDisplayName, &dwBufferSize) == ERROR_SUCCESS)
			{
				wprintf(L"%s\n", sDisplayName);
				
			}
			else{
				//Display name value doe not exist, this application was probably uninstalled.
			}

			//gListAppName.push_back(wstring(sAppKeyName));
			RegCloseKey(hAppKey);
		}
	}

	RegCloseKey(hUninstKey);

	return true;
}




void QueryKey(HKEY hKey)
{
	TCHAR    achKey[MAX_KEY_LENGTH];   // buffer for subkey name
	DWORD    cbName;                   // size of name string 
	TCHAR    achClass[MAX_PATH] = TEXT("");  // buffer for class name 
	DWORD    cchClassName = MAX_PATH;  // size of class string 
	DWORD    cSubKeys = 0;               // number of subkeys 
	DWORD    cbMaxSubKey;              // longest subkey size 
	DWORD    cchMaxClass;              // longest class string 
	DWORD    cValues;              // number of values for key 
	DWORD    cchMaxValue;          // longest value name 
	DWORD    cbMaxValueData;       // longest value data 
	DWORD    cbSecurityDescriptor; // size of security descriptor 
	FILETIME ftLastWriteTime;      // last write time 

	DWORD i, retCode;

	TCHAR  achValue[MAX_VALUE_NAME];
	DWORD cchValue = MAX_VALUE_NAME;

	// Get the class name and the value count. 
	retCode = RegQueryInfoKey(
		hKey,                    // key handle 
		achClass,                // buffer for class name 
		&cchClassName,           // size of class string 
		NULL,                    // reserved 
		&cSubKeys,               // number of subkeys 
		&cbMaxSubKey,            // longest subkey size 
		&cchMaxClass,            // longest class string 
		&cValues,                // number of values for this key 
		&cchMaxValue,            // longest value name 
		&cbMaxValueData,         // longest value data 
		&cbSecurityDescriptor,   // security descriptor 
		&ftLastWriteTime);       // last write time 

	// Enumerate the subkeys, until RegEnumKeyEx fails.

	if (cSubKeys)
	{
		printf("\nNumber of subkeys: %d\n", cSubKeys);

		for (i = 0; i < cSubKeys; i++)
		{
			cbName = MAX_KEY_LENGTH;
			retCode = RegEnumKeyEx(hKey, i,
				achKey,
				&cbName,
				NULL,
				NULL,
				NULL,
				&ftLastWriteTime);
			if (retCode == ERROR_SUCCESS)
			{
				_tprintf(TEXT("(%d) %s\n"), i + 1, achKey);
			}

			Program p;
			p.displayName = wstring(achKey);
			
			gListAppName.push_back(p);
		}
	}

	// Enumerate the key values. 

	if (cValues)
	{
		printf("\nNumber of values: %d\n", cValues);

		for (i = 0, retCode = ERROR_SUCCESS; i < cValues; i++)
		{
			cchValue = MAX_VALUE_NAME;
			achValue[0] = '\0';
			retCode = RegEnumValue(hKey, i,
				achValue,
				&cchValue,
				NULL,
				NULL,
				NULL,
				NULL);

			if (retCode == ERROR_SUCCESS)
			{
				_tprintf(TEXT("(%d) %s\n"), i + 1, achValue);
			}
		}
	}
}

void loadDataToListView(vector<Program> list)
{
	ListView_DeleteAllItems(hListProgram);
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
		ListView_InsertItem(hListProgram, &lv);
	}
}

wstring fixExecutablePath(wstring path)
{
	int index = path.find(L"exe");
	return path.substr(0, index + 3);
}