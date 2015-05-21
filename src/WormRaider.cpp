#include "stdafx.h"
#include "resource.h"
#include "mmsystem.h"

#define MAX_LOADSTRING	100
#define MAX_LEVEL		4
#define BATTLE_TIMER	1
#define MOVE_TIMER		2
#define SCREEN_SIZE		400
#define WALL_SIZE		10
#define EYE_BACK		7.5
#define VIEW_SIZE		6
#define SCREEN_UNIT		(SCREEN_SIZE/VIEW_SIZE)
#define EYE_LEVEL		5
#define ORIGIN			(SCREEN_SIZE/2)
#define EASY_PLAY       0
#define MEDIUM_PLAY     1
#define HARD_PLAY       2

extern BOOL showmap = FALSE;
extern BOOL showmsg = FALSE;
extern BOOL showexp = FALSE;
extern BOOL incombat = FALSE;
extern BOOL showstats = FALSE;
extern BOOL shakewindow = TRUE;
extern BOOL showhelp = TRUE;
extern int difficulty = MEDIUM_PLAY;
extern int my_level = 0;
extern double movetick = 0;
extern int experience = 0;
extern int myhits = 100;
extern int myx=1;
extern int myy=1;
extern int mydx=0;
extern int mydy=1;
extern char myMsg[100] = ""; 
extern WPARAM winx=0;
extern LPARAM winy=0;

#include "wormraider_map.h"

#define WALL	0
#define FLOOR	1
#define SPIDER1	2
#define SPIDER2 3
HBRUSH WRbrush[4][9];

HINSTANCE hInst;
TCHAR szTitle[MAX_LOADSTRING] = "WormRaider v2.0.0";
TCHAR szWindowClass[MAX_LOADSTRING] = "WormRaider";


ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

