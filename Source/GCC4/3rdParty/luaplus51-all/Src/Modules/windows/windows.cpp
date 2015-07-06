#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern "C" {
#include <lua.h>
#include <lauxlib.h>
}
#include "KeystrokeEngine.h"

static int l_GetForegroundWindow(lua_State* L) {
	HWND hwnd = ::GetForegroundWindow();
	lua_pushinteger(L, (UINT)hwnd);
	return 1;
}


static int l_GetActiveWindow(lua_State* L) {
	HWND hwnd = ::GetActiveWindow();
	lua_pushinteger(L, (UINT)hwnd);
	return 1;
}


static int l_GetFocus(lua_State* L) {
	HWND hwnd = ::GetFocus();
	lua_pushinteger(L, (UINT)hwnd);
	return 1;
}


static int l_GetWindowText(lua_State* L) {
	if (!lua_isnumber(L, 1)) {
		lua_pushstring(L, "");
		return 1;
	}

	HWND hwnd = (HWND)(UINT)lua_tonumber(L, 1);

	// ::GetWindowText does not always seem to work
	CString sText;

	char buffer[257];
	buffer[0] = 0;
	if (hwnd != 0)
	{
		::SendMessage(hwnd, WM_GETTEXT, 256, (LPARAM)(LPCTSTR)buffer);
	}

	lua_pushstring(L, buffer);
	return 1;
}


static int l_FindWindowEx(lua_State* L)
{
	if (!lua_isnumber(L, 1)) {
		lua_pushnumber(L, 0);
		return 1;
	}

	HWND hwnd = (HWND)(UINT)lua_tonumber(L, 1);
	HWND hwndChildAfter = (HWND)luaL_optinteger(L, 2, 0);
	LPCSTR className = luaL_optstring(L, 3, NULL);
	LPCSTR windowName = luaL_optstring(L, 4, NULL);
	HWND foundHwnd = FindWindowEx(hwnd, hwndChildAfter, className, windowName);
	lua_pushinteger(L, (UINT)foundHwnd);
	return 1;
}














#if 0



#include "PushKeys.h"
#pragma package(smart_init)

//common to pushkeys & TPushKeys
void DoPushKeys(class TPushKeys* TPK,CString k);
void DoDelay(int mS);
void ActivateTarget();
HWND GTargetWindow;
bool GTrackTarget;

//---------------------------------------------------------------------------
//constructor
__fastcall TPushKeys::TPushKeys(TComponent* Owner)
: TComponent(Owner)
{
	//initial values
	fKeys="";
	fWindowTitle="";
	fWinMatch=wmExactMatch;
	fDelay=0;
	fIsPushing=false;
	fUseANSI=false;
	fReturnFocus=false;
	fTrackTarget=true;
	TargetWindow=NULL;

}

//---------------------------------------------------------------------------
// activate target window
void ActivateTarget()
{

	if (GTargetWindow!=GetForegroundWindow() && GTrackTarget)
	{
		if (IsWindow(GTargetWindow) && GTargetWindow!=NULL)
		{
			SetForegroundWindow(GTargetWindow);
		}
	}
}

//---------------------------------------------------------------------------
//process individual OnKey events passed from pushkeys
//and default key delay
void TPushKeys::HandleOnKeyEvent(CString &Key,TShiftState &ShiftState)
{

	//pass keys to users OnKey event handler
	if (OnKey)
	{
		OnKey(this,Key,ShiftState);
	}
}
//---------------------------------------------------------------------------
//do inter-key delay
void TPushKeys::DoKeyDelay()
{
	if (Process==NULL)
	{
		//timed delay
		DoDelay(KeyDelayValue);
	}
	else //wait for input buffer to empty from process passed to onpus event
	{
		WaitForInputIdle(Process,ProcessWait);
	}

}

//---------------------------------------------------------------------------
void __fastcall TPushKeys::SetKeys(CString keys)
{
	fKeys=keys;
}

//---------------------------------------------------------------------------
//pass string in function call
HWND __fastcall TPushKeys::Push(CString k)
{
	return PushMethod(k);
}
//---------------------------------------------------------------------------
//normal call using Keys property
HWND __fastcall TPushKeys::Push()
{
	return PushMethod(fKeys);
}

