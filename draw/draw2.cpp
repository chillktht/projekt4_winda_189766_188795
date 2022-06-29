#include "stdafx.h"
#include "draw2.h"
#include <cstdio>
#include <sstream>
#include<queue>
#include <vector>
#include <mmsystem.h>
#include <string>


//#pragma comment(lib,"winmm.lib")

#define MAX_LOADSTRING 100
#define TMR_UP 100
#define TMR_DOWN 101
#define MAIN_TIMER 102

const int NUMBER_OF_FLOORS = 5;


// Global Variables:

HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name


INT value;
HWND hWnd;
HWND hWnd_current_destination;
HWND hText;
HWND hWnd_current_load;
HWND hWnd_current_people;
HWND h_floors[NUMBER_OF_FLOORS];
HWND h_floors_calls[NUMBER_OF_FLOORS];


const COLORREF rgbRed = 0x000000FF;
const COLORREF rgbGreen = 0x0000FF00;

struct elevator {
	int load = 0;
	std::vector<int> people;
};

elevator lift;


int wait = 0;

const int ELEV_X_POS = 320;
const int ELEV_WIDTH = 100;
const int ELEV_HEIGHT = 120;
int elev_y_pos = 595;
const int PARTER = 715;
const int FLOOR_1 = 570;
const int FLOOR_2 = 420;
const int FLOOR_3 = 270;
const int FLOOR_4 = 120;

const int fl4height = 5;
const int fl3height = 145;
const int fl2height = 290;
const int fl1height = 445;
const int fl0height = 585;

int current_num_of_people_inside = 0;

LPCSTR queued = "Queued!";
LPCSTR idle = "Idle";

bool is_floor_called[NUMBER_OF_FLOORS] = { 0 };
int how_much_floor_called[NUMBER_OF_FLOORS][NUMBER_OF_FLOORS] = { 0 };

RECT elevArea = { ELEV_X_POS - 5, 0, ELEV_X_POS + ELEV_WIDTH + 5, 735 };


std::queue<int> people_reqs;
std::queue<int> waiting_people_reqs[NUMBER_OF_FLOORS];

std::string string[NUMBER_OF_FLOORS] = { "People's requests: ","People's requests: ",
"People's requests: ", "People's requests: ", "People's requests: " };
std::string current_destination = "Current destination: ";
std::string destination_num = "";

// buttons
HWND hwndButton;

// sent data



// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	Buttons(HWND, UINT, WPARAM, LPARAM);

///functions
void paint_elevator(HDC hdc);
int generate_floor_requests(int floor);


void lift_exit(int floor) {
	is_floor_called[floor] = false;
	SetWindowTextA(h_floors_calls[floor], idle);

	for (int i = 0; i < lift.people.size(); i++) {
		if (lift.people[i] == floor) {
			lift.people.erase(lift.people.begin() + i);
			lift.load--;
		}
	}
}
void lift_enter(int floor) {
	while (!waiting_people_reqs[floor].empty() && lift.load < 8) {
		lift.people.push_back(waiting_people_reqs[floor].front());
		people_reqs.push(waiting_people_reqs[floor].front());
		waiting_people_reqs[floor].pop();
		lift.load++;
		if (string[floor].length() > 20) {
			string[floor].erase(19, 20);
			std::wstring stemp = std::wstring(string[floor].begin(), string[floor].end());
			LPCWSTR sw = stemp.c_str();
			SetWindowText(h_floors[floor], sw);
		}
	}
	std::string temp = "Current load: " + std::to_string(lift.load);
	std::wstring stemp = std::wstring(temp.begin(), temp.end());
	LPCWSTR sw = stemp.c_str();
	SetWindowText(hWnd_current_load, sw);

	temp = "Current people:";
	for (int i = 0; i < lift.people.size(); i++)
	{
		temp = temp + " " + std::to_string(lift.people[i]);
	}
	stemp = std::wstring(temp.begin(), temp.end());
	sw = stemp.c_str();
	SetWindowText(hWnd_current_people, sw);
}