void scanAround (HDC, int, int, int, int);
void drawWalls (HDC, int, int, int, int, int, int, int);
void scanMap (HDC, int, int, int, int);
void drawMap (HDC, int, int, int, int, int);
void scanHits (HDC, int, int);
void sightStore (int, int, int, int);
void sightScan(void);
void sightWipe(void);
void showMsg(HDC);
void showExp(HDC);
void showStats(HDC);
void showHelp(HDC);
void moveMnstr(int,int);
void startGame(HWND);
void saveGame(HWND);
void loadGame(HWND);

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	MSG msg;
	MyRegisterClass(hInstance);
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
	}
	return msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_ICON1);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(BLACK_BRUSH);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_ICON2);

	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance;

   hWnd = CreateWindow (szWindowClass, 
                        szTitle,
                        (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU),
                        0,						  // initial x position
                        0,						  // initial y position
                        408,						  // initial x size
                        476,						  // initial y size
                        NULL,                       // parent window handle
                        NULL,                       // window menu handle
                        hInstance,                  // program instance handle
                        NULL) ;
   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	        HDC         hdc;
	 static HDC         hdcBuffer, hdcBack;
	 static HBITMAP     hbitmap, hbitBack;
			PAINTSTRUCT ps ;
			RECT        rect ;
	 static	HBRUSH      hBrush;
	 static int         q,temp,crhits;
     switch (message)
     {
     case WM_CREATE:
		  //define player and timers
		  //myx=1;myy=1;mydx=0;mydy=1;
		  crhits=-1;
		  SetTimer (hwnd, BATTLE_TIMER, 1000, NULL);
		  //create graphic buffers
		  hdc = GetDC(hwnd);
		  hdcBuffer = CreateCompatibleDC(hdc);
		  hbitmap = CreateCompatibleBitmap(hdc, SCREEN_SIZE, SCREEN_SIZE+50);
		  hdcBack = CreateCompatibleDC(hdc);
		  hbitBack = CreateCompatibleBitmap(hdc, SCREEN_SIZE, SCREEN_SIZE/3);
		  ReleaseDC (hwnd, hdc);
		  //get initial info
		  startGame(hwnd);
		  //create brushes
		  WRbrush[WALL][8] = CreateSolidBrush(RGB(0,0,0));
		  WRbrush[WALL][7] = CreateSolidBrush(RGB(32,32,0));
		  WRbrush[WALL][6] = CreateSolidBrush(RGB(64,64,25));
		  WRbrush[WALL][5] = CreateSolidBrush(RGB(96,96,57));
		  WRbrush[WALL][4] = CreateSolidBrush(RGB(128,128,89));
		  WRbrush[WALL][3] = CreateSolidBrush(RGB(160,160,121));
		  WRbrush[WALL][2] = CreateSolidBrush(RGB(192,192,153));
		  WRbrush[WALL][1] = CreateSolidBrush(RGB(224,224,185));
		  WRbrush[WALL][0] = CreateSolidBrush(RGB(224,224,185));
		  
		  WRbrush[FLOOR][8] = CreateSolidBrush(RGB(0,0,0));
		  WRbrush[FLOOR][7] = CreateSolidBrush(RGB(32,32,0));
		  WRbrush[FLOOR][6] = CreateSolidBrush(RGB(64,64,0));
		  WRbrush[FLOOR][5] = CreateSolidBrush(RGB(96,96,32));
		  WRbrush[FLOOR][4] = CreateSolidBrush(RGB(128,128,64));
		  WRbrush[FLOOR][3] = CreateSolidBrush(RGB(160,160,96));
		  WRbrush[FLOOR][2] = CreateSolidBrush(RGB(192,192,128));
		  WRbrush[FLOOR][1] = CreateSolidBrush(RGB(224,224,160));
		  WRbrush[FLOOR][0] = CreateSolidBrush(RGB(224,224,160));
		  
		  WRbrush[SPIDER1][8] = CreateSolidBrush(RGB(0,0,0));
		  WRbrush[SPIDER1][7] = CreateSolidBrush(RGB(32,52,0));
		  WRbrush[SPIDER1][6] = CreateSolidBrush(RGB(64,84,0));
		  WRbrush[SPIDER1][5] = CreateSolidBrush(RGB(96,116,32));
		  WRbrush[SPIDER1][4] = CreateSolidBrush(RGB(128,148,64));
		  WRbrush[SPIDER1][3] = CreateSolidBrush(RGB(160,180,96));
		  WRbrush[SPIDER1][2] = CreateSolidBrush(RGB(192,212,128));
		  WRbrush[SPIDER1][1] = CreateSolidBrush(RGB(224,244,160));
		  WRbrush[SPIDER1][0] = CreateSolidBrush(RGB(224,244,160));

		  WRbrush[SPIDER2][8] = CreateSolidBrush(RGB(0,0,0));
		  WRbrush[SPIDER2][7] = CreateSolidBrush(RGB(52,32,0));
		  WRbrush[SPIDER2][6] = CreateSolidBrush(RGB(84,64,0));
		  WRbrush[SPIDER2][5] = CreateSolidBrush(RGB(116,96,32));
		  WRbrush[SPIDER2][4] = CreateSolidBrush(RGB(148,128,64));
		  WRbrush[SPIDER2][3] = CreateSolidBrush(RGB(180,160,96));
		  WRbrush[SPIDER2][2] = CreateSolidBrush(RGB(212,192,128));
		  WRbrush[SPIDER2][1] = CreateSolidBrush(RGB(244,224,160));
		  WRbrush[SPIDER2][0] = CreateSolidBrush(RGB(244,224,160));
		  //draw background sky
		  SelectObject(hdcBack,hbitBack);
		  SelectObject (hdcBack, GetStockObject(NULL_PEN));
		  for (q=0; q<=SCREEN_SIZE/3; q++)
		  {
			hBrush = CreateSolidBrush(RGB(max(100*(1-(q/(SCREEN_SIZE/3.0))),0),max(100*(1-(q/(SCREEN_SIZE/3.0))),0),max(170*(1-(q/(SCREEN_SIZE/3.0))),0)));
			SelectObject (hdcBack, hBrush);
			Rectangle (hdcBack, 0,q,SCREEN_SIZE+1,q+2);
			DeleteObject(hBrush);
		  }
		  SelectObject(hdcBuffer,hbitmap);
          return 0;

	 case WM_TIMER:
		  
		 if (incombat)
		  {
			//set creature hits
			if (mydy != 0 && myMap[my_level][myx+mydy][myy]==3 && crhits==-1) crhits=60;
			else if (myMap[my_level][myx][myy+mydx]==3 && crhits==-1) crhits=60;
			if (mydy != 0 && myMap[my_level][myx+mydy][myy]==6 && crhits==-1) crhits=100;
			else if (myMap[my_level][myx][myy+mydx]==6 && crhits==-1) crhits=100;
			if (rand()%10 <= 2)
			{
				myhits -= rand()%15;
				if (mydy != 0 && myMap[my_level][myx+mydy][myy]==6) myhits -= 5;
				else if (myMap[my_level][myx][myy+mydx]==6) myhits -= 5;
				switch (rand()%4)
				{
				case 0: PlaySound((PSTR)IDR_WAVE4,hInst,SND_RESOURCE|SND_SYNC);break;
				case 1: PlaySound((PSTR)IDR_WAVE5,hInst,SND_RESOURCE|SND_SYNC);break;
				case 2: PlaySound((PSTR)IDR_WAVE6,hInst,SND_RESOURCE|SND_SYNC);break;
				case 3: PlaySound((PSTR)IDR_WAVE7,hInst,SND_RESOURCE|SND_SYNC);break;
				}
			}
			else if (rand()%10 <= 5+(experience/20.0))
			{
				crhits -= (rand()%15)+experience;
				PlaySound((PSTR)IDR_WAVE11,hInst,SND_RESOURCE|SND_SYNC);
				switch (rand()%3)
				{
				case 0: PlaySound((PSTR)IDR_WAVE8,hInst,SND_RESOURCE|SND_SYNC);break;
				case 1: PlaySound((PSTR)IDR_WAVE9,hInst,SND_RESOURCE|SND_SYNC);break;
				case 2: PlaySound((PSTR)IDR_WAVE10,hInst,SND_RESOURCE|SND_SYNC);break;
				}
			}
			else
			{
				PlaySound((PSTR)IDR_WAVE13,hInst,SND_RESOURCE|SND_SYNC);
				/*switch (rand()%2)
				{
				case 0: PlaySound((PSTR)IDR_WAVE12,hInst,SND_RESOURCE|SND_SYNC);break;
				case 1: PlaySound((PSTR)IDR_WAVE13,hInst,SND_RESOURCE|SND_SYNC);break;
				}*/
			}
			if (crhits < 1) 
			{
				incombat = FALSE;
				experience++;
				if (mydy != 0 && myMap[my_level][myx+mydy][myy]==6) experience += 2;
				else if (myMap[my_level][myx][myy+mydx]==6) experience += 2;
				if (mydy != 0) myMap[my_level][myx+mydy][myy] = 0;
				else myMap[my_level][myx][myy+mydx] = 0;
				crhits = -1;
				showexp = TRUE;
			}
			if (myhits < 1)
			{
				KillTimer (hwnd, BATTLE_TIMER);
				DestroyWindow (hwnd) ;
			}
		  }
		  else
		  {
			if (myhits > 100) myhits = 100;
		  }
		  if (incombat || myhits < 100) InvalidateRect (hwnd, NULL, 0);
		  return 0;

     case WM_PAINT:
          hdc = BeginPaint (hwnd, &ps) ;
          
          GetClientRect (hwnd, &rect) ;
		  SelectObject (hdcBuffer, GetStockObject(BLACK_BRUSH));
		  SelectObject (hdcBuffer, GetStockObject(NULL_PEN));
		  Rectangle (hdcBuffer, -1, -1, SCREEN_SIZE+1, SCREEN_SIZE+51);
		  BitBlt (hdcBuffer, 0,0, SCREEN_SIZE, SCREEN_SIZE/3, hdcBack, 0,0, SRCCOPY);
		  SelectObject (hdcBuffer, GetStockObject(BLACK_PEN));
		  SelectObject (hdcBuffer, GetStockObject(WHITE_BRUSH));
		  scanAround(hdcBuffer, myx, myy, mydx, mydy);
		  sightScan();
		  miniMap[my_level][myx][myy]=1;
		  miniMap[my_level][myx+mydy][myy+mydx]=1;
		  miniMap[my_level][myx+mydx][myy+mydy]=1;
		  miniMap[my_level][myx-mydx][myy-mydy]=1;
		  if (showmap) 
		  {
			  scanMap(hdcBuffer, myx, myy, mydx, mydy);
		  }
		  if (myMap[my_level][myx+mydy][myy+mydx]==3 || myMap[my_level][myx+mydy][myy+mydx]==6)
		  {
			incombat = TRUE;
		  }
		  scanHits(hdcBuffer, myhits, crhits);
		  if (showstats) showStats(hdcBuffer);
		  if (showhelp) showHelp(hdcBuffer);
		  if (showexp) showExp(hdcBuffer);
		  if (showmsg) showMsg(hdcBuffer);
          BitBlt (hdc, 0,0, SCREEN_SIZE+1,SCREEN_SIZE+51, hdcBuffer, 0,0, SRCCOPY);
		  sightWipe();
          EndPaint (hwnd, &ps) ;
          return 0 ;
	 case WM_MOVE:
		  winx = LOWORD(lParam)-3;
		  winy = HIWORD(lParam)-22;
		  return 0;
     case WM_KEYDOWN:
		if (!incombat)
		{
		showexp = FALSE;
		showmsg = FALSE;
		showstats = FALSE;
		 switch (wParam)
			{
			case VK_UP:
			case VK_NUMPAD8:
				myhits += 2;
				if (myhits > 100) myhits = 100;
				if ((mydy != 0 && !myMap[my_level][myx+mydy][myy])||(mydx != 0 && !myMap[my_level][myx][myy+mydx])) 
					{
					myx += mydy;
					myy += mydx;
					}
				else if ((mydy != 0 && myMap[my_level][myx+mydy][myy]==1 && (myMap[my_level][myx+mydy+mydy][myy]==0 || myMap[my_level][myx+mydy+mydy][myy]==4) && (myx+mydy+mydy>0 && myx+mydy+mydy<20))||(mydx != 0 && myMap[my_level][myx][myy+mydx]==1 && (myMap[my_level][myx][myy+mydx+mydx]==0 || myMap[my_level][myx][myy+mydx+mydx]==4) &&(myy+mydx+mydx>0 && myy+mydx+mydx<20))) 
					{
					if (myMap[my_level][myx+mydy+mydy][myy+mydx+mydx]==0) 
					{
						PlaySound((PSTR)IDR_WAVE2,hInst,SND_RESOURCE|SND_ASYNC);
						myMap[my_level][myx+mydy+mydy][myy+mydx+mydx]=1;
					}
					else 
					{
						myMap[my_level][myx+mydy+mydy][myy+mydx+mydx]=0;
						PlaySound((PSTR)IDR_WAVE1,hInst,SND_RESOURCE|SND_ASYNC);
						//shake window
						if (shakewindow)
						{
						for (int shake=1; shake<10; shake++)
							{
								MoveWindow(hwnd,winx-5,winy-5,408,476,1);
								MoveWindow(hwnd,winx-5,winy-5,408,476,1);
								MoveWindow(hwnd,winx+5,winy,408,476,1);
								MoveWindow(hwnd,winx+5,winy,408,476,1);
								MoveWindow(hwnd,winx-5,winy+5,408,476,1);
								MoveWindow(hwnd,winx-5,winy+5,408,476,1);
								MoveWindow(hwnd,winx+5,winy,408,476,1);
								MoveWindow(hwnd,winx+5,winy,408,476,1);
								MoveWindow(hwnd,winx+5,winy+5,408,476,1);
								MoveWindow(hwnd,winx+5,winy+5,408,476,1);
								MoveWindow(hwnd,winx-5,winy,408,476,1);
								MoveWindow(hwnd,winx-5,winy,408,476,1);
								MoveWindow(hwnd,winx+5,winy-5,408,476,1);
								MoveWindow(hwnd,winx+5,winy-5,408,476,1);
								MoveWindow(hwnd,winx-5,winy,408,476,1);
								MoveWindow(hwnd,winx-5,winy,408,476,1);
							}
						}
					}
					myMap[my_level][myx+mydy][myy+mydx]=0;
					myx += mydy;
					myy += mydx;
					}
				else if ((mydy != 0 && myMap[my_level][myx+mydy][myy]==2)||(mydx != 0 && myMap[my_level][myx][myy+mydx]==2)) 
					{
						my_level += 1;
						if (my_level<MAX_LEVEL) 
						{
							if (mydy!=0)
							{
								myx=myx>1?1:18;
							}
							else
							{
								myy=myy>1?1:18;
							}
							PlaySound((PSTR)IDR_WAVE3,hInst,SND_RESOURCE|SND_ASYNC);
						} 
						else 
						{
							PlaySound((PSTR)IDR_WAVE3,hInst,SND_RESOURCE|SND_SYNC);
							SendMessage(hwnd,WM_DESTROY,NULL,NULL);
						}
					}
				else if ((mydy != 0 && myMap[my_level][myx+mydy][myy]==5)||(mydx != 0 && myMap[my_level][myx][myy+mydx]==5)) 
					{
						my_level -= 1;
						if (mydy!=0)
						{
							myx=myx>1?1:18;
						}
						else
						{
							myy=myy>1?1:18;
						}
						PlaySound((PSTR)IDR_WAVE3,hInst,SND_RESOURCE|SND_ASYNC);
					}
				if (showmap) showmap = FALSE;
				moveMnstr(myx,myy);
				break;
			case VK_DOWN:
			case VK_NUMPAD2:
				myhits += 2;
				if (myhits > 100) myhits = 100;
				if ((mydy != 0 && !myMap[my_level][myx-mydy][myy])||(mydx != 0 && !myMap[my_level][myx][myy-mydx])) {
					myx -= mydy;
					myy -= mydx;
					};
				if (showmap) showmap = FALSE;
				moveMnstr(myx,myy);
				break;
			case VK_RIGHT:
			case VK_NUMPAD6:
				temp = mydx;
				mydx = -mydy;
				mydy = temp;
				if (showmap) showmap = FALSE;
				break;
			case VK_LEFT:
			case VK_NUMPAD4:
				temp = mydy;
				mydy = -mydx;
				mydx = temp;
				if (showmap) showmap = FALSE;
				break;
			case VK_HOME:
			case VK_NUMPAD5:
				if (difficulty!=HARD_PLAY) showmap = !showmap;
				break;
			case VK_F1:
				shakewindow = !shakewindow;
				showstats = TRUE;
				if (showmap) showmap = FALSE;
				break;
			case VK_F2:
				difficulty = difficulty+1>HARD_PLAY?0:difficulty+1;
				showstats = TRUE;
				if (showmap) showmap = FALSE;
				break;
			case VK_F5:
				saveGame(hwnd);
				strcpy(myMsg,"Current Game Saved");
				showmsg = TRUE;
				break;
			case VK_F6:
				loadGame(hwnd);
				strcpy(myMsg,"Saved Game Loaded");
				showmsg = TRUE;
				break;
			case VK_HELP:
			case VK_ESCAPE:
				showhelp = !showhelp;
				showstats = TRUE;
				break;
			}
			InvalidateRect (hwnd, NULL, 0);
		}
			return 0;

     case WM_DESTROY:
		  DeleteDC(hdcBuffer);
		  DeleteDC(hdcBack);
		  DeleteObject(hbitmap);
		  DeleteObject(hbitBack);
		  DeleteObject(WRbrush);
		  KillTimer (hwnd, BATTLE_TIMER);
          PostQuitMessage (0) ;
          return 0 ;
     }
     return DefWindowProc (hwnd, message, wParam, lParam) ;
}