//---------------------------------------------------------------------------
//method to simulate key presses
//returns false if window title contained a value but window could not be found
HWND __fastcall TPushKeys::PushMethod(CString k)
{
	//active flag
	fIsPushing=true;

	//calculate default key delay value
	KeyDelayValue=0;
	if (fDelay!=0)
	{
		//value specified
		if (fDelay>0)
		{
			KeyDelayValue=fDelay;
		}
		else //negative==use keyboard delay value
		{
			//get value
			DWORD kbs;
			SystemParametersInfo(SPI_GETKEYBOARDSPEED,0,&kbs,0);

			//i think this is how it works
			kbs+=2;
			KeyDelayValue=1000/kbs;

		}

	}

	//no title specified?
	if (fWindowTitle=="")
	{
		TargetWindow=GetForegroundWindow();
	}
	else
	{
		//enum windows & match window title
		FindWindowTitle();
	}


	//pass window handle, activation flag, Process handle & wait time to OnPush event handler
	bool Activate=true;
	//default process handle & wait time
	Process=NULL;
	ProcessWait=0;

	if (OnPush)
	{
		OnPush(this,TargetWindow,Activate,Process,ProcessWait);
	}


	//ok to push flag
	bool DoPushing=true;

	//set target window as active?
	if (Activate)
	{
		//activate window if valid
		if (TargetWindow!=NULL && IsWindow(TargetWindow))
		{
			SetForegroundWindow(TargetWindow);
		}
		else //no pushing to be done
		{
			TargetWindow=NULL;
			DoPushing=false;
		}
	}

	//global target
	GTargetWindow=TargetWindow;
	GTrackTarget=Activate ? fTrackTarget : false;

	//ok to push?
	if (DoPushing)
	{
		//push the keys
		DoPushKeys(this,k);
	}

	//return focus?
	if (fReturnFocus)
	{
		//give time for key presses to complete
		DoDelay(100);

		HWND ThisWindow=((TForm*)(this->Owner))->Handle;
		if (IsWindow(ThisWindow) && ThisWindow!=NULL)
		{
			SetForegroundWindow(ThisWindow);
		}
	}

	//all done
	fIsPushing=false;

	return TargetWindow;


}
//---------------------------------------------------------------------------
//find match for window title
void TPushKeys::FindWindowTitle()
{
	TargetWindow=NULL;

	EnumWindows((WNDENUMPROC)TPKListWindows,(long)this);

}
//---------------------------------------------------------------------------
//enum windows proc used to step through window titles
BOOL __stdcall TPKListWindows(HWND hWnd, LPARAM lparam)
{

	TPushKeys* caller=(TPushKeys*)lparam;

	//get window title
	char buf[255];
	GetWindowText(hWnd,buf,255);

	CString srch(buf);

	if (lstrlen(buf)==0)
	{
		return true;
	}


	//do match
	switch(caller->fWinMatch)
	{
		//exact match required (case sensitive)
	case wmExactMatch:
		if (srch==caller->fWindowTitle)
		{
			caller->TargetWindow=hWnd;
			return false;
		}
		break;

		//only starting characters need to match (case insensitive)
	case wmStartMatch:
		if (srch.UpperCase().Pos(caller->fWindowTitle.UpperCase())==1)
		{
			caller->TargetWindow=hWnd;
			return false;
		}
		break;

		//any sub string ok (case insensitive)
	case wmPartialMatch:
		if (srch.UpperCase().Pos(caller->fWindowTitle.UpperCase())!=0)
		{
			caller->TargetWindow=hWnd;
			return false;
		}
		break;
	}


	//continue looking
	return true;

}
#endif 0
#if 0
//---------------------------------------------------------------------------
//***************************************************************************
//                      PushKeys Code                                       *
//***************************************************************************
//---------------------------------------------------------------------------

// the following functions are used internally only
void PushAKey(CHAR ch);
void PushFnKey(CString KeyCode);
void PushShiftKey(CHAR ch,bool Off=true);
void PushCTRLKey(char key,bool Off=true);
void PushALTKey(char key,bool Off=true);
void PushRALTKey(char key,bool Off=true);
void PressKey(BYTE Vk, UINT Scan);
void __PushKeys(CString src);
void CTRLOn();
void CTRLOff();
void ALTOn();
void ALTOff();
void RALTOn();
void RALTOff();
void SHIFTOn();
void SHIFTOff();
void AllOff();
bool IsAFunctionKey(CString fk);
byte GetFunctionKey(CString fk);
byte GetControlKey(CString fk);
int GetFunctionKeyCount(CString &fk);
bool ProcessFuncKey(CString k);
void SetKeyStates(TShiftState ss);
void PushKeys(CString src);
void PushDOSKeys(CString src);
void OnKeyHandler(bool IsFnKey,CString Key);
void InitialKeyState();
void PushAString(CString s);

//---------------------------------------------------------------------------

//PUSHKEYS for C++ Builder (component)

//original version 1.0 for Visual Basic
//Copyright by Data Solutions Pty Ltd (ACN 010 951 498)
//All rights reserved.
//Email for info: clatta@ozemail.com.au

//This is the C++Builder component implementation

//C++ component modifications and additions by Alan Warriner
//alan.warriner@bigfoot.com


#define VK_BREAK    VK_CANCEL
#define VK_BELL     0x07
#define VK_LINEFEED 0x0A

// These variables indicate whether these keys are pressed
bool AltOn,RAltOn, ShiftOn, ControlOn;
//ANSI key equivalents flag
bool DOSKey;
//instance calling these routines
TPushKeys* GTPK;

