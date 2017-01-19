#pragma once

#include "resource.h"
#include "Program.h"
#include <stdio.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>

using namespace std;

#ifdef _UNICODE
#define tcout       wcout
#define tstring     wstring
#define WM_MYMESSAGE (WM_USER+ 1)
UINT EXIT = 100;
UINT SCAN = 200;
UINT VIEW_STATITISTIC = 300;
#else
#define tcout       cout
#define tstring     string
#endif

#define MAX_LOADSTRING 100
#define DEFAULT_WIDTH 550
#define DEFAULT_HEIGHT 620

#define ASCENDING 1
#define DESCENDING 0

#define COLOR_A			RGB(72,133,237)
#define COLOR_B			RGB(219,50,54)
#define COLOR_C			RGB(156,39,176)
#define COLOR_D			RGB(244,194,13)
#define COLOR_E			RGB(60,186,84)
#define COLOR_F			RGB(141,110,99)
#define COLOR_G			RGB(255, 128, 0)
#define COLOR_TOOLBAR	RGB(44, 144, 244)
#define COLOR_BACKGROUND	RGB(240, 240, 240)
#define COLOR_WHITE		RGB(255,255,255)
#define COLOR_BLACK		RGB(0, 0, 0)
#define DEFAULT_COLOR	RGB(255,255,255)

#define TOOLBAR_HEIGHT	45
#define MAXTIME    5
#define PROGRAM_FILE_PATH L"C:\\Program Files"
#define PROGRAM_FILE_PATH_X86 L"C:\\Program Files (x86)"
#define FREQUENCY_FILE_PATH L"program_frequency.txt"

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383


HWND CreateListView(long lExtStyle, HWND parentWnd, long ID, HINSTANCE hParentInst, int x, int y, int nWidth, int nHeight, long lStyle);
void Draw(HWND hDlg, HDC hdc, int nX, int nY, DWORD dwRadius, float xStartAngle, float xSweepAngle);
void LoadDataToListView(vector<Program> list);
void SetupViews(HWND hWnd, int wndWidth, int wndHeight);
void ClearData();
void WriteProgramFrequency(wstring filePath);
void ReadProgramFrequency(wstring filePath);
void InsertFrequencyToDb();
void ScanProgramFileX86();
void ScanProgramFile();
void SortList(int mode);
void SearchList(wstring keyword);
tstring QueryValueData(HKEY hKey, LPCTSTR szValueName);
void RegistryEnumeration();
void AddNotificationIcon(HWND hWnd);
void ScanDatabase();
void RunProgram(int index);

//Hepler
wstring fixExecutablePath(wstring path);
void deepPathSearch(TCHAR* appPath, int level);
void setChartAnnotation(HWND staticX, int i, vector<Program>sort, int sum);
void _doRemoveHook(HWND hWnd);
void _doInstallHook(HWND hWnd);
bool isExecutableFile(WCHAR file[]);
void fillRectangle(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color);