void scanAround (HDC hdcBuffer, int myx, int myy, int mydx, int mydy)
{
	auto int i;
	auto int dist;
	
	if (mydy != 0)
	{
		if (mydy == 1)
		{dist = 19-myx;}
		else
		{dist = myx;}
		for (i=dist; i>=0; i--)
		{
			drawWalls(hdcBuffer, i, (myx+(i*mydy)), (myy+mydy+mydy), myx, myy, mydx, mydy);
			drawWalls(hdcBuffer, i, (myx+(i*mydy)), (myy+mydy), myx, myy, mydx, mydy);
			drawWalls(hdcBuffer, i, (myx+(i*mydy)), (myy-mydy-mydy), myx, myy, mydx, mydy);
			drawWalls(hdcBuffer, i, (myx+(i*mydy)), (myy-mydy), myx, myy, mydx, mydy);
			drawWalls(hdcBuffer, i, (myx+(i*mydy)), (myy), myx, myy, mydx, mydy);
		}
	}
	else
	{
		if (mydx == 1)
		{dist = 19-myy;}
		else
		{dist = myy;}
		for (i=dist; i>=0; i--)
		{
			drawWalls(hdcBuffer, i, (myx-mydx-mydx), myy+(i*mydx), myx, myy, mydx, mydy);
			drawWalls(hdcBuffer, i, (myx-mydx), myy+(i*mydx), myx, myy, mydx, mydy);
			drawWalls(hdcBuffer, i, (myx+mydx+mydx), (myy+(i*mydx)), myx, myy, mydx, mydy);
			drawWalls(hdcBuffer, i, (myx+mydx), (myy+(i*mydx)), myx, myy, mydx, mydy);
			drawWalls(hdcBuffer, i, (myx), (myy+(i*mydx)), myx, myy, mydx, mydy);
		}
	}
}