//-----------------------------------------------------------
//entry point for component
void DoPushKeys(TPushKeys* TPK,CString keys)
{
	//set DOS flag
	DOSKey=TPK->UseANSI;

	//set global access
	GTPK=TPK;

	//extract {}} }}} {{{ {} 4} type thing
	bool lb=false, rb=false;
	CString tv="";
	int lastbrace=-1;
	int cnt=0;
	for (int i=1;i<=keys.GetLength();i++)
	{
		char sc=keys[i];

		if (sc=='{')
		{
			//if processed left brace last then delete
			if (!lb)
			{
				lb=true;
				rb=false;
				tv+=sc;
				cnt++;
				lastbrace=cnt;
			}
		}
		else if (sc=='}')
		{
			//ditto for right brace (or left brace not previous)
			if (!rb && lb && cnt-lastbrace>1)
			{
				rb=true;
				lb=false;
				tv+=sc;
				cnt++;
			}
		}
		else
		{
			tv+=sc;
			cnt++;
		}
	}


	//orphan {
	if (lb)
	{
		tv.Delete(lastbrace,1);
	}

	//delete trailing
	int tvl=tv.GetLength();
	if (tv>0)
	{
		if (tv[tvl]=='{')
		{
			tv.Delete(tvl,1);
		}
	}

	keys=tv;

	//call appropriate routine
	if (DOSKey)
	{
		PushDOSKeys(keys);
	}
	else
	{
		PushKeys(keys);
	}
}

//-----------------------------------------------------------
//friend of TPK on key handler
//extract key string and pass to handler
//process keypushes etc on return
void OnKeyHandler(bool IsFnKey,CString Key)
{
	CString k=Key;
	CString uk=k.UpperCase();

	int count;
	bool CallKeyHandler=false;

	//delay
	bool IsSleep=uk.Pos("SLEEP")==1;

	//*********************
	//clear clipboard
	bool IsClearClip=uk.Pos("EMPTYCLIP")==1;

	//run
	bool IsRun=uk.Pos("RUN ")==1;

	//{\0xxx}
	bool IsDirect=k.Pos("\\")==1 && k.GetLength()>1;

	if (k=="\\")
	{
		IsDirect=false;
		IsFnKey=false;
	}

	//function key? or clear clipboard or run
	if(IsFnKey || IsSleep || IsDirect  || IsClearClip || IsRun)
	{
		//get count value & strip out count characters
		if (!IsRun)
		{
			count=GetFunctionKeyCount(k);
		}

		if(IsAFunctionKey(uk) || IsDirect)
		{
			//not a control key?
			if (GetControlKey(uk)==(byte)-1)
				k="{"+uk+"}";

			CallKeyHandler=true;

		}

		//sleep or clear clipboard or run
		if (IsSleep || IsClearClip || IsRun)
		{
			k="{"+Key.UpperCase()+"}";
			CallKeyHandler=true;
		}
	}
	else
	{
		if (k.GetLength()==1)
		{
			CallKeyHandler=true;

		}
	}

	if (!CallKeyHandler)
		return;


	//set up shiftstate to pass here
	//**************************
	TShiftState KeyState,CopyOfKeyState;
	if (ShiftOn)
		KeyState<<ssShift;

	if (ControlOn)
		KeyState<<ssCtrl;

	if (AltOn)
		KeyState<<ssAlt;

	//keep a copy
	CopyOfKeyState=KeyState;

	//pass to onkey & delay handler
	//************************
	GTPK->HandleOnKeyEvent(k,KeyState);

	//returned keys
	//************************
	CString returned=k;
	int rl=returned.GetLength();
	uk=returned.UpperCase();

	if (rl==0)
		return;

	//process returned shiftstate
	//**************
	//changed?
	if (KeyState!=CopyOfKeyState)
	{
		//set key states
		SetKeyStates(KeyState);
	}

	//process keys returned
	//***************************
	IsSleep=uk.Pos("{SLEEP")==1;

	//clear clipboard
	IsClearClip=uk.Pos("{EMPTYCLIP")==1;

	//run
	IsRun=uk.Pos("{RUN ")==1;

	//single key(s)?
	if (rl<3)
	{
		PushAString(returned);
	}
	else
	{
		if (returned[1]=='{' && returned[rl]=='}')
		{
			//strip out braces
			CString fk=returned.SubString(2,rl-2).UpperCase();

			bool IsNL=k=="{NEWLINE}" || uk=="{NL}";

			//is it a function key?
			if(IsAFunctionKey(fk) && !IsNL)
			{
				PushFnKey(fk);
			}
			//CRLF
			else if (IsNL)
			{
				// New line = Carriage return & Line Feed = ^M^J
				for (int i=0;i<count;i++)
				{
					if (DOSKey)//ANSI equivalent
					{
						PushCTRLKey('M');
						PushCTRLKey('J');
					}
					else
					{
						UINT ScanKey = MapVirtualKey(VK_RETURN, 0);
						PressKey(VK_RETURN,ScanKey);
						ScanKey = MapVirtualKey(VK_LINEFEED, 0);
						PressKey(VK_LINEFEED,ScanKey);
					}
				}
			}
			//direct {\xxxx}
			else if(fk.Pos("\\")==1 && fk.GetLength()>1)
			{
				if (fk.GetLength()>1)
				{
					//start key presses
					ALTOn();

					//step along numbers
					for (int numpointer=2;numpointer<=fk.GetLength();numpointer++)
					{
						char number=fk[numpointer];

						if (number>='0' && number<='9')
						{
							//get numpad key
							byte npk[]={VK_NUMPAD0,VK_NUMPAD1,VK_NUMPAD2,VK_NUMPAD3,VK_NUMPAD4
								,VK_NUMPAD5,VK_NUMPAD6,VK_NUMPAD7,VK_NUMPAD8,VK_NUMPAD9};

							byte numpadkey=npk[number-'0'];

							//press key
							PressKey(numpadkey,MapVirtualKey(numpadkey,0));
						}

					}

					//all done
					ALTOff();
				}
			}
			//delay
			else if(IsSleep)//sleep
			{
				int count=GetFunctionKeyCount(fk);
				DoDelay(count);
			}
			//clear clipboard
			else if (IsClearClip)
			{
				int count=GetFunctionKeyCount(fk);
				for (int c=0;c<count;c++)
				{
					if (OpenClipboard(NULL))
					{
						EmptyClipboard();
						CloseClipboard();
					}
				}
			}
			else if (IsRun)//run
			{
				CString runcmd=Key.SubString(4,fk.GetLength());
				WinExec(runcmd.c_str(),SW_SHOWNORMAL);
			}

			else //do single keys
			{
				PushAString(returned);
			}
		}
		else
		{
			PushAString(returned);
		}
	}

	//reset changed shiftstate
	//**************
	//changed?
	if (KeyState!=CopyOfKeyState)
	{
		//reset key states
		SetKeyStates(CopyOfKeyState);
	}

	//do inter-key delay if not a sleep command
	if(!IsSleep)
	{
		GTPK->DoKeyDelay();
	}
}
//-----------------------------------------------------------
//push each key in a string
void PushAString(CString s)
{
	for(int i=0;i<s.GetLength();i++)
	{
		PushAKey(s[i+1]);
	}
}

