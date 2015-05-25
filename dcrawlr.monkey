'dcrawlr - by Joby Bednar
'Stage 1 (currently) is to convert the Win32 API version from early 2000s, called WormRaider, into MonkeyX
'Stage 2 is to optimize and object orient
'Stage 3 is to build on the features and expand

Import mojo
Import monkey.math

Const MAX_LEVEL:Int = 4
Const WALL_SIZE:Float = 10.0
Const EYE_BACK:Float = 7.5
Const VIEW_SIZE:Float = 6.0
Const EYE_LEVEL:Float = 5.0

Const WALL:Int = 0
Const FLOOR:Int = 1
Const SPIDER1:Int = 2
Const SPIDER2:Int = 3

Global ORIGIN_X:Float 
Global ORIGIN_Y:Float
Global ORIGIN:Float 
Global SCREEN_UNIT_X:Float
Global SCREEN_UNIT_Y:Float
Global SCREEN_UNIT:Float

Class Point
	Field x:Float
	Field y:Float
	
	Method New(_x:Float = 0.0, _y:Float = 0.0)
		x = _x
		y = _y
	End
End

'MAP:
'	0: walk way
'	1: wall
'	2: portal +1 level
'	3: spider 1
'	4: pit
'	5: portal -1 level
'	6: spider 2
'	7: 
'	8: [reserved]
'	9: [reserved]
	
Global myMap:Int[MAX_LEVEL*20*20]
Global miniMap:Bool[MAX_LEVEL*20*20]
Global sightBuffer:Point[DeviceWidth]