void drawWalls (HDC hdcBuffer, int space, int wx, int wy, int myx, int myy, int mydx, int mydy)
{
	auto POINT myWall[4] = {0,0, 0,0, 0,0, 0,0}; 
	auto POINT mySpider[12] = {0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0};
	auto double x1=0,y1=0;
	auto int dx=0, dy=0;
	auto int offset = 0;
	//select brush color
	if (myMap[my_level][wx][wy]==0 || myMap[my_level][wx][wy]==2 || myMap[my_level][wx][wy]==5)
	{
		SelectObject (hdcBuffer, WRbrush[FLOOR][min(space,8)]);
	}
	else if (myMap[my_level][wx][wy]==4)
	{
		SelectObject (hdcBuffer, WRbrush[FLOOR][min(space+3,8)]);
	}
	else if (myMap[my_level][wx][wy]==3 || myMap[my_level][wx][wy]==6)
	{
		SelectObject (hdcBuffer, WRbrush[FLOOR][min(space,8)]);
	}
	else
	{
		SelectObject (hdcBuffer, WRbrush[WALL][min(space,8)]);
	}
	//if facing north or south
	if (mydy != 0)
	{
		dy = myy-wy;
		if (dy > 1) offset = 1;
		if (dy < -1) offset = 1;
		if (dy > 0) dy = 1;
		if (dy < 0) dy = -1;
		if (dy == 0 && myMap[my_level][wx][wy]==1)
		{
			//wall directly infront
			x1 = (EYE_BACK*(WALL_SIZE/2.0))/(EYE_BACK+(WALL_SIZE*space));
			y1 = (EYE_BACK*EYE_LEVEL)/(EYE_BACK+(WALL_SIZE*space));
			myWall[0].x = ORIGIN+(x1*SCREEN_UNIT);
			myWall[0].y = ORIGIN+(y1*SCREEN_UNIT);
			myWall[1].x = myWall[0].x;
			myWall[1].y = ORIGIN-(y1*SCREEN_UNIT);
			myWall[2].x = ORIGIN-(x1*SCREEN_UNIT);
			myWall[2].y = myWall[1].y;
			myWall[3].x = myWall[2].x;
			myWall[3].y = myWall[0].y;
			Polygon (hdcBuffer, myWall, 4);
			//sightBuffer
			if (space<8)
			{
				sightStore(wx,wy,myWall[2].x,myWall[0].x);
			}
			return;
		}
		if (dy == 0 && (myMap[my_level][wx][wy]==0 || myMap[my_level][wx][wy]==2 || myMap[my_level][wx][wy]==5))
		{
			//floor directly infront
			x1 = (EYE_BACK*(WALL_SIZE/2.0))/(EYE_BACK+(WALL_SIZE*space));
			y1 = (EYE_BACK*EYE_LEVEL)/(EYE_BACK+(WALL_SIZE*space));
			myWall[0].x = ORIGIN+(x1*SCREEN_UNIT);
			myWall[0].y = ORIGIN+(y1*SCREEN_UNIT);
			x1 = (EYE_BACK*(WALL_SIZE/2.0))/(EYE_BACK+(WALL_SIZE*(space+1)));
			y1 = (EYE_BACK*EYE_LEVEL)/(EYE_BACK+(WALL_SIZE*(space+1)));
			myWall[1].x = ORIGIN+(x1*SCREEN_UNIT);
			myWall[1].y = ORIGIN+(y1*SCREEN_UNIT);
			x1 = (EYE_BACK*(WALL_SIZE/2.0))/(EYE_BACK+(WALL_SIZE*(space+1)));
			y1 = (EYE_BACK*EYE_LEVEL)/(EYE_BACK+(WALL_SIZE*(space+1)));
			myWall[2].x = ORIGIN-(x1*SCREEN_UNIT);
			myWall[2].y = ORIGIN+(y1*SCREEN_UNIT);
			x1 = (EYE_BACK*(WALL_SIZE/2.0))/(EYE_BACK+(WALL_SIZE*space));
			y1 = (EYE_BACK*EYE_LEVEL)/(EYE_BACK+(WALL_SIZE*space));
			myWall[3].x = ORIGIN-(x1*SCREEN_UNIT);
			myWall[3].y = ORIGIN+(y1*SCREEN_UNIT);
			Polygon (hdcBuffer, myWall, 4);
			return;
		}
		if (dy == 0 && myMap[my_level][wx][wy]==4)
		{
			//pit directly infront
			x1 = (EYE_BACK*(WALL_SIZE/2.0))/(EYE_BACK+(WALL_SIZE*space));
			y1 = (EYE_BACK*EYE_LEVEL)/(EYE_BACK+(WALL_SIZE*space));
			myWall[0].x = ORIGIN+(x1*SCREEN_UNIT);
			myWall[0].y = ORIGIN+(y1*SCREEN_UNIT);
			x1 = (EYE_BACK*(WALL_SIZE/2.0))/(EYE_BACK+(WALL_SIZE*(space+1)));
			y1 = (EYE_BACK*EYE_LEVEL)/(EYE_BACK+(WALL_SIZE*(space+1)));
			myWall[1].x = ORIGIN+(x1*SCREEN_UNIT);
			myWall[1].y = ORIGIN+(y1*SCREEN_UNIT);
			x1 = (EYE_BACK*(WALL_SIZE/2.0))/(EYE_BACK+(WALL_SIZE*(space+1)));
			y1 = (EYE_BACK*EYE_LEVEL)/(EYE_BACK+(WALL_SIZE*(space+1)));
			myWall[2].x = ORIGIN-(x1*SCREEN_UNIT);
			myWall[2].y = ORIGIN+(y1*SCREEN_UNIT);
			x1 = (EYE_BACK*(WALL_SIZE/2.0))/(EYE_BACK+(WALL_SIZE*space));
			y1 = (EYE_BACK*EYE_LEVEL)/(EYE_BACK+(WALL_SIZE*space));
			myWall[3].x = ORIGIN-(x1*SCREEN_UNIT);
			myWall[3].y = ORIGIN+(y1*SCREEN_UNIT);
			Polygon (hdcBuffer, myWall, 4);
			SelectObject (hdcBuffer, WRbrush[FLOOR][min(space+2,8)]);
			Rectangle(hdcBuffer,myWall[2].x,myWall[2].y,myWall[1].x,myWall[0].y+1);
			return;
		}
		if (dy == 0 && (myMap[my_level][wx][wy]==3 || myMap[my_level][wx][wy]==6))
		{
			//Spider
			//floor directly infront
			SelectObject (hdcBuffer, WRbrush[FLOOR][min(space,8)]);
			x1 = (EYE_BACK*(WALL_SIZE/2.0))/(EYE_BACK+(WALL_SIZE*space));
			y1 = (EYE_BACK*EYE_LEVEL)/(EYE_BACK+(WALL_SIZE*space));
			myWall[0].x = ORIGIN+(x1*SCREEN_UNIT);
			myWall[0].y = ORIGIN+(y1*SCREEN_UNIT);
			x1 = (EYE_BACK*(WALL_SIZE/2.0))/(EYE_BACK+(WALL_SIZE*(space+1)));
			y1 = (EYE_BACK*EYE_LEVEL)/(EYE_BACK+(WALL_SIZE*(space+1)));
			myWall[1].x = ORIGIN+(x1*SCREEN_UNIT);
			myWall[1].y = ORIGIN+(y1*SCREEN_UNIT);
			x1 = (EYE_BACK*(WALL_SIZE/2.0))/(EYE_BACK+(WALL_SIZE*(space+1)));
			y1 = (EYE_BACK*EYE_LEVEL)/(EYE_BACK+(WALL_SIZE*(space+1)));
			myWall[2].x = ORIGIN-(x1*SCREEN_UNIT);
			myWall[2].y = ORIGIN+(y1*SCREEN_UNIT);
			x1 = (EYE_BACK*(WALL_SIZE/2.0))/(EYE_BACK+(WALL_SIZE*space));
			y1 = (EYE_BACK*EYE_LEVEL)/(EYE_BACK+(WALL_SIZE*space));
			myWall[3].x = ORIGIN-(x1*SCREEN_UNIT);
			myWall[3].y = ORIGIN+(y1*SCREEN_UNIT);
			Polygon (hdcBuffer, myWall, 4);
			//spider directly infront
			x1 = (EYE_BACK*(WALL_SIZE/2.0))/(EYE_BACK+(WALL_SIZE*(space+.5)));
			y1 = (EYE_BACK*EYE_LEVEL)/(EYE_BACK+(WALL_SIZE*(space+.5)));
			mySpider[0].x = ORIGIN;
			mySpider[0].y = ORIGIN+((y1*2/5)*SCREEN_UNIT);
			mySpider[1].x = ORIGIN-((x1*3/5)*SCREEN_UNIT);
			mySpider[1].y = ORIGIN;
			mySpider[2].x = ORIGIN-((x1*4/5)*SCREEN_UNIT);
			mySpider[2].y = ORIGIN+((y1*4/5)*SCREEN_UNIT);
			mySpider[3].x = ORIGIN-((x1*3/5)*SCREEN_UNIT);
			mySpider[3].y = ORIGIN+(y1*SCREEN_UNIT);
			mySpider[4].x = ORIGIN-((x1*3.5/5)*SCREEN_UNIT);
			mySpider[4].y = ORIGIN+((y1*4/5)*SCREEN_UNIT);
			mySpider[5].x = ORIGIN-((x1*3/5)*SCREEN_UNIT);
			mySpider[5].y = ORIGIN+((y1/10)*SCREEN_UNIT);
			mySpider[6].x = ORIGIN;
			mySpider[6].y = ORIGIN+((y1/2)*SCREEN_UNIT);
			mySpider[7].x = ORIGIN+((x1*3/5)*SCREEN_UNIT);
			mySpider[7].y = ORIGIN+((y1/10)*SCREEN_UNIT);
			mySpider[8].x = ORIGIN+((x1*3.5/5)*SCREEN_UNIT);
			mySpider[8].y = ORIGIN+((y1*4/5)*SCREEN_UNIT);
			mySpider[9].x = ORIGIN+((x1*3/5)*SCREEN_UNIT);
			mySpider[9].y = ORIGIN+(y1*SCREEN_UNIT);
			mySpider[10].x = ORIGIN+((x1*4/5)*SCREEN_UNIT);
			mySpider[10].y = ORIGIN+((y1*4/5)*SCREEN_UNIT);
			mySpider[11].x = ORIGIN+((x1*3/5)*SCREEN_UNIT);
			mySpider[11].y = ORIGIN;
			if (myMap[my_level][wx][wy]==3) SelectObject (hdcBuffer, WRbrush[SPIDER1][min(space+2,8)]);
			else SelectObject (hdcBuffer, WRbrush[SPIDER2][min(space+2,8)]);
			Polygon (hdcBuffer, mySpider, 12);
			if (myMap[my_level][wx][wy]==3) SelectObject (hdcBuffer, WRbrush[SPIDER1][min(space+1,8)]);
			else SelectObject (hdcBuffer, WRbrush[SPIDER2][min(space+1,8)]);
			Ellipse (hdcBuffer, mySpider[0].x-(y1/5)*SCREEN_UNIT, mySpider[0].y-(y1/5)*SCREEN_UNIT, mySpider[0].x+(y1/5)*SCREEN_UNIT, mySpider[0].y+(y1/5)*SCREEN_UNIT);
			return;
		}
	}
	else
	{
		mydy = mydx;
		dy = wx-myx;
		if (dy > 1) offset = 1;
		if (dy < -1) offset = 1;
		if (dy > 0) dy = 1;
		if (dy < 0) dy = -1;
		if (dy == 0 && myMap[my_level][wx][wy]==1)
		{
			//wall directly infront
			x1 = (EYE_BACK*(WALL_SIZE/2.0))/(EYE_BACK+(WALL_SIZE*space));
			y1 = (EYE_BACK*EYE_LEVEL)/(EYE_BACK+(WALL_SIZE*space));
			myWall[0].x = ORIGIN+(x1*SCREEN_UNIT);
			myWall[0].y = ORIGIN+(y1*SCREEN_UNIT);
			myWall[1].x = myWall[0].x;
			myWall[1].y = ORIGIN-(y1*SCREEN_UNIT);
			myWall[2].x = ORIGIN-(x1*SCREEN_UNIT);
			myWall[2].y = myWall[1].y;
			myWall[3].x = myWall[2].x;
			myWall[3].y = myWall[0].y;
			Polygon (hdcBuffer, myWall, 4);
			//sightBuffer
			if (space<8)
			{
				sightStore(wx,wy,myWall[2].x,myWall[0].x);
			}
			return;
		}
		if (dy == 0 && (myMap[my_level][wx][wy]==0 || myMap[my_level][wx][wy]==2 || myMap[my_level][wx][wy]==5))
		{
			//floor directly infront
			x1 = (EYE_BACK*(WALL_SIZE/2.0))/(EYE_BACK+(WALL_SIZE*space));
			y1 = (EYE_BACK*EYE_LEVEL)/(EYE_BACK+(WALL_SIZE*space));
			myWall[0].x = ORIGIN+(x1*SCREEN_UNIT);
			myWall[0].y = ORIGIN+(y1*SCREEN_UNIT);
			x1 = (EYE_BACK*(WALL_SIZE/2.0))/(EYE_BACK+(WALL_SIZE*(space+1)));
			y1 = (EYE_BACK*EYE_LEVEL)/(EYE_BACK+(WALL_SIZE*(space+1)));
			myWall[1].x = ORIGIN+(x1*SCREEN_UNIT);
			myWall[1].y = ORIGIN+(y1*SCREEN_UNIT);
			x1 = (EYE_BACK*(WALL_SIZE/2.0))/(EYE_BACK+(WALL_SIZE*(space+1)));
			y1 = (EYE_BACK*EYE_LEVEL)/(EYE_BACK+(WALL_SIZE*(space+1)));
			myWall[2].x = ORIGIN-(x1*SCREEN_UNIT);
			myWall[2].y = ORIGIN+(y1*SCREEN_UNIT);
			x1 = (EYE_BACK*(WALL_SIZE/2.0))/(EYE_BACK+(WALL_SIZE*space));
			y1 = (EYE_BACK*EYE_LEVEL)/(EYE_BACK+(WALL_SIZE*space));
			myWall[3].x = ORIGIN-(x1*SCREEN_UNIT);
			myWall[3].y = ORIGIN+(y1*SCREEN_UNIT);
			Polygon (hdcBuffer, myWall, 4);
			return;
		}
		if (dy == 0 && myMap[my_level][wx][wy]==4)
		{
			//pit directly infront
			x1 = (EYE_BACK*(WALL_SIZE/2.0))/(EYE_BACK+(WALL_SIZE*space));
			y1 = (EYE_BACK*EYE_LEVEL)/(EYE_BACK+(WALL_SIZE*space));
			myWall[0].x = ORIGIN+(x1*SCREEN_UNIT);
			myWall[0].y = ORIGIN+(y1*SCREEN_UNIT);
			x1 = (EYE_BACK*(WALL_SIZE/2.0))/(EYE_BACK+(WALL_SIZE*(space+1)));
			y1 = (EYE_BACK*EYE_LEVEL)/(EYE_BACK+(WALL_SIZE*(space+1)));
			myWall[1].x = ORIGIN+(x1*SCREEN_UNIT);
			myWall[1].y = ORIGIN+(y1*SCREEN_UNIT);
			x1 = (EYE_BACK*(WALL_SIZE/2.0))/(EYE_BACK+(WALL_SIZE*(space+1)));
			y1 = (EYE_BACK*EYE_LEVEL)/(EYE_BACK+(WALL_SIZE*(space+1)));
			myWall[2].x = ORIGIN-(x1*SCREEN_UNIT);
			myWall[2].y = ORIGIN+(y1*SCREEN_UNIT);
			x1 = (EYE_BACK*(WALL_SIZE/2.0))/(EYE_BACK+(WALL_SIZE*space));
			y1 = (EYE_BACK*EYE_LEVEL)/(EYE_BACK+(WALL_SIZE*space));
			myWall[3].x = ORIGIN-(x1*SCREEN_UNIT);
			myWall[3].y = ORIGIN+(y1*SCREEN_UNIT);
			Polygon (hdcBuffer, myWall, 4);
			SelectObject (hdcBuffer, WRbrush[FLOOR][min(space+2,8)]);
			Rectangle(hdcBuffer,myWall[2].x,myWall[2].y,myWall[1].x,myWall[0].y+1);
			return;
		}
		if (dy == 0 && (myMap[my_level][wx][wy]==3 || myMap[my_level][wx][wy]==6))
		{
			//Spider
			//floor directly infront
			SelectObject (hdcBuffer, WRbrush[FLOOR][min(space,8)]);
			x1 = (EYE_BACK*(WALL_SIZE/2.0))/(EYE_BACK+(WALL_SIZE*space));
			y1 = (EYE_BACK*EYE_LEVEL)/(EYE_BACK+(WALL_SIZE*space));
			myWall[0].x = ORIGIN+(x1*SCREEN_UNIT);
			myWall[0].y = ORIGIN+(y1*SCREEN_UNIT);
			x1 = (EYE_BACK*(WALL_SIZE/2.0))/(EYE_BACK+(WALL_SIZE*(space+1)));
			y1 = (EYE_BACK*EYE_LEVEL)/(EYE_BACK+(WALL_SIZE*(space+1)));
			myWall[1].x = ORIGIN+(x1*SCREEN_UNIT);
			myWall[1].y = ORIGIN+(y1*SCREEN_UNIT);
			x1 = (EYE_BACK*(WALL_SIZE/2.0))/(EYE_BACK+(WALL_SIZE*(space+1)));
			y1 = (EYE_BACK*EYE_LEVEL)/(EYE_BACK+(WALL_SIZE*(space+1)));
			myWall[2].x = ORIGIN-(x1*SCREEN_UNIT);
			myWall[2].y = ORIGIN+(y1*SCREEN_UNIT);
			x1 = (EYE_BACK*(WALL_SIZE/2.0))/(EYE_BACK+(WALL_SIZE*space));
			y1 = (EYE_BACK*EYE_LEVEL)/(EYE_BACK+(WALL_SIZE*space));
			myWall[3].x = ORIGIN-(x1*SCREEN_UNIT);
			myWall[3].y = ORIGIN+(y1*SCREEN_UNIT);
			Polygon (hdcBuffer, myWall, 4);
			//spider directly infront
			x1 = (EYE_BACK*(WALL_SIZE/2.0))/(EYE_BACK+(WALL_SIZE*(space+.5)));
			y1 = (EYE_BACK*EYE_LEVEL)/(EYE_BACK+(WALL_SIZE*(space+.5)));
			mySpider[0].x = ORIGIN;
			mySpider[0].y = ORIGIN+((y1*2/5)*SCREEN_UNIT);
			mySpider[1].x = ORIGIN-((x1*3/5)*SCREEN_UNIT);
			mySpider[1].y = ORIGIN;
			mySpider[2].x = ORIGIN-((x1*4/5)*SCREEN_UNIT);
			mySpider[2].y = ORIGIN+((y1*4/5)*SCREEN_UNIT);
			mySpider[3].x = ORIGIN-((x1*3/5)*SCREEN_UNIT);
			mySpider[3].y = ORIGIN+(y1*SCREEN_UNIT);
			mySpider[4].x = ORIGIN-((x1*3.5/5)*SCREEN_UNIT);
			mySpider[4].y = ORIGIN+((y1*4/5)*SCREEN_UNIT);
			mySpider[5].x = ORIGIN-((x1*3/5)*SCREEN_UNIT);
			mySpider[5].y = ORIGIN+((y1/10)*SCREEN_UNIT);
			mySpider[6].x = ORIGIN;
			mySpider[6].y = ORIGIN+((y1/2)*SCREEN_UNIT);
			mySpider[7].x = ORIGIN+((x1*3/5)*SCREEN_UNIT);
			mySpider[7].y = ORIGIN+((y1/10)*SCREEN_UNIT);
			mySpider[8].x = ORIGIN+((x1*3.5/5)*SCREEN_UNIT);
			mySpider[8].y = ORIGIN+((y1*4/5)*SCREEN_UNIT);
			mySpider[9].x = ORIGIN+((x1*3/5)*SCREEN_UNIT);
			mySpider[9].y = ORIGIN+(y1*SCREEN_UNIT);
			mySpider[10].x = ORIGIN+((x1*4/5)*SCREEN_UNIT);
			mySpider[10].y = ORIGIN+((y1*4/5)*SCREEN_UNIT);
			mySpider[11].x = ORIGIN+((x1*3/5)*SCREEN_UNIT);
			mySpider[11].y = ORIGIN;
			if (myMap[my_level][wx][wy]==3) SelectObject (hdcBuffer, WRbrush[SPIDER1][min(space+2,8)]);
			else SelectObject (hdcBuffer, WRbrush[SPIDER2][min(space+2,8)]);
			Polygon (hdcBuffer, mySpider, 12);
			if (myMap[my_level][wx][wy]==3) SelectObject (hdcBuffer, WRbrush[SPIDER1][min(space+1,8)]);
			else SelectObject (hdcBuffer, WRbrush[SPIDER2][min(space+1,8)]);
			Ellipse (hdcBuffer, mySpider[0].x-(y1/5)*SCREEN_UNIT, mySpider[0].y-(y1/5)*SCREEN_UNIT, mySpider[0].x+(y1/5)*SCREEN_UNIT, mySpider[0].y+(y1/5)*SCREEN_UNIT);
			return;
		}
	}
	//floors to the sides
	x1 = (EYE_BACK*((WALL_SIZE/2.0)+(WALL_SIZE*offset)))/((EYE_BACK+(WALL_SIZE*space))>0?(EYE_BACK+(WALL_SIZE*space)):.1);
	y1 = (EYE_BACK*EYE_LEVEL)/((EYE_BACK+(WALL_SIZE*space))>0?(EYE_BACK+(WALL_SIZE*space)):.1);
	myWall[0].x = ORIGIN+(x1*SCREEN_UNIT*mydy*dy);
	myWall[0].y = ORIGIN+(y1*SCREEN_UNIT);
	x1 = (EYE_BACK*((WALL_SIZE/2.0)+(WALL_SIZE*offset)))/((EYE_BACK+(WALL_SIZE*(space+1)))>0?(EYE_BACK+(WALL_SIZE*(space+1))):.1);
	y1 = (EYE_BACK*EYE_LEVEL)/((EYE_BACK+(WALL_SIZE*(space+1)))>0?(EYE_BACK+(WALL_SIZE*(space+1))):.1);
	myWall[1].x = ORIGIN+(x1*SCREEN_UNIT*mydy*dy);
	myWall[1].y = ORIGIN+(y1*SCREEN_UNIT);
	x1 = (EYE_BACK*((WALL_SIZE/2.0)+(WALL_SIZE*(offset+1))))/((EYE_BACK+(WALL_SIZE*(space+1)))>0?(EYE_BACK+(WALL_SIZE*(space+1))):.1);
	y1 = (EYE_BACK*EYE_LEVEL)/((EYE_BACK+(WALL_SIZE*(space+1)))>0?(EYE_BACK+(WALL_SIZE*(space+1))):.1);
	myWall[2].x = ORIGIN+(x1*SCREEN_UNIT*mydy*dy);
	myWall[2].y = ORIGIN+(y1*SCREEN_UNIT);
	x1 = (EYE_BACK*((WALL_SIZE/2.0)+(WALL_SIZE*(offset+1))))/((EYE_BACK+(WALL_SIZE*space))>0?(EYE_BACK+(WALL_SIZE*space)):.1);
	y1 = (EYE_BACK*EYE_LEVEL)/((EYE_BACK+(WALL_SIZE*space))>0?(EYE_BACK+(WALL_SIZE*space)):.1);
	myWall[3].x = ORIGIN+(x1*SCREEN_UNIT*mydy*dy);
	myWall[3].y = ORIGIN+(y1*SCREEN_UNIT);
	Polygon (hdcBuffer, myWall, 4);
	if (myMap[my_level][wx][wy]==4)
	{
		//pit to the sides
		SelectObject (hdcBuffer, WRbrush[FLOOR][min(space+2,8)]);
		Rectangle(hdcBuffer,myWall[2].x,myWall[2].y,myWall[1].x,myWall[0].y+1);
	}
	if (myMap[my_level][wx][wy]==1)
	{
		//wall front on the sides
		x1 = (EYE_BACK*((WALL_SIZE/2.0)+(WALL_SIZE*offset)))/((EYE_BACK+(WALL_SIZE*space))>0?(EYE_BACK+(WALL_SIZE*space)):.1);
		y1 = (EYE_BACK*EYE_LEVEL)/((EYE_BACK+(WALL_SIZE*space))>0?(EYE_BACK+(WALL_SIZE*space)):.1);
		myWall[0].x = ORIGIN+(x1*SCREEN_UNIT*mydy*dy);
		myWall[0].y = ORIGIN+(y1*SCREEN_UNIT);
		myWall[1].x = myWall[0].x;
		myWall[1].y = ORIGIN-(y1*SCREEN_UNIT);
		x1 = (EYE_BACK*((WALL_SIZE/2.0)+(WALL_SIZE*(offset+1))))/((EYE_BACK+(WALL_SIZE*space))>0?(EYE_BACK+(WALL_SIZE*space)):.1);
		y1 = (EYE_BACK*EYE_LEVEL)/((EYE_BACK+(WALL_SIZE*space))>0?(EYE_BACK+(WALL_SIZE*space)):.1);
		myWall[2].x = ORIGIN+(x1*SCREEN_UNIT*mydy*dy);
		myWall[2].y = ORIGIN-(y1*SCREEN_UNIT);
		myWall[3].x = myWall[2].x;
		myWall[3].y = ORIGIN+(y1*SCREEN_UNIT);
		Polygon (hdcBuffer, myWall, 4);
		//sightBuffer
		if (space<8)
		{
			if (myWall[0].x < myWall[2].x) sightStore(wx,wy,myWall[0].x,myWall[2].x);
			else sightStore(wx,wy,myWall[2].x,myWall[0].x);
		}
		//wall side on the sides
		x1 = (EYE_BACK*((WALL_SIZE/2.0)+WALL_SIZE*offset))/((EYE_BACK+(WALL_SIZE*(space+1.0)))>0?(EYE_BACK+(WALL_SIZE*(space+1.0))):.1);
		y1 = (EYE_BACK*EYE_LEVEL)/((EYE_BACK+(WALL_SIZE*(space+1.0)))>0?(EYE_BACK+(WALL_SIZE*(space+1.0))):.1);
		myWall[2].x = ORIGIN+(x1*SCREEN_UNIT*mydy*dy);
		myWall[2].y = ORIGIN-(y1*SCREEN_UNIT);
		myWall[3].x = myWall[2].x;
		myWall[3].y = ORIGIN+(y1*SCREEN_UNIT);
		Polygon (hdcBuffer, myWall, 4);
		//sightBuffer
		if (space<8)
		{
			if (myWall[0].x < myWall[2].x) sightStore(wx,wy,myWall[0].x,myWall[2].x);
			else sightStore(wx,wy,myWall[2].x,myWall[0].x);
		}
	}
	if (myMap[my_level][wx][wy]==3 || myMap[my_level][wx][wy]==6)
	{
			//Spider
			if (mydx==0) offset = abs(wy-myy);
			else offset = abs(wx-myx);
			x1 = (EYE_BACK*(WALL_SIZE/2.0))/(EYE_BACK+(WALL_SIZE*(space+.5)));
			y1 = (EYE_BACK*EYE_LEVEL)/(EYE_BACK+(WALL_SIZE*(space+.5)));
			mySpider[0].x = ORIGIN+(x1*2*SCREEN_UNIT*mydy*dy*offset);
			mySpider[0].y = ORIGIN+((y1*2/5)*SCREEN_UNIT);
			mySpider[1].x = ORIGIN-((x1*3/5)*SCREEN_UNIT)+(x1*2*SCREEN_UNIT*mydy*dy*offset);
			mySpider[1].y = ORIGIN;
			mySpider[2].x = ORIGIN-((x1*4/5)*SCREEN_UNIT)+(x1*2*SCREEN_UNIT*mydy*dy*offset);
			mySpider[2].y = ORIGIN+((y1*4/5)*SCREEN_UNIT);
			mySpider[3].x = ORIGIN-((x1*3/5)*SCREEN_UNIT)+(x1*2*SCREEN_UNIT*mydy*dy*offset);
			mySpider[3].y = ORIGIN+(y1*SCREEN_UNIT);
			mySpider[4].x = ORIGIN-((x1*3.5/5)*SCREEN_UNIT)+(x1*2*SCREEN_UNIT*mydy*dy*offset);
			mySpider[4].y = ORIGIN+((y1*4/5)*SCREEN_UNIT);
			mySpider[5].x = ORIGIN-((x1*3/5)*SCREEN_UNIT)+(x1*2*SCREEN_UNIT*mydy*dy*offset);
			mySpider[5].y = ORIGIN+((y1/10)*SCREEN_UNIT);
			mySpider[6].x = ORIGIN+(x1*2*SCREEN_UNIT*mydy*dy*offset);
			mySpider[6].y = ORIGIN+((y1/2)*SCREEN_UNIT);
			mySpider[7].x = ORIGIN+((x1*3/5)*SCREEN_UNIT)+(x1*2*SCREEN_UNIT*mydy*dy*offset);
			mySpider[7].y = ORIGIN+((y1/10)*SCREEN_UNIT);
			mySpider[8].x = ORIGIN+((x1*3.5/5)*SCREEN_UNIT)+(x1*2*SCREEN_UNIT*mydy*dy*offset);
			mySpider[8].y = ORIGIN+((y1*4/5)*SCREEN_UNIT);
			mySpider[9].x = ORIGIN+((x1*3/5)*SCREEN_UNIT)+(x1*2*SCREEN_UNIT*mydy*dy*offset);
			mySpider[9].y = ORIGIN+(y1*SCREEN_UNIT);
			mySpider[10].x = ORIGIN+((x1*4/5)*SCREEN_UNIT)+(x1*2*SCREEN_UNIT*mydy*dy*offset);
			mySpider[10].y = ORIGIN+((y1*4/5)*SCREEN_UNIT);
			mySpider[11].x = ORIGIN+((x1*3/5)*SCREEN_UNIT)+(x1*2*SCREEN_UNIT*mydy*dy*offset);
			mySpider[11].y = ORIGIN;
			if (myMap[my_level][wx][wy]==3) SelectObject (hdcBuffer, WRbrush[SPIDER1][min(space+2,8)]);
			else SelectObject (hdcBuffer, WRbrush[SPIDER2][min(space+2,8)]);
			Polygon (hdcBuffer, mySpider, 12);
			if (myMap[my_level][wx][wy]==3) SelectObject (hdcBuffer, WRbrush[SPIDER1][min(space+1,8)]);
			else SelectObject (hdcBuffer, WRbrush[SPIDER2][min(space+1,8)]);
			Ellipse (hdcBuffer, mySpider[0].x-(y1/5)*SCREEN_UNIT, mySpider[0].y-(y1/5)*SCREEN_UNIT, mySpider[0].x+(y1/5)*SCREEN_UNIT, mySpider[0].y+(y1/5)*SCREEN_UNIT);
	}
}