//-----------------------------------------------------------
//set control key states
void SetKeyStates(TShiftState ss)
{
	if (ss.Contains(ssShift))
	{
		SHIFTOn();
	}
	else
	{
		SHIFTOff();
	}

	if (ss.Contains(ssCtrl))
	{
		CTRLOn();
	}
	else
	{
		CTRLOff();
	}

	if (ss.Contains(ssAlt))
	{
		ALTOn();
	}
	else
	{
		ALTOff();
	}

}
//-----------------------------------------------------------
//return no of times function key {xx y} will be pressed
//strip out count charcters
int GetFunctionKeyCount(CString &fk)
{
	int rv=1;

	int numstart=fk.LastDelimiter(" ");

	if (numstart!=0 && numstart<fk.GetLength())
	{
		//extract count
		rv=StrToIntDef(fk.SubString(numstart,999),-1);

		//valid numeric characters?
		if (rv>=0)
		{
			//strip out count characters
			fk=fk.SubString(1,numstart-1);
		}
		else
		{
			rv=1;
		}
	}

	return rv;
}
//-----------------------------------------------------------
//return a control key character key code from a string
//-1 if no match
byte GetControlKey(CString fk)
{
	CString ck="+^%!";

	int idx=ck.Pos(fk[1]);

	if (idx==0)
		idx=-1;

	return (byte)idx;

}

