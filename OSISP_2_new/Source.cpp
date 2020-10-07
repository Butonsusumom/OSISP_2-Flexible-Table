#include <windows.h>
#include <math.h>
#include <string>
#include <time.h>
#include <fstream>

using namespace std;

#define SCROLL_SPEED 30
#define MAX_ROWS_COUNT 30
#define MAX_COLUMNS_COUNT 30
#define MAX_LETTER_HEIGHT 35
#define MIN_LETTER_HEIGHT 10
#define MAX_LETTER_WIDTH 20
#define MIN_LETTER_WIDTH 3
#define MIN_LINE_SPACING 3
#define MAX_LINE_SPACING 30

#define DELTA 3

#define BOLD_MENU_ID 11
#define COURSIVE_MENU_ID 12
#define UNDERLINE_MENU_ID 13
#define COLOUR_MENU_ID 2
#define TIMES_ID 31
#define ARIAL_ID 32
#define IMPACT_ID 33

int rows = 10, columns = 5;
int top = 0, bottom = 0;
int letterWidth = 6, letterHeight = 20;
int textSpacing = 0; 
int delta = 0;
int TableHeight;
int linesSpacing = 10, letterAngle = 0;
float epsilon = 0;
bool isBold = false;
bool isCoursive = false;
bool isUnderline = false;
string **stringMatrix;
ifstream fileName("text.txt");
HMENU menu;
COLORREF rgbCurrent;
RECT clientRect;