void scanMap (HDC hdcBuffer, int myx, int myy, int mydx, int mydy)
{
	auto int i,j;
	POINT myup [3] = {SCREEN_SIZE/40,0, 0,SCREEN_SIZE/20, SCREEN_SIZE/20,SCREEN_SIZE/20};
	POINT mydown [3] = {0,0, SCREEN_SIZE/20,0, SCREEN_SIZE/40,SCREEN_SIZE/20};
	POINT myright [3] = {0,0, 0,SCREEN_SIZE/20, SCREEN_SIZE/20,SCREEN_SIZE/40};
	POINT myleft [3] = {SCREEN_SIZE/20,0, 0,SCREEN_SIZE/40, SCREEN_SIZE/20,SCREEN_SIZE/20};
	POINT *mypos;

	SelectObject (hdcBuffer, WRbrush[FLOOR][8]);
	Rectangle (hdcBuffer, 0, 0, SCREEN_SIZE, SCREEN_SIZE);
	for (i=0; i<20; i++)
	{
		for (j=0; j<20; j++)
		{
			if (difficulty==EASY_PLAY)
			{
				 drawMap(hdcBuffer, i, j,myMap[my_level][i][j], mydx, mydy);
			}
			else if (miniMap[my_level][i][j]) drawMap(hdcBuffer, i, j,myMap[my_level][i][j], mydx, mydy);
		}
	}
	SelectObject (hdcBuffer, GetStockObject(WHITE_BRUSH));
	if (mydy == -1) mypos = myup;
	else if (mydy == 1) mypos = mydown;
	else if (mydx == -1) mypos = myleft;
	else if (mydx == 1) mypos = myright;
	for (int k=0; k<3; k++)
	{
		mypos[k].x += myy*SCREEN_SIZE/20;
		mypos[k].y += myx*SCREEN_SIZE/20;
	}
	Polygon (hdcBuffer, mypos, 3);
}