//-----------------------------------------------------------
//return a function key virtual key code from a string
//-1 if no match
byte GetFunctionKey(CString fk)
{
	byte rv=-1;

	// Function Keys
	CString fkeys[]={
		"BACKSPACE","BS","BKSP"
			,"BELL"
			,"RIGHTWIN"
			,"LEFTWIN","START"
			,"APPS","CONTEXT"
			,"PAUSE"
			,"BREAK"
			,"CANCEL"
			,"CAPSLOCK","CAPS"
			,"DELETE","DEL"
			,"DOWN"
			,"END"
			,"ENTER","RETURN"
			,"ESCAPE","ESC"
			,"FF"
			,"HELP"
			,"HOME"
			,"INSERT","INS"
			,"LEFT"
			,"NEWLINE","NL"
			,"NUMLOCK"
			,"PGDN","PAGEDOWN","NEXT"
			,"PGUP","PAGEUP","PRIOR"
			,"PRINTSCREEN","PRTSC"
			,"RIGHT"
			,"SCROLLLOCK","SCRLK"
			,"TAB"
			,"UP"
			,"F1","F2","F3","F4","F5","F6"
			,"F7","F8","F9","F10","F11","F12"
			,"F13","F14","F15","F16","F17","F18"
			,"F19","F20","F21","F22","F23","F24"
			,"NUMPAD0","NUMPAD1","NUMPAD2","NUMPAD3","NUMPAD4"
			,"NUMPAD5","NUMPAD6","NUMPAD7","NUMPAD8","NUMPAD9"
			,"NUMPADMUTIPLY","NUMPAD*"
			,"NUMPADADD","NUMPAD+"
			,"NUMPADSUBTRACT","NUMPAD-"
			,"NUMPADDECIMAL","NUMPAD."
			,"NUMPADDIVIDE","NUMPAD/"
			,"LEFTBRACE","RIGHTBRACE"
	};

	byte fvk[]={
		VK_BACK,VK_BACK,VK_BACK
			,VK_BELL
			,VK_RWIN
			,VK_LWIN,VK_LWIN
			,VK_APPS, VK_APPS
			,VK_PAUSE
			,VK_BREAK
			,VK_CANCEL
			,VK_CAPITAL,VK_CAPITAL
			,VK_DELETE,VK_DELETE
			,VK_DOWN
			,VK_END
			,VK_RETURN,VK_RETURN
			,VK_ESCAPE,VK_ESCAPE
			,VK_CLEAR
			,VK_HELP
			,VK_HOME
			,VK_INSERT,VK_INSERT
			,VK_LEFT
			,VK_LINEFEED,VK_LINEFEED
			,VK_NUMLOCK
			,VK_NEXT,VK_NEXT,VK_NEXT
			,VK_PRIOR,VK_PRIOR,VK_PRIOR
			,VK_SNAPSHOT,VK_SNAPSHOT
			,VK_RIGHT
			,VK_SCROLL,VK_SCROLL
			,VK_TAB
			,VK_UP
			,VK_F1,VK_F2,VK_F3,VK_F4,VK_F5,VK_F6
			,VK_F7,VK_F8,VK_F9,VK_F10,VK_F11,VK_F12
			,VK_F13,VK_F14,VK_F15,VK_F16,VK_F17,VK_F18
			,VK_F19,VK_F20,VK_F21,VK_F22,VK_F23,VK_F24
			,VK_NUMPAD0,VK_NUMPAD1,VK_NUMPAD2,VK_NUMPAD3,VK_NUMPAD4
			,VK_NUMPAD5,VK_NUMPAD6,VK_NUMPAD7,VK_NUMPAD8,VK_NUMPAD9
			,VK_MULTIPLY,VK_MULTIPLY
			,VK_ADD,VK_ADD
			,VK_SUBTRACT,VK_SUBTRACT
			,VK_DECIMAL,VK_DECIMAL
			,VK_DIVIDE,VK_DIVIDE
			,'{','}'
	};

	int arraysize=sizeof(fvk)/sizeof(fvk[0]);

	fk=fk.UpperCase();

	for (int i=0;i<arraysize;i++)
	{
		if (fk==fkeys[i])
		{
			rv=fvk[i];
			break;
		}
	}

	return rv;
}
//-----------------------------------------------------------
//indicate if passed string represents a function key
bool IsAFunctionKey(CString fk)
{
	bool rv=false;

	if (GetFunctionKey(fk)!=(byte)-1 || GetControlKey(fk)!=(byte)-1)
		rv=true;

	return rv;
}
//-----------------------------------------------------------
//delay loop
void DoDelay(int mS)
{
	if (mS==0)
		return;

	DWORD start=GetTickCount();
	DWORD end=start+mS;


	//has time wrapped
	if (end<start)
		//wait for it to reach zero
	{
		while (GetTickCount()>start)
		{
			Application->ProcessMessages();
		}
	}

	while(GetTickCount()<end)
	{
		Application->ProcessMessages();
	}
}
//-----------------------------------------------------------
//do CTRL keys
void CTRLOn()
{
	if (ControlOn) return;
	ActivateTarget();
	keybd_event(VK_CONTROL, (BYTE)MapVirtualKey(VK_CONTROL,0), 0, 0);
	ControlOn=true;
}
//-----------------------------------------------------------
void CTRLOff()
{
	if(!ControlOn) return;
	ActivateTarget();
	keybd_event(VK_CONTROL, (BYTE)MapVirtualKey(VK_CONTROL, 0), KEYEVENTF_KEYUP, 0);
	ControlOn=false;
}
//-----------------------------------------------------------
//ctrl +
void PushCTRLKey(char k,bool Off)
{
	CTRLOn();
	PushAKey(k);
	if (Off)
		CTRLOff();
}
//-----------------------------------------------------------
//do normal Alt keys
void ALTOn()
{
	if(AltOn) return;
	ActivateTarget();
	keybd_event(VK_MENU, (BYTE)MapVirtualKey(VK_MENU,0), 0, 0);
	AltOn=true;
}
//-----------------------------------------------------------
void ALTOff()
{
	if(!AltOn) return;
	ActivateTarget();
	keybd_event(VK_MENU, (BYTE)MapVirtualKey(VK_MENU, 0), KEYEVENTF_KEYUP, 0);
	AltOn=false;
}
//-----------------------------------------------------------
//do right Alt keys AltGr
void RALTOn()
{
	if(RAltOn) return;
	//simulate
	ALTOn();
	CTRLOn();
	RAltOn=true;
}
//-----------------------------------------------------------
void RALTOff()
{
	if(!RAltOn) return;
	//simulate
	ALTOff();
	CTRLOff();
	RAltOn=false;
}
//-----------------------------------------------------------
//do shift keys
void SHIFTOn()
{
	if (ShiftOn) return;
	ActivateTarget();
	keybd_event(VK_SHIFT, (BYTE)MapVirtualKey(VK_SHIFT,0), 0, 0);
	ShiftOn=true;
}
//-----------------------------------------------------------
void SHIFTOff()
{
	if(!ShiftOn) return;
	ActivateTarget();
	keybd_event(VK_SHIFT, (BYTE)MapVirtualKey(VK_SHIFT, 0), KEYEVENTF_KEYUP, 0);
	ShiftOn=false;
}
//-----------------------------------------------------------
void PressKey(BYTE Vk, UINT Scan)
{

	//Presses the appropriate key specified
	ActivateTarget();
	keybd_event(Vk, (BYTE)Scan, 0, 0);
	keybd_event(Vk, (BYTE)Scan, KEYEVENTF_KEYUP, 0);

}
//-----------------------------------------------------------
void PushAKey(CHAR k)
{

	bool doshift=false;
	bool doalt=false;
	bool doctrl=false;

	DWORD vks=VkKeyScan(k);
	DWORD oeks=OemKeyScan(k);

	if(oeks!=(DWORD)-1) //standard key
	{
		if (oeks & 0x00020000)
		{
			doshift=true;
		}

		if (oeks & 0x00040000)
		{
			doctrl=true;
		}

		if (oeks & 0x00080000)
		{
			doalt=true;
		}
	}
	//not a standard key
	else
	{
		oeks=vks;

		if (oeks & 0x0100)
		{
			doshift=true;
		}

		if (oeks & 0x0200)
		{
			doctrl=true;
		}

		if (oeks & 0x0400)
		{
			doalt=true;
		}

	}

	//invalid key code
	if (oeks==(DWORD)-1 || vks==(DWORD)-1)
		return;

	//no need for shift if it's already on
	if (ShiftOn)
	{
		doshift=false;
	}

	//no need for ctrl if it's already on
	if (ControlOn)
	{
		doctrl=false;
	}

	//no need for alt if it's already on
	if (AltOn)
	{
		doalt=false;
	}

	//shift on
	if (doshift)
	{
		SHIFTOn();
	}

	//ctrl on
	if (doctrl)
	{
		CTRLOn();
	}

	//alt on
	if (doalt)
	{
		ALTOn();
	}

	// Press the key
	PressKey((BYTE)vks, (BYTE)oeks);

	//shift off
	if (doshift)
	{
		SHIFTOff();
	}

	//ctrl off
	if (doctrl)
	{
		CTRLOff();
	}

	//alt off
	if (doalt)
	{
		ALTOff();
	}


}