Class Dcrawlr Extends App

	Field showmap:Bool = False
	Field showmsg:Bool = False
	Field showexp:Bool = False
	Field incombat:Bool = False
	Field showstats:Bool = False
	Field showhelp:Bool = True

	Field my_level:Int = 0
	Field movetick:Float = 0
	Field experience:Int = 0
	Field myhits:Int = 100
	Field crhits:Int = -1
	
	Field myx:Int = 1
	Field myy:Int = 1
	Field mydx:Int = 0
	Field mydy:Int = 1
	
	Field myMsg:String = ""
	
	Field sky:Image
	Field WRbrush:Int[4*9]

	Method ToARGB:Int(r:Int,g:Int,b:Int,a:Int=255)
		Local argb:Int = 0
		argb = b | g Shl 8 | r Shl 16 | a Shl 24
		Return argb
	End
	
	Method MyMap:Int(_myx:Int, _myy:Int, _lvl:Int=my_level)
		Return myMap[(_lvl*20*20) + (_myx*20) + _myy]
	End
	
	Method SetMyMap(_myx:Int, _myy:Int, _val:Int, _lvl:Int=my_level)
		myMap[(_lvl*20*20) + (_myx*20) + _myy] = _val
	End

	Method OnCreate()
	
		ORIGIN_X = (DeviceWidth/2.0)
		ORIGIN_Y = (DeviceHeight/2.0)
		ORIGIN = (DeviceHeight/2.0) 'TODO
		SCREEN_UNIT_X = (DeviceWidth/VIEW_SIZE)
		SCREEN_UNIT_Y = (DeviceHeight/VIEW_SIZE)
		SCREEN_UNIT = (DeviceHeight/VIEW_SIZE) 'TODO

		SetUpdateRate 30
		
		'init map
		For Local i:Int=0 To MAX_LEVEL*20*20-1
			myMap[i] = 0
		End 
		
		'TODO: Load saved game data
		
		'init minimap
		For Local i:Int=0 To MAX_LEVEL*20*20-1
			miniMap[i] = False
		End
		
		'generate background sky
		sky = CreateImage(DeviceWidth, DeviceHeight/3)
		For Local q:=0 To (DeviceHeight/3)-1
			Local skyPixels:Int[DeviceWidth]
			For Local p:=0 To DeviceWidth-1
				skyPixels[p] = ToARGB( Max(Int(80.0*(1.0-(q/(DeviceHeight/3.0)))),0), Max(Int(80.0*(1.0-(q/(DeviceHeight/3.0)))),0), Max(Int(170.0*(1.0-(q/(DeviceHeight/3.0)))),0) )
			End
			sky.WritePixels(skyPixels, 0,q, DeviceWidth,1)
		End
		
		'generate brush colors
		WRbrush[WALL*8 + 8] = ToARGB(0,0,0)
		WRbrush[WALL*8 + 7] = ToARGB(32,32,0)
		WRbrush[WALL*8 + 6] = ToARGB(64,64,25)
		WRbrush[WALL*8 + 5] = ToARGB(96,96,57)
		WRbrush[WALL*8 + 4] = ToARGB(128,128,89)
		WRbrush[WALL*8 + 3] = ToARGB(160,160,121)
		WRbrush[WALL*8 + 2] = ToARGB(192,192,153)
		WRbrush[WALL*8 + 1] = ToARGB(224,224,185)
		WRbrush[WALL*8 + 0] = ToARGB(224,224,185)
		  
		WRbrush[FLOOR*8 + 8] = ToARGB(0,0,0)
		WRbrush[FLOOR*8 + 7] = ToARGB(32,32,0)
		WRbrush[FLOOR*8 + 6] = ToARGB(64,64,0)
		WRbrush[FLOOR*8 + 5] = ToARGB(96,96,32)
		WRbrush[FLOOR*8 + 4] = ToARGB(128,128,64)
		WRbrush[FLOOR*8 + 3] = ToARGB(160,160,96)
		WRbrush[FLOOR*8 + 2] = ToARGB(192,192,128)
		WRbrush[FLOOR*8 + 1] = ToARGB(224,224,160)
		WRbrush[FLOOR*8 + 0] = ToARGB(224,224,160)
		  
		WRbrush[SPIDER1*8 + 8] = ToARGB(0,0,0)
		WRbrush[SPIDER1*8 + 7] = ToARGB(32,52,0)
		WRbrush[SPIDER1*8 + 6] = ToARGB(64,84,0)
		WRbrush[SPIDER1*8 + 5] = ToARGB(96,116,32)
		WRbrush[SPIDER1*8 + 4] = ToARGB(128,148,64)
		WRbrush[SPIDER1*8 + 3] = ToARGB(160,180,96)
		WRbrush[SPIDER1*8 + 2] = ToARGB(192,212,128)
		WRbrush[SPIDER1*8 + 1] = ToARGB(224,244,160)
		WRbrush[SPIDER1*8 + 0] = ToARGB(224,244,160)
		
		WRbrush[SPIDER2*8 + 8] = ToARGB(0,0,0)
		WRbrush[SPIDER2*8 + 7] = ToARGB(52,32,0)
		WRbrush[SPIDER2*8 + 6] = ToARGB(84,64,0)
		WRbrush[SPIDER2*8 + 5] = ToARGB(116,96,32)
		WRbrush[SPIDER2*8 + 4] = ToARGB(148,128,64)
		WRbrush[SPIDER2*8 + 3] = ToARGB(180,160,96)
		WRbrush[SPIDER2*8 + 2] = ToARGB(212,192,128)
		WRbrush[SPIDER2*8 + 1] = ToARGB(244,224,160)
		WRbrush[SPIDER2*8 + 0] = ToARGB(244,224,160)
		
	End

	Method OnUpdate()
		miniMap[(my_level*20*20)+(myx*20)+myy] = True
		miniMap[(my_level*20*20)+((myx+mydy)*20)+myy+mydx] = True
		miniMap[(my_level*20*20)+((myx+mydx)*20)+myy+mydy] = True
		miniMap[(my_level*20*20)+((myx-mydx)*20)+myy-mydy] = True
		
		If MyMap(myx+mydy, myy+mydx) = 3 Or MyMap(myx+mydy, myy+mydx) = 6
			incombat = True
		End
		
		If Not incombat
			Local keypressed = False
			
			If KeyHit(KEY_UP)
				myhits = Min(myhits+2, 100)
				If (Not mydy = 0 And MyMap(myx+mydy, myy) = 0) Or (Not mydx = 0 And MyMap(myx, myy+mydx) = 0)
					myx += mydy
					myy += mydx
				Elseif ((Not mydy = 0 And MyMap(myx+mydy, myy) = 1 And (MyMap(myx+mydy+mydy,myy) = 0 Or MyMap(myx+mydy+mydy, myy) = 4) And (myx+mydy+mydy > 0 And myx+mydy+mydy < 20)) Or (Not mydx = 0 And MyMap(myx, myy+mydx) = 1 And (MyMap(myx, myy+mydx+mydx) = 0 Or MyMap(myx, myy+mydx+mydx) = 4) And (myy+mydx+mydx > 0 And myy+mydx+mydx < 20))) 
					If MyMap(myx+mydy+mydy, myy+mydx+mydx) = 0
						'TODO: play wall slide sound
						SetMyMap(myx+mydy+mydy, myy+mydx+mydx, 1)
					Else
						'TODO: play wall drop sound
						SetMyMap(myx+mydy+mydy, myy+mydx+mydx, 0)
						'TODO: shake window?
					End
					SetMyMap(myx+mydy, myy+mydx, 0)
					myx += mydy
					myy += mydx
				Elseif ((Not mydy = 0 And MyMap(myx+mydy, myy) = 2) Or (Not mydx = 0 And MyMap(myx, myy+mydx) = 2))
					my_level += 1
					If my_level < MAX_LEVEL
						If Not mydy = 0
							If myx > 1 Then myx = 1 Else myx = 18
						Else
							If myy > 1 Then myy = 1 Else myy = 18
						End
						'TODO: play level teleport sound
					Else
						'TODO: play level teleport sound
						'TODO: win game
					End
				Elseif ((Not mydy = 0 And MyMap(myx+mydy, myy) = 5) Or (Not mydx = 0 And MyMap(myx, myy+mydx) = 5))
					my_level -= 1
					If Not mydy = 0
						If myx > 1 Then myx = 1 Else myx = 18
					Else
						If myy > 1 Then myy = 1 Else myy = 18
					End
					'TODO: play level teleport sound
				End
				keypressed = True
				
			Elseif KeyHit(KEY_DOWN)
				myhits = Max(myhits+2, 100)
				If ((Not mydy = 0 And MyMap(myx-mydy, myy) = 0) Or (Not mydx = 0 And MyMap(myx, myy-mydx) = 0))
					myx -= mydy
					myy -= mydx
				End
				keypressed = True
				
			Elseif KeyHit(KEY_RIGHT)
				Local temp:Int = mydx
				mydx = -mydy
				mydy = temp
				showmap = False
			
			Elseif KeyHit(KEY_LEFT)
				Local temp:Int = mydy
				mydy = -mydx
				mydx = temp
				showmap = False
				
			Elseif KeyHit(KEY_M)	
				showmap = Not showmap
				
			End
				
			If keypressed
				showmap = False
				showexp = False
				showmsg = False
				showstats = False
				MoveMnstr(myx, myy)
			End
		End
	End
	
	Method OnRender()
		Cls
		DrawImage(sky, 0,0)
		
		ScanAround()
		
		SightScan()
		
		If showmap = True
			ScanMap()
		End
		
		ScanHits()
		
		If showstats = True
			ShowStats()
		End
		
		If showhelp = True
			ShowHelp()
		End
		
		If showexp = True
			ShowExp()
		End
		
		If showmsg = True
			ShowMsg()
		End
		
		SightWipe()
	End
	
	Method ScanAround()
		Local i:Int = 0
		Local dist:Int = 0
		
		If Not mydy = 0
			If mydy = 1
				dist = 19-myx
			Else
				dist = myx
			End
			For i = dist To 0 Step -1
				DrawWalls(i, (myx+(i*mydy)), (myy+mydy+mydy), myx, myy, mydx, mydy)
				DrawWalls(i, (myx+(i*mydy)), (myy+mydy), myx, myy, mydx, mydy)
				DrawWalls(i, (myx+(i*mydy)), (myy-mydy-mydy), myx, myy, mydx, mydy)
				DrawWalls(i, (myx+(i*mydy)), (myy-mydy), myx, myy, mydx, mydy)
				DrawWalls(i, (myx+(i*mydy)), (myy), myx, myy, mydx, mydy)
			End
		Else
			If mydx = 1
				dist = 19-myy
			Else
				dist = myy
			End
			For i = dist To 0 Step -1
				DrawWalls(i, (myx-mydx-mydx), myy+(i*mydx), myx, myy, mydx, mydy)
				DrawWalls(i, (myx-mydx), myy+(i*mydx), myx, myy, mydx, mydy)
				DrawWalls(i, (myx+mydx+mydx), (myy+(i*mydx)), myx, myy, mydx, mydy)
				DrawWalls(i, (myx+mydx), (myy+(i*mydx)), myx, myy, mydx, mydy)
				DrawWalls(i, (myx), (myy+(i*mydx)), myx, myy, mydx, mydy)
			End
		End
		
	End
	
	Method SightScan()
	
	End
	
	Method SightStore(_x:Int, _y:Int, _x1:Float, _x2:Float)
	
	End
	
	Method ScanMap()
	
	End
	
	Method ScanHits()
	
	End
	
	Method ShowStats()
	
	End
	
	Method ShowHelp()
	
	End
	
	Method ShowExp()
	
	End
	
	Method ShowMsg()
	
	End
	
	Method SightWipe()
	
	End
	
	Method MoveMnstr(_x:Int, _y:Int)
	
	End
	
	Method Polygon(points:Point[])
		Local poly:Float[8]
		poly[0] = points[0].x
		poly[1] = points[0].y
		poly[2] = points[1].x
		poly[3] = points[1].y
		poly[4] = points[2].x
		poly[5] = points[2].y
		poly[6] = points[3].x
		poly[7] = points[3].y
		DrawPoly(poly)
	End
	
	Method DrawWalls(_space:Int, _wx:Int, _wy:Int, _myx:Int, _myy:Int, _mydx:Int, _mydy:Int)
		Local myWall:Point[4]
		Local mySpider:Point[12]
		Local x1:Float = 0
		Local y1:Float = 0
		Local dx:Int = 0
		Local dy:Int = 0
		Local offset:Int = 0
		Local myColor:Int = 0
		
		'select brush color
		If MyMap(_wx, _wy) = 0 Or MyMap(_wx, _wy) = 2 Or MyMap(_wx, _wy) = 5
			myColor = WRbrush[FLOOR*8 + Min(_space,8)]
		Elseif MyMap(_wx, _wy) = 4
			myColor = WRbrush[FLOOR*8 + Min(_space+3,8)]
		Elseif MyMap(_wx, _wy) = 3 Or MyMap(_wx, _wy) = 6
			myColor = WRbrush[FLOOR*8 + Min(_space,8)]
		Else
			myColor = WRbrush[WALL*8 + Min(_space,8)]
		End
		SetColor((myColor Shr 16) & 255 , (myColor Shr 8) & 255, myColor & 255)
		
		'if facing north or south
		If Not mydy = 0
			dy = _myy - _wy
			If dy > 1 Then offset = 1
			If dy < -1 Then offset = 1
			If dy > 0 Then dy = 1
			If dy < 0 Then dy = -1
			
			If dy = 0 And MyMap(_wx, _wy) = 1
				'wall directly in front
				x1 = (EYE_BACK*(WALL_SIZE/2.0))/(EYE_BACK+(WALL_SIZE*_space*1.0))
				y1 = (EYE_BACK*EYE_LEVEL*1.0)/(EYE_BACK+(WALL_SIZE*_space*1.0))
				myWall[0] = New Point(ORIGIN+(x1*SCREEN_UNIT), ORIGIN+(y1*SCREEN_UNIT))
				myWall[1] = New Point(myWall[0].x, ORIGIN-(y1*SCREEN_UNIT))
				myWall[2] = New Point(ORIGIN-(x1*SCREEN_UNIT), myWall[1].y)
				myWall[3] = New Point(myWall[2].x, myWall[0].y)
				Polygon(myWall)
				'sight buffer
				If _space < 8
					SightStore(_wx, _wy, myWall[2].x, myWall[0].x)
				End
				Return
			End
			
			If dy = 0 And (MyMap(_wx, _wy) = 0 Or MyMap(_wx, _wy) = 2 Or MyMap(_wx, _wy) = 5)
				'floor directly in front
				x1 = (EYE_BACK*(WALL_SIZE/2.0))/(EYE_BACK+(WALL_SIZE*_space*1.0))
				y1 = (EYE_BACK*EYE_LEVEL*1.0)/(EYE_BACK+(WALL_SIZE*_space*1.0))
				myWall[0] = New Point(ORIGIN+(x1*SCREEN_UNIT), ORIGIN+(y1*SCREEN_UNIT))
				x1 = (EYE_BACK*(WALL_SIZE/2.0))/(EYE_BACK+(WALL_SIZE*(_space+1)))
				y1 = (EYE_BACK*EYE_LEVEL*1.0)/(EYE_BACK+(WALL_SIZE*(_space+1.0)))
				myWall[1] = New Point(ORIGIN+(x1*SCREEN_UNIT), ORIGIN+(y1*SCREEN_UNIT))
				x1 = (EYE_BACK*(WALL_SIZE/2.0))/(EYE_BACK+(WALL_SIZE*(_space+1.0)))
				y1 = (EYE_BACK*EYE_LEVEL*1.0)/(EYE_BACK+(WALL_SIZE*(_space+1.0)))
				myWall[2] = New Point(ORIGIN-(x1*SCREEN_UNIT), ORIGIN+(y1*SCREEN_UNIT))
				x1 = (EYE_BACK*(WALL_SIZE/2.0))/(EYE_BACK+(WALL_SIZE*_space*1.0))
				y1 = (EYE_BACK*EYE_LEVEL*1.0)/(EYE_BACK+(WALL_SIZE*_space*1.0))
				myWall[3] = New Point(ORIGIN-(x1*SCREEN_UNIT), ORIGIN+(y1*SCREEN_UNIT))
				Polygon (myWall)
				Return
			End
			
		Else
		
		End
	End
	
End

Function Main()
    New Dcrawlr()
End