void Initialization(HDC hdc);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void DrawTable(HDC hdc, int sx, int sy, int borderSize);
bool ChangeChked(HWND hWnd, bool parametr, RECT window, int MenuId);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSEX wcex;
	HWND hWnd;
	MSG msg;


	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_DBLCLKS;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = NULL;
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = 0;
	wcex.lpszClassName = "TableBuilderClass";
	wcex.hIconSm = wcex.hIcon;
	RegisterClassEx(&wcex);

	RegisterClassEx(&wcex);


	hWnd = CreateWindow("TableBuilderClass", "Flexible Table",
		WS_OVERLAPPEDWINDOW | WS_VSCROLL, CW_USEDEFAULT, 0,
		CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
	Initialization(GetDC(hWnd));

	menu = CreateMenu();
	HMENU hStyle = CreateMenu();
	HMENU hFont = CreateMenu();
	AppendMenu(menu, MF_POPUP, (UINT_PTR)hStyle, "Style");
	AppendMenu(menu, MF_POPUP, COLOUR_MENU_ID, "Colour");
	AppendMenu(menu, MF_SEPARATOR, NULL, NULL);

	AppendMenu(hStyle, MF_BYCOMMAND | MF_UNCHECKED, BOLD_MENU_ID, "BOLD");
	AppendMenu(hStyle, MF_BYCOMMAND | MF_UNCHECKED, UNDERLINE_MENU_ID, "UNDERLINE");
	AppendMenu(hStyle, MF_BYCOMMAND | MF_UNCHECKED, COURSIVE_MENU_ID, "COURSIVE");

	SetMenu(hWnd, menu);


	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static CHOOSECOLOR cc;
	static COLORREF acrCustClr[16];

	HDC hdc;
	HDC memDC;
	HBITMAP memBM;
	PAINTSTRUCT ps;
	HANDLE hOld;
	int step = 0;
	int wheelDelta;
	static int width = 0, height = 0;
	RECT window = {};


	int currentPos;
	SCROLLINFO scrInfo;

	switch (message)
	{
	case WM_CREATE:
		ZeroMemory(&cc, sizeof(cc));
		cc.lStructSize = sizeof(cc);
		cc.hwndOwner = hWnd;
		cc.lpCustColors = (LPDWORD)acrCustClr;
		cc.rgbResult = rgbCurrent;
		cc.Flags = CC_FULLOPEN | CC_RGBINIT;

		break;
	case WM_COMMAND:
		GetWindowRect(hWnd, &window);
		switch (LOWORD(wParam))
		{
		case BOLD_MENU_ID:
			isBold = ChangeChked(hWnd, isBold, window, BOLD_MENU_ID);
			break;
		case COURSIVE_MENU_ID:
			isCoursive = ChangeChked(hWnd, isCoursive, window, COURSIVE_MENU_ID);
			break;
		case UNDERLINE_MENU_ID:
			isUnderline = ChangeChked(hWnd, isUnderline, window, UNDERLINE_MENU_ID);
			break;
		case COLOUR_MENU_ID:
			cc.rgbResult = rgbCurrent;
			if (ChooseColor(&cc) == TRUE) {
				rgbCurrent = (COLORREF)cc.rgbResult;
			}
			//UpdateWindow(hWnd);
			InvalidateRect(hWnd, NULL, TRUE);
			break;
		}
		break;
	case WM_SIZE:
		width = LOWORD(lParam);
		height = HIWORD(lParam);
		//ScrollWindow(hWnd, 0, -top, NULL, NULL);
		//UpdateWindow(hWnd);
		top = 0;
		InvalidateRect(hWnd, NULL, TRUE);


		scrInfo.cbSize = sizeof(SCROLLINFO);

		scrInfo.nPage = HIWORD(lParam); //размер страницы устанавливаем равным высоте окна
		scrInfo.nMin = 0; //диапазон прокрутки устанавливаем по размеру содержимого
		scrInfo.nMax = TableHeight;
		scrInfo.fMask = SIF_RANGE | SIF_PAGE; //применяем новые параметры
		SetScrollInfo(hWnd, SB_VERT, &scrInfo, TRUE);
		break;
	case WM_MOUSEWHEEL:
		wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		if (wheelDelta > 0)
			SendMessage(hWnd, WM_VSCROLL, SB_LINEUP, NULL);
		if (wheelDelta < 0)
			SendMessage(hWnd, WM_VSCROLL, SB_LINEDOWN, NULL);
		break;
	case WM_VSCROLL:
		
		scrInfo.cbSize = sizeof(SCROLLINFO);

		scrInfo.fMask = SIF_ALL; //получаем текущие параметры scrollbar-а
		GetScrollInfo(hWnd, SB_VERT, &scrInfo);

		currentPos = scrInfo.nPos; //запоминаем текущее положение содержимого

		switch (LOWORD(wParam)) { //определяем действие пользователя и изменяем положение
		case SB_LINEUP: //клик на стрелку вверх
			scrInfo.nPos -= SCROLL_SPEED;
			step = SCROLL_SPEED;
			break;
		case SB_LINEDOWN: //клик на стрелку вниз 
			scrInfo.nPos += SCROLL_SPEED;
			step = -SCROLL_SPEED;
			break;
		case SB_THUMBTRACK: //перетаскивание ползунка
			scrInfo.nPos = scrInfo.nTrackPos;
			step = currentPos - scrInfo.nPos;
			break;
		default: return 0; //все прочие действия (например нажатие PageUp/PageDown) игнорируем
		}

		scrInfo.fMask = SIF_POS; //пробуем применить новое положение
		SetScrollInfo(hWnd, SB_VERT, &scrInfo, TRUE);
		GetScrollInfo(hWnd, SB_VERT, &scrInfo);

		top += step;
		if (((top != SCROLL_SPEED) && (step > 0)) || ((step < 0) && (bottom >= height)))
		{
			ScrollWindow(hWnd, 0, step, NULL, NULL);
			UpdateWindow(hWnd);
		}
		else
			top -= step;

		break;
	case WM_PAINT:
		//GetClientRect(hWnd, &clientRect);
		hdc = BeginPaint(hWnd, &ps); /*Создаётся hdc*/
		SetTextColor(hdc, rgbCurrent);
		DrawTable(hdc, width, height, 3);/*Функция, в которой всё рисуется в memDC*/
		EndPaint(hWnd, &ps); /*hdc удаляется*/
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

bool ChangeChked(HWND hWnd, bool parametr, RECT window, int MenuId) {
	parametr = !parametr;
	CheckMenuItem(menu, MenuId, MF_BYCOMMAND | (parametr ? MF_CHECKED : MF_UNCHECKED));
	PostMessage(hWnd, WM_SIZE, 0, LOWORD(window.right - window.left) + HIWORD(window.bottom - window.top));
	PostMessage(hWnd, WM_PAINT, 0, LOWORD(window.right - window.left) + HIWORD(window.bottom - window.top));
	return parametr;
}

void Initialization(HDC hdc)
{
	stringMatrix = new string *[rows];
	for (int i = 0; i < rows; i++)
		stringMatrix[i] = new string[columns];

	string str;
	for (char c = fileName.get(); fileName.good(); c = fileName.get())
	{
		if (c != '\n')
			str += c;
	}


	fileName.close();

	int len = str.length();

	int quantityOfCharInCell = (int)len / (columns * rows);
	int charactersLeft = len % (columns * rows);

	int currentOffset = 0;
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < columns; j++)
		{
			int countCharsInThisCell = quantityOfCharInCell;

			if (charactersLeft-- > 0)
				countCharsInThisCell++;

			stringMatrix[i][j] = str.substr(currentOffset, countCharsInThisCell);

			currentOffset += countCharsInThisCell;
		}
	}
}