//-----------------------------------------------------------
//function keys
void PushFnKey(CString KeyCode)
{

	// Outputs function key. KeyCode may have a number of times to be output
	LONG NumPushes;
	INT index;
	CString FnKey;
	CString OrigCode;
	bool isfnkey=false;

	// Work out which function key to push and how many times

	//get key press count & strip out count characters
	NumPushes=GetFunctionKeyCount(KeyCode);

	//keep a copy
	OrigCode=KeyCode;

	FnKey=KeyCode.UpperCase();

	//direct code entry
	if (FnKey.Pos("\\")==1)
	{
		ProcessFuncKey(FnKey);
		return;
	}

	//braces
	if (FnKey=="LEFTBRACE" || FnKey=="RIGHTBRACE")
	{
		char pc=FnKey.Pos("LEFT")<1 ? '}' : '{';
		for (index=1;index<=NumPushes;index++)
		{
			PushAKey(pc);
		}
		return;
	}

	//search for F key
	byte fnkey=GetFunctionKey(FnKey);

	if (fnkey==(byte)-1)
		fnkey=GetControlKey(FnKey);

	if (fnkey!=(byte)-1)
	{
		isfnkey=true;
	}

	//press f key
	if (isfnkey)
	{
		bool dodos=false;

		//do DOS conversion?
		if (DOSKey)
		{
			byte vk[]={VK_BELL,VK_BREAK,VK_RETURN,VK_ESCAPE,VK_CLEAR,VK_LINEFEED,VK_TAB};
			char dk[]={'G','C','M','[','L','J','I'};

			//search for key equivalent
			for (int i=0;i<sizeof(vk)/sizeof(vk[0]);i++)
			{
				if (fnkey==vk[i])
				{
					//match found
					dodos=true;

					for (index=1;index<=NumPushes;index++)
					{
						//do key press
						PushCTRLKey(dk[i]);
					}
					break;
				}
			}
		}
		if (!dodos)//normal fkey
		{
			for (index=1;index<=NumPushes;index++)
			{
				//reserved characters
				if (GetControlKey(fnkey)!=(byte)-1)
				{
					PushAKey(fnkey);
				}
				else
				{
					//full printscreen
					if (fnkey==VK_SNAPSHOT && !AltOn)
					{
						PressKey(fnkey,1);
					}
					else
					{
						PressKey(fnkey,MapVirtualKey(fnkey,0));
					}
				}
			}
		}
	}
	//sleeep or NL or clear clipboard or run
	else if(FnKey!="SLEEP" && FnKey!="NEWLINE" && FnKey!="NL"  && FnKey !="EMPTYCLIP"
		&& FnKey.Pos("RUN ")!=1)
	{
		// Ordinary keys
		for (index=1; index<=NumPushes; index++)
		{
			for (int i=0;i<OrigCode.GetLength();i++)
			{
				char ss=OrigCode[i+1];

				//watch for {~ 2}
				if(ss!='~' || OrigCode=="~")
				{
					OnKeyHandler(false,ss);
				}
				else
				{
					//{hello world~ 3}
					OnKeyHandler(true,"ENTER");
				}
			}
		}
	}
}