void MyOnPaint(HDC hdc) /// background
{

	Graphics graphics(hdc);
	Pen pen_osx(Color(255, 0, 0, 0), 10);
	Pen pen_osy(Color(100, 0, 0, 0), 4);
	Pen pen_osy2(Color(50, 0, 0, 0), 4);
	graphics.DrawLine(&pen_osx, 0, FLOOR_4, 311, FLOOR_4);
	graphics.DrawLine(&pen_osx, 0, FLOOR_3, 311, FLOOR_3);
	graphics.DrawLine(&pen_osx, 0, FLOOR_2, 311, FLOOR_2);
	graphics.DrawLine(&pen_osx, 0, FLOOR_1, 311, FLOOR_1);
	graphics.DrawLine(&pen_osx, 0, PARTER, 311, PARTER);
	graphics.DrawLine(&pen_osy2, 313, 0, 313, PARTER + 5);
	graphics.DrawLine(&pen_osy, ELEV_X_POS + ELEV_WIDTH + 7, 0, ELEV_X_POS + ELEV_WIDTH + 7, PARTER + 5);
	paint_elevator(hdc);

}


void paint_elevator(HDC hdc) {
	Graphics graphics(hdc);
	Pen pen_elev(Color(255, 120, 25, 187), 10);
	graphics.DrawRectangle(&pen_elev, ELEV_X_POS, elev_y_pos, ELEV_WIDTH, ELEV_HEIGHT);
}


void repaintWindow(HWND hWnd, HDC& hdc, PAINTSTRUCT& ps, RECT* drawArea)
{


	if (drawArea == NULL)
		InvalidateRect(hWnd, NULL, TRUE); // repaint all
	else
		InvalidateRect(hWnd, drawArea, TRUE); //repaint drawArea
	hdc = BeginPaint(hWnd, &ps);
	paint_elevator(hdc);
	EndPaint(hWnd, &ps);
}

void main_f(PAINTSTRUCT ps, HDC hdc) {
	switch (people_reqs.front())
	{
	case 0:
		if (elev_y_pos < fl0height) {
			elev_y_pos++;
			repaintWindow(hWnd, hdc, ps, &elevArea);
		}
		else
			people_reqs.pop();
		break;
	case 1:
		if (elev_y_pos < fl1height) {
			elev_y_pos++;
			repaintWindow(hWnd, hdc, ps, &elevArea);
		}
		else if (elev_y_pos > fl1height) {
			elev_y_pos--;
			repaintWindow(hWnd, hdc, ps, &elevArea);
		}
		else
			people_reqs.pop();
		break;
	case 2:
		if (elev_y_pos < fl2height) {
			elev_y_pos++;
			repaintWindow(hWnd, hdc, ps, &elevArea);
		}
		else if (elev_y_pos > fl2height) {
			elev_y_pos--;
			repaintWindow(hWnd, hdc, ps, &elevArea);
		}
		else
			people_reqs.pop();
		break;
	case 3:
		if (elev_y_pos < fl3height) {
			elev_y_pos++;
			repaintWindow(hWnd, hdc, ps, &elevArea);
		}
		else if (elev_y_pos > fl3height) {
			elev_y_pos--;
			repaintWindow(hWnd, hdc, ps, &elevArea);
		}
		else
			people_reqs.pop();
		break;
	case 4:if (elev_y_pos > fl0height) {
		elev_y_pos--;
		repaintWindow(hWnd, hdc, ps, &elevArea);
	}
		  else
		people_reqs.pop();
		break;
	default:
		break;
	}
}


int OnCreate(HWND window)
{
	srand(time(NULL));

	return 0;
}


// main function (exe hInstance)
int APIENTRY _tWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow)
{

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	value = 0;

	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_DRAW, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);



	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DRAW));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	GdiplusShutdown(gdiplusToken);

	return (int)msg.wParam;
}