void drawMap (HDC hdcBuffer, int x, int y, int block, int mydx, int mydy)
{	
	if (block == 1)	// || block==2
	{
		SelectObject (hdcBuffer, WRbrush[WALL][2]);
		Rectangle (hdcBuffer, (SCREEN_SIZE/20)*y, (SCREEN_SIZE/20)*x, (SCREEN_SIZE/20)+((SCREEN_SIZE/20)*y), (SCREEN_SIZE/20)+((SCREEN_SIZE/20)*x));
	}
	if (block == 4)
	{
		SelectObject (hdcBuffer, WRbrush[FLOOR][6]);
		Rectangle (hdcBuffer, (SCREEN_SIZE/20)*y, (SCREEN_SIZE/20)*x, (SCREEN_SIZE/20)+((SCREEN_SIZE/20)*y), (SCREEN_SIZE/20)+((SCREEN_SIZE/20)*x));
		SelectObject (hdcBuffer, WRbrush[FLOOR][7]);
		Rectangle (hdcBuffer, 3+(SCREEN_SIZE/20)*y, 3+(SCREEN_SIZE/20)*x, (SCREEN_SIZE/20)+((SCREEN_SIZE/20)*y)-3, (SCREEN_SIZE/20)+((SCREEN_SIZE/20)*x)-3);
	}
	if (block == 0 || block == 3 || block == 6)
	{
		SelectObject (hdcBuffer, WRbrush[FLOOR][5]);
		Rectangle (hdcBuffer, (SCREEN_SIZE/20)*y, (SCREEN_SIZE/20)*x, (SCREEN_SIZE/20)+((SCREEN_SIZE/20)*y), (SCREEN_SIZE/20)+((SCREEN_SIZE/20)*x));
	}
	if (block == 3)
	{
		SelectObject (hdcBuffer, WRbrush[SPIDER1][1]);
		Ellipse (hdcBuffer, 3+(SCREEN_SIZE/20)*y, 3+(SCREEN_SIZE/20)*x, (SCREEN_SIZE/20)+((SCREEN_SIZE/20)*y)-3, (SCREEN_SIZE/20)+((SCREEN_SIZE/20)*x)-3);
	}
	if (block == 6)
	{
		SelectObject (hdcBuffer, WRbrush[SPIDER2][1]);
		Ellipse (hdcBuffer, 3+(SCREEN_SIZE/20)*y, 3+(SCREEN_SIZE/20)*x, (SCREEN_SIZE/20)+((SCREEN_SIZE/20)*y)-3, (SCREEN_SIZE/20)+((SCREEN_SIZE/20)*x)-3);
	}
}