//-----------------------------------------------------------
//set initial key state variables
void InitialKeyState()
{
	SHORT ks=GetAsyncKeyState(VK_SHIFT);
	ShiftOn=(ks & 0x8000)==0x8000;

	ks=GetAsyncKeyState(VK_CONTROL);
	ControlOn=(ks & 0x8000)==0x8000;

	ks=GetAsyncKeyState(VK_MENU);
	AltOn=(ks & 0x8000)==0x8000;

	RAltOn=AltOn & ControlOn;

}

//-----------------------------------------------------------
//all control keys off
void AllOff()
{
	SHIFTOff();
	ALTOff();
	RALTOff();
	CTRLOff();
}

//normal pushkeys
void PushKeys(CString src)
{
	InitialKeyState();
	AllOff();
	DOSKey=false;
	__PushKeys(src);
}
//-----------------------------------------------------------
//DOS pushkeys
void PushDOSKeys(CString src)
{
	InitialKeyState();
	AllOff();
	DOSKey=true;
	__PushKeys(src);
}
//-----------------------------------------------------------
//send f keys to onkey handler
bool ProcessFuncKey(CString k)
{
	//talke a copy
	CString temp=k;

	//get count value & strip out count characters
	//from temporary value
	int count=GetFunctionKeyCount(k);

	//is it a function key?
	if (IsAFunctionKey(k) || k.Pos("\\")==1)
	{
		//pass it count times to handler
		for(int i=0;i<count;i++)
		{
			OnKeyHandler(true,k);
		}
		return true;
	}
	else
	{
		//restore original if not an F key
		k=temp;
	}

	return false;
}
//-----------------------------------------------------------
void __PushKeys(const char* src)
{

	//Copyright by Data Solutions Pty Ltd (ACN 010 951 498)
	//All rights reserved.
	//Email for info: clatta@ozemail.com.au


	// This is the routine that does all the work
	int index;
	CString SubStr;
	INT BrCnt;
	CHAR BrOpen;
	CHAR BrClose;
	CHAR Ch;
	CHAR nextCh;

	int srcLen=strlen(src);
	if (srcLen == 0)
		return;

	char ck[]={'+','^','%','!'};
	void (*ckonfuncs[])()={SHIFTOn,CTRLOn,ALTOn,RALTOn};
	void (*ckofffuncs[])()={SHIFTOff,CTRLOff,ALTOff,RALTOff};

	index = 0;
	while (index<srcLen)
	{
		char nextCh = src[index + 1];

		// control keys & functions
		bool controlkey = false;
		int ckidx;

		//is it a control key next?
		for (int ckidx = 0; ckidx < sizeof(ck) / sizeof(*ck); ckidx++)
		{
			if (nextCh == ck[ckidx])
			{
				//contol key alone?
				if (srcLen == 1)
				{
					//turn control key on
					(*ckonfuncs[ckidx])();

					//control key off
					(*ckofffuncs[ckidx])();

					//exit
					return;
				}

				controlkey=true;
				break;
			}
		}

		//shift,ctrl,alt,ralt found
		if (controlkey && srcLen>=index+2)
		{
			BrOpen=src[index+2];
			if ((BrOpen=='(')||(BrOpen=='{'))
			{
				if (BrOpen=='(')
				{
					BrClose=')';
				}
				else
				{
					BrClose='}';
				}
				index++;
				SubStr="";
				BrCnt=1;
				while ((index<srcLen-1)&&(BrCnt!=0))
				{
					index++;
					Ch=src[index+1];

					if (Ch!=BrClose)
					{
						SubStr+=Ch;
					}
					else 
					{
						BrCnt--;
					}
					if (Ch==BrOpen)
						BrCnt++;
				}

				// Turn control key on
				(*ckonfuncs[ckidx])();

				// Push the keys
				if (SubStr!="")
				{
					if (BrOpen=='(')
					{
						__PushKeys(SubStr);
					}
					else
					{
						CString t=SubStr;
						if(ProcessFuncKey(t))
						{
							t="";
						}

						if (t.GetLength()>0)
							PushFnKey(t);
					}
				}
				// Turn control key off
				(*ckofffuncs[ckidx])();
			}
			else
			{
				// The next key uses control
				index++;
				(*ckonfuncs[ckidx])();

				OnKeyHandler(false,src[index+1]);
				(*ckofffuncs[ckidx])();
			}

		}
		else if (nextCh=='{' && src!="{}" && src!="}{")
		{
			// Function keys
			index++;
			SubStr="";

			while ((index<(srcLen-1))&&(src[index+1]!='}'))
			{
				SubStr+=src[index+1];
				index++;
			}

			if(srcLen>1)
			{
				if (src[srcLen]=='}' && src[srcLen-1]=='}')
				{
					// Right brace
					OnKeyHandler(false,'}');
					PushAKey('}');
					index++;
				}
				else
				{
					CString k=SubStr;
					if (ProcessFuncKey(k))
					{
						k="";
					}
					else if(k.UpperCase().Pos("SLEEP")==1)
					{
						//sleep
						OnKeyHandler(false,k);
					}
					//****************************************
					//clear clipboard
					else if (k.UpperCase().Pos("EMPTYCLIP")==1)
					{
						OnKeyHandler(false,k);
					}
					//run
					else if (k.UpperCase().Pos("RUN ")==1)
					{
						OnKeyHandler(false,k);
					}

					//not a function key (a 3} type of thing
					if(k.GetLength()>0)
					{
						PushFnKey(k);
					}
				}
			}
		}
		else if (nextCh=='~')
		{
			// The enter key
			ProcessFuncKey("ENTER");
		}
		else
		{
			OnKeyHandler(false,nextCh);
		}
		index++;
	}

	//turn all control keys off
	AllOff();

}