int generate_floor_requests(int floor) {
	int los = -1;
	los = rand() % 5;
	while (los == floor)
	{
		los = rand() % 5;

	}
	return los;
}




//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DRAW));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCE(IDC_DRAW);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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
	//HWND hWnd;



	hInst = hInstance; // Store instance handle (of exe) in our global variable

	// main window
	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);


	hwndButton = CreateWindow(TEXT("button"),                      // The class name required is button  //  parter
		TEXT("PARTER"),                  // the caption of the button
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,  // the styles
		10, 600,                                  // the left and top co-ordinates
		100, 60,                              // width and height
		hWnd,                                 // parent window handle
		(HMENU)ID_BUTTON5,                   // the ID of your button
		hInstance,                            // the instance of your application
		NULL);


	hwndButton = CreateWindow(TEXT("button"),                      // The class name required is button  //  1st floor
		TEXT("1"),                  // the caption of the button
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,  // the styles
		10, 450,                                  // the left and top co-ordinates
		60, 60,                              // width and height
		hWnd,                                 // parent window handle
		(HMENU)ID_BUTTON1,                   // the ID of your button
		hInstance,                            // the instance of your application
		NULL);                               // extra bits you dont really need

	hwndButton = CreateWindow(TEXT("button"),                      // The class name required is button
		TEXT("2"),                  // the caption of the button
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,  // the styles
		10, 300,                                  // the left and top co-ordinates
		60, 60,                              // width and height
		hWnd,                                 // parent window handle
		(HMENU)ID_BUTTON2,                   // the ID of your button
		hInstance,                            // the instance of your application
		NULL);                               // extra bits you dont really need


	hwndButton = CreateWindow(TEXT("button"),                      // The class name required is button
		TEXT("3"),                  // the caption of the button
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,  // the styles
		10, 150,                                  // the left and top co-ordinates
		60, 60,                              // width and height
		hWnd,                                 // parent window handle
		(HMENU)ID_BUTTON3,                   // the ID of your button
		hInstance,                            // the instance of your application
		NULL);                               // extra bits you dont really need


	hwndButton = CreateWindow(TEXT("button"),                      // The class name required is button
		TEXT("4"),                  // the caption of the button
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,  // the styles
		10, 10,                                  // the left and top co-ordinates
		60, 60,                              // width and height
		hWnd,                                 // parent window handle
		(HMENU)ID_BUTTON4,                   // the ID of your button
		hInstance,                            // the instance of your application
		NULL);                               // extra bits you dont really need


	hWnd_current_destination = CreateWindow(TEXT("EDIT"),
		TEXT("Current destination: "),
		WS_CHILD | WS_VISIBLE,
		900, 120,
		200, 60,
		hWnd, 0, 0, 0);

	hWnd_current_load = CreateWindow(TEXT("EDIT"),
		TEXT("Current load"),
		WS_CHILD | WS_VISIBLE,
		900, 280,
		200, 60,
		hWnd, 0, 0, 0);

	hWnd_current_people = CreateWindow(TEXT("EDIT"),
		TEXT("Current people"),
		WS_CHILD | WS_VISIBLE,
		900, 320,
		200, 60,
		hWnd, 0, 0, 0);


	for (int i = 0, j = 635; i < 5; i++) {
		h_floors[i] = CreateWindow(TEXT("EDIT"),
			TEXT("People's requests: "),
			WS_CHILD | WS_VISIBLE,
			700, j,
			500, 60,
			hWnd, 0, 0, 0);

		h_floors_calls[i] = CreateWindow(TEXT("EDIT"),
			TEXT("idle"),
			WS_CHILD | WS_VISIBLE | WS_BORDER,
			150, j,
			100, 60,
			hWnd, 0, 0, 0);

		j -= 150;
	}



	OnCreate(hWnd);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	SetTimer(hWnd,             // handle to main window 
		MAIN_TIMER,            // timer identifier 
		1,                 // 1/10-second interval 
		(TIMERPROC)NULL);     // no timer callback 

	return TRUE;
}