void  scanHits(HDC hdcBuffer, int myhits, int crhits)
{
	if (myhits > 0)
	{
		SelectObject (hdcBuffer, GetStockObject(LTGRAY_BRUSH));
		Rectangle (hdcBuffer, 0,SCREEN_SIZE+(50-(myhits/2)), 50,SCREEN_SIZE+50);
	}
	if (crhits > 0)
	{
		SelectObject (hdcBuffer, GetStockObject(DKGRAY_BRUSH));
		Rectangle (hdcBuffer, SCREEN_SIZE-50,SCREEN_SIZE+(50-(crhits/2)), SCREEN_SIZE,SCREEN_SIZE+50);
	}
}

void sightStore (int wx, int wy, int xbegin, int xend)
{
	for (int i=max(xbegin,0); i<=min(xend,399); i++)
	{
		sightBuffer[i].x = wx;
		sightBuffer[i].y = wy;
	}
}

void sightScan(void)
{
	for (int i=0; i<399; i++)
	{
		miniMap[my_level][sightBuffer[i].x][sightBuffer[i].y]=TRUE;
	}
}

void sightWipe(void)
{
	for (int i=0; i<399; i++)
	{
		sightBuffer[i].x=0;
		sightBuffer[i].y=0;
	}
}

void showStats(HDC hdcBuffer)
{
	static RECT rectShake = {55,400,250,450};
	static RECT rectDiff = {55,416,250,450};
	static RECT rectLevel = {55,432,250,450};
	char text[12];
	int textLen;

	SetBkMode (hdcBuffer, TRANSPARENT);
	SetTextColor (hdcBuffer, RGB(0,0,0));

	if (shakewindow) DrawTextEx (hdcBuffer, TEXT("F1: Shake - Yes"), 15, &rectShake, DT_LEFT, NULL);
	else DrawTextEx (hdcBuffer, TEXT("F1: Shake - No"), 14, &rectShake, DT_LEFT, NULL);
	switch (difficulty)
	{
	case EASY_PLAY:
		DrawTextEx (hdcBuffer, TEXT("F2: Difficulty - Wife's Mode"), 28, &rectDiff, DT_LEFT, NULL);
		break;
	case MEDIUM_PLAY:
		DrawTextEx (hdcBuffer, TEXT("F2: Difficulty - Friend's Mode"), 30, &rectDiff, DT_LEFT, NULL);
		break;
	case HARD_PLAY:
		DrawTextEx (hdcBuffer, TEXT("F2: Difficulty - Joby's Mode"), 28, &rectDiff, DT_LEFT, NULL);
		break;
	}
	textLen = wsprintf(text,TEXT("LEVEL: %i"),my_level);
	DrawTextEx (hdcBuffer, text, textLen, &rectLevel, DT_LEFT, NULL);
}

void showMsg(HDC hdcBuffer)
{
	static RECT rectMsg = {0,350,400,400};
	int textLen;
	char text[100];
	textLen = wsprintf(text,myMsg);
	SetBkMode (hdcBuffer, TRANSPARENT);
	SetTextColor (hdcBuffer, RGB(0,0,100));
	DrawTextEx (hdcBuffer, text, textLen, &rectMsg, DT_CENTER, NULL);
}

void showExp(HDC hdcBuffer)
{
	static RECT rectExp = {0,400,400,450};
	int textLen;
	char text[40];
	textLen = wsprintf(text,TEXT("You Gained Experience!\nTotal Exp: %i"),experience*10);
	SetBkMode (hdcBuffer, TRANSPARENT);
	SetTextColor (hdcBuffer, RGB(200,0,0));
	DrawTextEx (hdcBuffer, text, textLen, &rectExp, DT_CENTER, NULL);
}

void showHelp(HDC hdcBuffer)
{
	static RECT rectHelp1 = {5,100,395,395};
	static RECT rectHelp2 = {3,98,393,393};
	static RECT rectHelp3 = {4,99,394,394};
	static char * text = TEXT("WormRaider\n\nWander through the dungeon by using the keys on the NumberPad (8,4,6,2).  You can also view your map by pressing '5' on the NumberPad or the 'Home' key.\n\nYou can push the walls around, if nothing blocks their way.  Pits are too deep and big to jump over... but they seem to be the same size as a block... hint... hint...\n\nESC: Info\nF1: Window Shake\nF2: Difficulty Mode\nF5: Save Game\nF6: Load Saved Game\0");

	SetBkMode (hdcBuffer, TRANSPARENT);
	
	SetTextColor (hdcBuffer, RGB(60,0,0));
	DrawTextEx (hdcBuffer, text, -1, &rectHelp1, DT_CENTER|DT_WORDBREAK, NULL);
	SetTextColor (hdcBuffer, RGB(255,160,160));
	DrawTextEx (hdcBuffer, text, -1, &rectHelp2, DT_CENTER|DT_WORDBREAK, NULL);
	SetTextColor (hdcBuffer, RGB(200,0,0));
	DrawTextEx (hdcBuffer, text, -1, &rectHelp3, DT_CENTER|DT_WORDBREAK, NULL);

	showhelp = FALSE;
}

