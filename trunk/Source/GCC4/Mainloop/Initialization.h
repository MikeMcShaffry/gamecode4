#pragma once
//========================================================================
// Initialization.h : Defines utility functions for game initialization
//
//
// Part of the GameCode4 Application
//
// GameCode4 is the sample application that encapsulates much of the source code
// discussed in "Game Coding Complete - 4th Edition" by Mike McShaffry and David
// "Rez" Graham, published by Charles River Media. 
// ISBN-10: 1133776574 | ISBN-13: 978-1133776574
//
// If this source code has found it's way to you, and you think it has helped you
// in any way, do the authors a favor and buy a new copy of the book - there are 
// detailed explanations in it that compliment this code well. Buy a copy at Amazon.com
// by clicking here: 
//    http://www.amazon.com/gp/product/1133776574/ref=olp_product_details?ie=UTF8&me=&seller=
//
// There's a companion web site at http://www.mcshaffry.com/GameCode/
// 
// The source code is managed and maintained through Google Code: 
//    http://code.google.com/p/gamecode4/
//
// (c) Copyright 2012 Michael L. McShaffry and David Graham
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser GPL v3
// as published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See 
// http://www.gnu.org/licenses/lgpl-3.0.txt for more details.
//
// You should have received a copy of the GNU Lesser GPL v3
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
//========================================================================


extern bool CheckStorage(const DWORDLONG diskSpaceNeeded);
extern DWORD ReadCPUSpeed();
extern bool CheckMemory(const DWORDLONG physicalRAMNeeded, const DWORDLONG virtualRAMNeeded);
extern bool IsOnlyInstance(LPCTSTR gameTitle);
extern const TCHAR *GetSaveGameDirectory(HWND hWnd, const TCHAR *gameAppDirectory);
extern bool CheckForJoystick(HWND hWnd);

//extern DWORD GetFreeVRAM();		// deprecated!!!!

struct GameOptions
{
	// Level option
	std::string m_Level;

	// Rendering options
	std::string m_Renderer;
	bool m_runFullSpeed;
	Point m_ScreenSize;

	// Sound options
	float m_soundEffectsVolume;			
	float m_musicVolume;				

	// Multiplayer options
	int m_expectedPlayers;				
	int m_listenPort;					
	std::string m_gameHost;				
	int m_numAIs;
	int m_maxAIs;
	int m_maxPlayers;

    // resource cache options
    bool m_useDevelopmentDirectories;

	// TiXmlElement - look at this to find other options added by the developer
	TiXmlDocument *m_pDoc;

	GameOptions();
	~GameOptions() { SAFE_DELETE(m_pDoc); }

	void Init(const char* xmlFilePath, LPWSTR lpCmdLine);
};