#endif 0


/*
static int l_DoIt(lua_State* L)
{
	if (!args[1].IsNumber())
	{
		return 0;
	}

	HWND hwnd = (HWND)(UINT)args[1].GetNumber();
	UINT vk = '1';
	UINT scan = MapVirtualKey(vk, 0);

	INPUT input;
	input.type = INPUT_KEYBOARD;
	input.ki.wVk = vk;
	input.ki.wScan = scan;
	input.ki.dwFlags = 0;
	input.ki.time = 0;
	input.ki.dwExtraInfo = GetMessageExtraInfo();

//	::SendMessage(hwnd, WM_KEYDOWN, VK_NUMPAD1, 1 | (scan << 16));
	UINT numEvents = ::SendInput(1, &input, sizeof(INPUT));
	printf("%d\n", numEvents);
	input.type = INPUT_KEYBOARD;
	input.ki.wVk = vk;
	input.ki.wScan = scan;
	input.ki.dwFlags = KEYEVENTF_KEYUP;
	input.ki.time = 0;
	input.ki.dwExtraInfo = GetMessageExtraInfo();

//	::SendMessage(hwnd, WM_KEYDOWN, VK_NUMPAD1, 1 | (scan << 16));
	numEvents = ::SendInput(1, &input, sizeof(INPUT));
	printf("%d\n", numEvents);
	return 0;
}
*/

static int l_SendKeys(lua_State* L) {
	LPCSTR keys = luaL_checkstring(L, 1);
	CKeystrokeEngine keystrokeEngine(keys);
	keystrokeEngine.SendKeys();
	return 0;
}


static const struct luaL_reg windows_funcs[] = {
	{ "GetForegroundWindow", l_GetForegroundWindow },
	{ "GetActiveWindow", l_GetActiveWindow },
	{ "GetFocus", l_GetFocus },
	{ "GetWindowText", l_GetWindowText },
	{ "FindWindowEx", l_FindWindowEx },
	{ "SendKeys", l_SendKeys },
	{ NULL, NULL }
};


LUALIB_API int luaopen_windows(lua_State* L) {
	luaL_register(L, "windows", windows_funcs);
	return 1;
}