void moveMnstr(int myx, int myy)
{
	bool attack = false;
	int i,j,dx,dy;
	for (i=1; i<19; i++)
	{
		for (j=1; j<19; j++)
		{
			if (myMap[my_level][i][j]==3)
			{
				dx = myy-j;
				dy = myx-i;
				if (dx>0) dx = 1;
				else if (dx<0) dx = -1;
				if (dy>0) dy = 1;
				else if (dy<0) dy = -1;
				switch(rand()%2)
				{
				case 0:
					if (dy!=0)
					{
						if (myMap[my_level][i+dy][j]==0 && !(i+dy==myx && j==myy))
						{
							myMap[my_level][i+dy][j] = 8;
							myMap[my_level][i][j] = 0;
						}
						if (myMap[my_level][i+dy][j]==0 && (i+dy==myx && j==myy))
						{
							attack = TRUE;
						}
					}
					else if (dx!=0)
					{
						if (myMap[my_level][i][j+dx]==0 && !(i==myx && j+dx==myy))
						{
							myMap[my_level][i][j+dx] = 8;
							myMap[my_level][i][j] = 0;
						}
						if (myMap[my_level][i][j+dx]==0 && (i==myx && j+dx==myy))
						{
							attack = TRUE;
						}
					}
					break;
				case 1:
					if (dx!=0)
					{
						if (myMap[my_level][i][j+dx]==0 && !(i==myx && j+dx==myy))
						{
							myMap[my_level][i][j+dx] = 8;
							myMap[my_level][i][j] = 0;
						}
						if (myMap[my_level][i][j+dx]==0 && (i==myx && j+dx==myy))
						{
							attack = TRUE;
						}
					}
					else if (dy!=0)
					{
						if (myMap[my_level][i+dy][j]==0 && !(i+dy==myx && j==myy))
						{
							myMap[my_level][i+dy][j] = 8;
							myMap[my_level][i][j] = 0;
						}
						if (myMap[my_level][i+dy][j]==0 && (i+dy==myx && j==myy))
						{
							attack = TRUE;
						}
					}
					break;
				}
				if (attack)
				{
					myhits -= 5;
					switch (rand()%4)
					{
					case 0: PlaySound((PSTR)IDR_WAVE4,hInst,SND_RESOURCE|SND_SYNC);break;
					case 1: PlaySound((PSTR)IDR_WAVE5,hInst,SND_RESOURCE|SND_SYNC);break;
					case 2: PlaySound((PSTR)IDR_WAVE6,hInst,SND_RESOURCE|SND_SYNC);break;
					case 3: PlaySound((PSTR)IDR_WAVE7,hInst,SND_RESOURCE|SND_SYNC);break;
					}
					attack = FALSE;
				}
			}
			if (myMap[my_level][i][j]==6)
			{
				dx = myy-j;
				dy = myx-i;
				if (dx>0) dx = 1;
				else if (dx<0) dx = -1;
				if (dy>0) dy = 1;
				else if (dy<0) dy = -1;
				switch(rand()%2)
				{
				case 0:
					if (dy!=0)
					{
						if (myMap[my_level][i+dy][j]==0 && !(i+dy==myx && j==myy))
						{
							myMap[my_level][i+dy][j] = 9;
							myMap[my_level][i][j] = 0;
						}
						if (myMap[my_level][i+dy][j]==0 && (i+dy==myx && j==myy))
						{
							attack = TRUE;
						}
					}
					else if (dx!=0)
					{
						if (myMap[my_level][i][j+dx]==0 && !(i==myx && j+dx==myy))
						{
							myMap[my_level][i][j+dx] = 9;
							myMap[my_level][i][j] = 0;
						}
						if (myMap[my_level][i][j+dx]==0 && (i==myx && j+dx==myy))
						{
							attack = TRUE;
						}
					}
					break;
				case 1:
					if (dx!=0)
					{
						if (myMap[my_level][i][j+dx]==0 && !(i==myx && j+dx==myy))
						{
							myMap[my_level][i][j+dx] = 9;
							myMap[my_level][i][j] = 0;
						}
						if (myMap[my_level][i][j+dx]==0 && (i==myx && j+dx==myy))
						{
							attack = TRUE;
						}
					}
					else if (dy!=0)
					{
						if (myMap[my_level][i+dy][j]==0 && !(i+dy==myx && j==myy))
						{
							myMap[my_level][i+dy][j] = 9;
							myMap[my_level][i][j] = 0;
						}
						if (myMap[my_level][i+dy][j]==0 && (i+dy==myx && j==myy))
						{
							attack = TRUE;
						}
					}
					break;
				}
				if (attack)
				{
					myhits -= 10;
					switch (rand()%4)
					{
					case 0: PlaySound((PSTR)IDR_WAVE4,hInst,SND_RESOURCE|SND_SYNC);break;
					case 1: PlaySound((PSTR)IDR_WAVE5,hInst,SND_RESOURCE|SND_SYNC);break;
					case 2: PlaySound((PSTR)IDR_WAVE6,hInst,SND_RESOURCE|SND_SYNC);break;
					case 3: PlaySound((PSTR)IDR_WAVE7,hInst,SND_RESOURCE|SND_SYNC);break;
					}
					attack = FALSE;
				}
			}
		}
	}
	for (i=1; i<19; i++)
	{
		for (j=1; j<19; j++)
		{
			if (myMap[my_level][i][j]==8) myMap[my_level][i][j]=3;
			if (myMap[my_level][i][j]==9) myMap[my_level][i][j]=6;
		}
	}
}

void startGame(HWND hwnd)
{
	fstream f;
	
	/*f.open(TEXT("wrstart.wr"), ios::in|ios::out|ios::binary);
	if (!f) {DestroyWindow(hwnd);}
	f.write((char *)&myMap,sizeof(myMap));
	f.write((char *)&miniMap,sizeof(miniMap));
	f.write((char *)&my_level,sizeof(my_level));
	f.write((char *)&myhits,sizeof(myhits));
	f.write((char *)&experience,sizeof(experience));
	f.write((char *)&myx,sizeof(myx));
	f.write((char *)&myy,sizeof(myy));
	f.write((char *)&mydx,sizeof(mydx));
	f.write((char *)&mydy,sizeof(mydy));
	f.close();
	f.clear();
	*/

	f.open(TEXT("wrstart.wr"), ios::in|ios::out|ios::binary);
	if (!f) {DestroyWindow(hwnd);}
	f.read((char *)&myMap,sizeof(myMap));
	f.read((char *)&miniMap,sizeof(miniMap));
	f.read((char *)&my_level,sizeof(my_level));
	f.read((char *)&myhits,sizeof(myhits));
	f.read((char *)&experience,sizeof(experience));
	f.read((char *)&myx,sizeof(myx));
	f.read((char *)&myy,sizeof(myy));
	f.read((char *)&mydx,sizeof(mydx));
	f.read((char *)&mydy,sizeof(mydy));
	f.close();
	f.clear();
}

void saveGame(HWND hwnd)
{
	fstream f;
	
	f.open(TEXT("wrsaved.wr"), ios::in|ios::out|ios::binary);
	if (!f) {DestroyWindow(hwnd);}
	f.write((char *)&myMap,sizeof(myMap));
	f.write((char *)&miniMap,sizeof(miniMap));
	f.write((char *)&my_level,sizeof(my_level));
	f.write((char *)&myhits,sizeof(myhits));
	f.write((char *)&experience,sizeof(experience));
	f.write((char *)&myx,sizeof(myx));
	f.write((char *)&myy,sizeof(myy));
	f.write((char *)&mydx,sizeof(mydx));
	f.write((char *)&mydy,sizeof(mydy));
	f.close();
	f.clear();
}

void loadGame(HWND hwnd)
{
	fstream f;
	
	f.open(TEXT("wrsaved.wr"), ios::in|ios::out|ios::binary);
	if (!f) {DestroyWindow(hwnd);}
	f.read((char *)&myMap,sizeof(myMap));
	f.read((char *)&miniMap,sizeof(miniMap));
	f.read((char *)&my_level,sizeof(my_level));
	f.read((char *)&myhits,sizeof(myhits));
	f.read((char *)&experience,sizeof(experience));
	f.read((char *)&myx,sizeof(myx));
	f.read((char *)&myy,sizeof(myy));
	f.read((char *)&mydx,sizeof(mydx));
	f.read((char *)&mydy,sizeof(mydy));
	f.close();
	f.clear();
}