HFONT generateFont()
{
	int fnWeight = 400;
	DWORD fdwItalic = FALSE;
	DWORD fdwUnderline = FALSE;

	if (isBold) fnWeight += 300;
	if (isCoursive) fdwItalic = TRUE;
	if (isUnderline) fdwUnderline = TRUE;
	HFONT hFont = CreateFont(20, 7, 0, 0, fnWeight, fdwItalic, fdwUnderline, FALSE, RUSSIAN_CHARSET, OUT_OUTLINE_PRECIS,
		CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Times New Roman"));

	return hFont;
}

string getLongestString(int currentLine)
{
	string longestSrting = stringMatrix[currentLine][0];
	for (int j = 1; j < columns; j++)
	{
		if (stringMatrix[currentLine][j].length() > longestSrting.length())
			longestSrting = stringMatrix[currentLine][j];
	}

	return longestSrting;
}

int GetBlockHeight(HDC hdc, int currentLine, int width)
{
	string longestString = getLongestString(currentLine);

	RECT nonDrawableBlock;
	nonDrawableBlock.left = 0;
	nonDrawableBlock.top = 0;
	nonDrawableBlock.bottom = 1;
	nonDrawableBlock.right = width;

	//draw longest string to deside block height
	string a = longestString.c_str();
	int b = longestString.length();

	int height = (int)DrawText(hdc, longestString.c_str(), longestString.length(), &nonDrawableBlock,
		DT_CALCRECT | DT_WORDBREAK | DT_EDITCONTROL | DT_CENTER) + DELTA;

	return  height;
}


void DrawLine(HDC hdc, int x1, int y1, int x2, int y2)
{
	MoveToEx(hdc, x1, y1, NULL);
	LineTo(hdc, x2, y2);
}


void DrawTextBlock(HDC hdc, int left, int top, int width, int height, int raw, int column,int borderSize)
{
	RECT rect;
	rect.top = (long)(top + borderSize);
	rect.left = (long)(left + borderSize);
	rect.right = (long)(left + width - borderSize);
	rect.bottom = (long)(top + height - borderSize);

	DrawText(hdc, stringMatrix[raw][column].c_str(),
		stringMatrix[raw][column].length(),
		&rect,
		DT_WORDBREAK | DT_EDITCONTROL | DT_CENTER);
}

void DrawTable(HDC hdc, int sx, int sy, int borderSize)
{
	float posX = 0, posY = (float)top;
	float hx = (float)sx / columns;
	float hy = 0;
	HFONT hFont;
	string str;

	textSpacing = (int)((hx - 2 * borderSize) / 4);
	hFont = generateFont();

	for (int i = 0; i < rows; i++)
	{
		hy = GetBlockHeight(hdc, i, hx);
		posX = 0;
		for (int j = 0; j < columns; j++)
		{
			//hFont = generateFont();
			SelectObject(hdc, hFont);

			DrawLine(hdc,(int)posX, (int)posY, (int)posX, (int)(posY + hy));
			DrawTextBlock(hdc,posX,posY,hx,hy,i,j,borderSize);
			posX += hx;

			//DeleteObject(hFont);
		}

		DrawLine(hdc, (int)(posX - 1), (int)posY, (int)(posX - 1), (int)(posY + hy));
		DrawLine(hdc, 0, (int)posY, sx, (int)posY);
		posY += hy;
	}
	DrawLine(hdc, 0, (int)posY, sx, (int)posY);
	bottom = (int)posY;
	DeleteObject(hFont);
	TableHeight = posY;
	//TableHeight = 100;
}