std::string create_floor_req_string(int los, int floor) {
	string[floor] = string[floor] + std::to_string(los) + " ";
	return string[floor];
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window (low priority)
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{


	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	DWORD dlugosc = GetWindowTextLength(hText);
	LPWSTR Bufor = (LPWSTR)GlobalAlloc(GPTR, dlugosc + 1);
	std::wstring n_str;
	switch (message)
	{
	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);

		// MENU & BUTTON messages
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case ID_BUTTON5: //parter
			if (!is_floor_called[0])
				people_reqs.push(0);
			is_floor_called[0] = true;
			{int req = generate_floor_requests(0);
			waiting_people_reqs[0].push(req);
			string[0] = create_floor_req_string(req, 0);
			std::wstring stemp = std::wstring(string[0].begin(), string[0].end());
			LPCWSTR sw = stemp.c_str();
			SetWindowText(h_floors[0], sw); }
			SetWindowTextA(h_floors_calls[0], queued);

			break;
		case ID_BUTTON1: //1st floor
			if (!is_floor_called[1])
				people_reqs.push(1);
			is_floor_called[1] = true;
			{int req = generate_floor_requests(1);
			waiting_people_reqs[1].push(req);
			string[1] = create_floor_req_string(req, 1);
			std::wstring stemp = std::wstring(string[1].begin(), string[1].end()); //string to lpcstr conversion
			LPCWSTR sw = stemp.c_str();
			SetWindowText(h_floors[1], sw); }
			SetWindowTextA(h_floors_calls[1], queued);


			break;
		case ID_BUTTON2: //2nd floor
			if (!is_floor_called[2])
				people_reqs.push(2);
			is_floor_called[2] = true;
			{int req = generate_floor_requests(2);
			waiting_people_reqs[2].push(req);

			string[2] = create_floor_req_string(req, 2);
			std::wstring stemp = std::wstring(string[2].begin(), string[2].end());
			LPCWSTR sw = stemp.c_str();
			SetWindowText(h_floors[2], sw); }

			SetWindowTextA(h_floors_calls[2], queued);

			break;
		case ID_BUTTON3: //3rd floor
			if (!is_floor_called[3])
				people_reqs.push(3);
			is_floor_called[3] = true;

			{int req = generate_floor_requests(3);
			waiting_people_reqs[3].push(req);
			string[3] = create_floor_req_string(req, 3);
			std::wstring stemp = std::wstring(string[3].begin(), string[3].end());
			LPCWSTR sw = stemp.c_str();
			SetWindowText(h_floors[3], sw); }

			SetWindowTextA(h_floors_calls[3], queued);

			break;
		case ID_BUTTON4: //4th floor
			if (!is_floor_called[4])
				people_reqs.push(4);
			is_floor_called[4] = true;

			{int req = generate_floor_requests(4);
			waiting_people_reqs[4].push(req);
			string[4] = create_floor_req_string(req, 4);
			std::wstring stemp = std::wstring(string[4].begin(), string[4].end());
			LPCWSTR sw = stemp.c_str();
			SetWindowText(h_floors[4], sw); }

			SetWindowTextA(h_floors_calls[4], queued);

			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here (not depend on timer, buttons)
		MyOnPaint(hdc);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_TIMER:
		switch (wParam)
		{
		case MAIN_TIMER:
			if (wait > 0) {
				wait--;
				if (wait == 39) {
					if (elev_y_pos == fl4height) {
						people_reqs.pop();
						lift_exit(4);
						lift_enter(4);
					}
					if (elev_y_pos == fl3height) {
						people_reqs.pop();
						lift_exit(3);
						lift_enter(3);
					}
					if (elev_y_pos == fl2height) {
						people_reqs.pop();
						lift_exit(2);
						lift_enter(2);
					}
					if (elev_y_pos == fl1height) {
						people_reqs.pop();
						lift_exit(1);
						lift_enter(1);
					}
					if (elev_y_pos == fl0height) {
						people_reqs.pop();
						lift_exit(0);
						lift_enter(0);
					}
				}
			}
			else {
				if (!people_reqs.empty()) {
					switch (people_reqs.front())
					{
					case 0:
						if (destination_num != "0") {
							destination_num = "0";
							std::string temp = current_destination + "0";
							std::wstring stemp = std::wstring(temp.begin(), temp.end());
							LPCWSTR sw = stemp.c_str();
							SetWindowText(hWnd_current_destination, sw);
						}

						if (elev_y_pos < fl0height) {
							elev_y_pos++;
							repaintWindow(hWnd, hdc, ps, &elevArea);
						}
						else {
							//people_reqs.pop();
							wait = 40;
						}
						break;
					case 1:
						if (destination_num != "1") {
							destination_num = "1";
							std::string temp = current_destination + "1";
							std::wstring stemp = std::wstring(temp.begin(), temp.end());
							LPCWSTR sw = stemp.c_str();
							SetWindowText(hWnd_current_destination, sw);
						}

						if (elev_y_pos < fl1height) {
							elev_y_pos++;
							repaintWindow(hWnd, hdc, ps, &elevArea);
						}
						else if (elev_y_pos > fl1height) {
							elev_y_pos--;
							repaintWindow(hWnd, hdc, ps, &elevArea);
						}
						else {
							//people_reqs.pop();
							wait = 40;
						}
						break;
					case 2:
						if (destination_num != "2") {
							destination_num = "2";
							std::string temp = current_destination + "2";
							std::wstring stemp = std::wstring(temp.begin(), temp.end());
							LPCWSTR sw = stemp.c_str();
							SetWindowText(hWnd_current_destination, sw);
						}
						if (elev_y_pos < fl2height) {
							elev_y_pos++;
							repaintWindow(hWnd, hdc, ps, &elevArea);
						}
						else if (elev_y_pos > fl2height) {
							elev_y_pos--;
							repaintWindow(hWnd, hdc, ps, &elevArea);
						}
						else {
							//people_reqs.pop();
							wait = 40;
						}
						break;
					case 3:
						if (destination_num != "3") {
							destination_num = "3";
							std::string temp = current_destination + "3";
							std::wstring stemp = std::wstring(temp.begin(), temp.end());
							LPCWSTR sw = stemp.c_str();
							SetWindowText(hWnd_current_destination, sw);
						}
						if (elev_y_pos < fl3height) {
							elev_y_pos++;
							repaintWindow(hWnd, hdc, ps, &elevArea);
						}
						else if (elev_y_pos > fl3height) {
							elev_y_pos--;
							repaintWindow(hWnd, hdc, ps, &elevArea);
						}
						else {
							//people_reqs.pop();
							wait = 40;
						}
						break;
					case 4:
						if (destination_num != "4") {
							destination_num = "4";
							std::string temp = current_destination + "4";
							std::wstring stemp = std::wstring(temp.begin(), temp.end());
							LPCWSTR sw = stemp.c_str();
							SetWindowText(hWnd_current_destination, sw);
						}

						if (elev_y_pos > fl4height) {
							elev_y_pos--;
							repaintWindow(hWnd, hdc, ps, &elevArea);
						}
						else {
							//people_reqs.pop();
							wait = 40;
						}
						break;
					default:
						break;
					}
				}
			}
			break;
			//case MAIN_TIMER:
			//	if (wait > 0) {
			//		wait--;
			//		if (elev_y_pos == fl4height) {
			//			is_floor_called[4] = false;
			//			SetWindowTextA(h_floors_calls[4], idle);
			//			if (!waiting_people_reqs[4].empty()) {
			//			current_num_of_people_inside -= how_much_floor_called[4][waiting_people_reqs[4].front()];
			//			current_num_of_people_inside -= how_much_floor_called[3][waiting_people_reqs[4].front()];
			//			current_num_of_people_inside -= how_much_floor_called[2][waiting_people_reqs[4].front()];
			//			current_num_of_people_inside -= how_much_floor_called[1][waiting_people_reqs[4].front()];
			//			current_num_of_people_inside -= how_much_floor_called[0][waiting_people_reqs[4].front()];
			//			
			//		}
			//			while (!waiting_people_reqs[4].empty() && current_num_of_people_inside < 8) { // 8 osob = 560 kg (maksymalna ilosc)
			//				current_num_of_people_inside+=how_much_floor_called[4][waiting_people_reqs[4].front()];
							//std::string temp = "Current load: " + std::to_string(current_num_of_people_inside);
							//std::wstring stemp = std::wstring(temp.begin(), temp.end());
							//LPCWSTR sw = stemp.c_str();
							//SetWindowText(hWnd_current_load, sw);
			//				people_reqs.push(waiting_people_reqs[4].front());
			//				
			//				waiting_people_reqs[4].pop();

							//if (string[4].length() > 20) {
							//	string[4].erase(19, 20);
							//	std::wstring stemp = std::wstring(string[4].begin(), string[4].end());
							//	LPCWSTR sw = stemp.c_str();
							//	SetWindowText(h_floors[4], sw);
			//				}
			//			}
			//		}
			//		if (elev_y_pos == fl3height) {
			//			is_floor_called[3] = false;
			//			SetWindowTextA(h_floors_calls[3], idle);
			//			if (!waiting_people_reqs[3].empty()) {
			//			current_num_of_people_inside -= how_much_floor_called[4][waiting_people_reqs[3].front()];
			//			current_num_of_people_inside -= how_much_floor_called[3][waiting_people_reqs[3].front()];
			//			current_num_of_people_inside -= how_much_floor_called[2][waiting_people_reqs[3].front()];
			//			current_num_of_people_inside -= how_much_floor_called[1][waiting_people_reqs[3].front()];
			//			current_num_of_people_inside -= how_much_floor_called[0][waiting_people_reqs[3].front()];
			//			
			//		}
			//			while (!waiting_people_reqs[3].empty() && current_num_of_people_inside < 8) {
			//				current_num_of_people_inside += how_much_floor_called[3][waiting_people_reqs[3].front()];
			//				std::string temp = "Current load: " + std::to_string(current_num_of_people_inside);
			//				std::wstring stemp = std::wstring(temp.begin(), temp.end());
			//				LPCWSTR sw = stemp.c_str();
			//				SetWindowText(hWnd_current_load, sw);
			//				people_reqs.push(waiting_people_reqs[3].front());
			//				waiting_people_reqs[3].pop();
			//				if (string[3].length() > 20) {
			//					string[3].erase(19, 20);
			//					std::wstring stemp = std::wstring(string[3].begin(), string[3].end());
			//					LPCWSTR sw = stemp.c_str();
			//					SetWindowText(h_floors[3], sw);
			//				}
			//			}
			//		}
			//		if (elev_y_pos == fl2height) {
			//			is_floor_called[2] = false;
			//			SetWindowTextA(h_floors_calls[2], idle);
			//			if (!waiting_people_reqs[2].empty()) {
			//			current_num_of_people_inside -= how_much_floor_called[4][waiting_people_reqs[2].front()];
			//			current_num_of_people_inside -= how_much_floor_called[3][waiting_people_reqs[2].front()];
			//			current_num_of_people_inside -= how_much_floor_called[2][waiting_people_reqs[2].front()];
			//			current_num_of_people_inside -= how_much_floor_called[1][waiting_people_reqs[2].front()];
			//			current_num_of_people_inside -= how_much_floor_called[0][waiting_people_reqs[2].front()];
			//			
			//		}
			//			while (!waiting_people_reqs[2].empty() && current_num_of_people_inside < 8) {
			//				current_num_of_people_inside += how_much_floor_called[2][waiting_people_reqs[2].front()];
			//				std::string temp = "Current load: " + std::to_string(current_num_of_people_inside);
			//				std::wstring stemp = std::wstring(temp.begin(), temp.end());
			//				LPCWSTR sw = stemp.c_str();
			//				SetWindowText(hWnd_current_load, sw);
			//				people_reqs.push(waiting_people_reqs[2].front());
			//				waiting_people_reqs[2].pop();
			//				if (string[2].length() > 20) {
			//					string[2].erase(19, 20);
			//					std::wstring stemp = std::wstring(string[2].begin(), string[2].end());
			//					LPCWSTR sw = stemp.c_str();
			//					SetWindowText(h_floors[2], sw);
			//				}
			//			}
			//		}
			//		if (elev_y_pos == fl1height) {
			//			is_floor_called[1] = false;
			//			SetWindowTextA(h_floors_calls[1], idle);
			//			if (!waiting_people_reqs[1].empty()) {
			//			current_num_of_people_inside -= how_much_floor_called[4][waiting_people_reqs[1].front()];
			//			current_num_of_people_inside -= how_much_floor_called[3][waiting_people_reqs[1].front()];
			//			current_num_of_people_inside -= how_much_floor_called[2][waiting_people_reqs[1].front()];
			//			current_num_of_people_inside -= how_much_floor_called[1][waiting_people_reqs[1].front()];
			//			current_num_of_people_inside -= how_much_floor_called[0][waiting_people_reqs[1].front()];
			//			
			//		}
			//			while (!waiting_people_reqs[1].empty() && current_num_of_people_inside < 8) {
			//				current_num_of_people_inside += how_much_floor_called[1][waiting_people_reqs[1].front()];
			//				std::string temp = "Current load: " + std::to_string(current_num_of_people_inside);
			//				std::wstring stemp = std::wstring(temp.begin(), temp.end());
			//				LPCWSTR sw = stemp.c_str();
			//				SetWindowText(hWnd_current_load, sw);
			//				people_reqs.push(waiting_people_reqs[1].front());
			//				waiting_people_reqs[1].pop();
			//				if (string[1].length() > 20) {
			//					string[1].erase(19, 20);
			//					std::wstring stemp = std::wstring(string[1].begin(), string[1].end());
			//					LPCWSTR sw = stemp.c_str();
			//					SetWindowText(h_floors[1], sw);
			//				}
			//			}
			//		}
			//		if (elev_y_pos == fl0height) {
			//			is_floor_called[0] = false;
			//			SetWindowTextA(h_floors_calls[0], idle);
			//			if (!waiting_people_reqs[0].empty()) {
			//			current_num_of_people_inside -= how_much_floor_called[4][waiting_people_reqs[0].front()];
			//			current_num_of_people_inside -= how_much_floor_called[3][waiting_people_reqs[0].front()];
			//			current_num_of_people_inside -= how_much_floor_called[2][waiting_people_reqs[0].front()];
			//			current_num_of_people_inside -= how_much_floor_called[1][waiting_people_reqs[0].front()];
			//			current_num_of_people_inside -= how_much_floor_called[0][waiting_people_reqs[0].front()];
			//			
			//		}
			//			while (!waiting_people_reqs[0].empty() && current_num_of_people_inside < 8) {
			//				current_num_of_people_inside += how_much_floor_called[0][waiting_people_reqs[0].front()];
			//				std::string temp ="Current load: " +std::to_string(current_num_of_people_inside);
			//				std::wstring stemp = std::wstring(temp.begin(), temp.end());
			//				LPCWSTR sw = stemp.c_str();
			//				SetWindowText(hWnd_current_load, sw);
			//				people_reqs.push(waiting_people_reqs[0].front());
			//				waiting_people_reqs[0].pop();
			//				if (string[0].length() > 20) {
			//					string[0].erase(19, 20);
			//					std::wstring stemp = std::wstring(string[0].begin(), string[0].end());
			//					LPCWSTR sw = stemp.c_str();
			//					SetWindowText(h_floors[0], sw);
			//				}
			//			}
			//		}
			//	}

				//else {
				//	if (!people_reqs.empty()) {
				//		switch (people_reqs.front())
				//		{
				//		case 0:
				//			if (destination_num != "0") {
				//				destination_num = "0";
				//				std::string temp = current_destination + "0";
				//				std::wstring stemp = std::wstring(temp.begin(), temp.end());
				//				LPCWSTR sw = stemp.c_str();
				//				SetWindowText(hWnd_current_destination, sw);
				//			}

				//			if (elev_y_pos < fl0height) {
				//				elev_y_pos++;
				//				repaintWindow(hWnd, hdc, ps, &elevArea);
				//			}
				//			else {
				//				people_reqs.pop();
				//				wait = 40;;
				//			}
				//			break;
				//		case 1:
				//			if (destination_num != "1") {
				//				destination_num = "1";
				//				std::string temp = current_destination + "1";
				//				std::wstring stemp = std::wstring(temp.begin(), temp.end());
				//				LPCWSTR sw = stemp.c_str();
				//				SetWindowText(hWnd_current_destination, sw);
				//			}

				//			if (elev_y_pos < fl1height) {
				//				elev_y_pos++;
				//				repaintWindow(hWnd, hdc, ps, &elevArea);
				//			}
				//			else if (elev_y_pos > fl1height) {
				//				elev_y_pos--;
				//				repaintWindow(hWnd, hdc, ps, &elevArea);
				//			}
				//			else {
				//				people_reqs.pop();
				//				wait = 40;;
				//			}
				//			break;
				//		case 2:
				//			if (destination_num != "2") {
				//				destination_num = "2";
				//				std::string temp = current_destination + "2";
				//				std::wstring stemp = std::wstring(temp.begin(), temp.end());
				//				LPCWSTR sw = stemp.c_str();
				//				SetWindowText(hWnd_current_destination, sw);
				//			}
				//			if (elev_y_pos < fl2height) {
				//				elev_y_pos++;
				//				repaintWindow(hWnd, hdc, ps, &elevArea);
				//			}
				//			else if (elev_y_pos > fl2height) {
				//				elev_y_pos--;
				//				repaintWindow(hWnd, hdc, ps, &elevArea);
				//			}
				//			else {
				//				people_reqs.pop();
				//				wait = 40;;
				//			}
				//			break;
				//		case 3:
				//			if (destination_num != "3") {
				//				destination_num = "3";
				//				std::string temp = current_destination + "3";
				//				std::wstring stemp = std::wstring(temp.begin(), temp.end());
				//				LPCWSTR sw = stemp.c_str();
				//				SetWindowText(hWnd_current_destination, sw);
				//			}
				//			if (elev_y_pos < fl3height) {
				//				elev_y_pos++;
				//				repaintWindow(hWnd, hdc, ps, &elevArea);
				//			}
				//			else if (elev_y_pos > fl3height) {
				//				elev_y_pos--;
				//				repaintWindow(hWnd, hdc, ps, &elevArea);
				//			}
				//			else {
				//				people_reqs.pop();
				//				wait = 40;;
				//			}
				//			break;
				//		case 4:
				//			if (destination_num != "4") {
				//				destination_num = "4";
				//				std::string temp = current_destination + "4";
				//				std::wstring stemp = std::wstring(temp.begin(), temp.end());
				//				LPCWSTR sw = stemp.c_str();
				//				SetWindowText(hWnd_current_destination, sw);
				//			}

				//			if (elev_y_pos > fl4height) {
				//				elev_y_pos--;
				//				repaintWindow(hWnd, hdc, ps, &elevArea);
				//			}
				//			else {
				//				people_reqs.pop();
				//				wait = 40;;
				//			}
				//			break;
				//		default:
				//			break;
				//		}
				//	}
				//}
				//break;
		case TMR_UP:
			elev_y_pos--;
			repaintWindow(hWnd, hdc, ps, &elevArea);
			value++;
			if (elev_y_pos == 6)
				KillTimer(hWnd, TMR_UP);
			break;
		}
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
