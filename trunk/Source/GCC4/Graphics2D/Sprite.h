#pragma once
//========================================================================
// Sprite.h : Defines sprites
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


//========================================================================
//  Content References in Game Coding Complete 3rd Edition
// 
// class Sprite			- Chapter 10, page 313
// class BitmapSprite   - Chapter 10, page 322
//
//========================================================================

#include "../GameCode4/interfaces.h"
#include "../ResourceCache/ResCache.h"				// needed for ResourceSprite
#include "Font.h"



class Sprite : public IScreenElement
{
protected:
	// Position and Pixel data
	// -----------------------------
	ID3DXSprite *m_d3dSprite;			// D3D X Sprite
	IDirect3DTexture9 *m_pTexture;		// Texture to draw on the Sprite

	CPoint m_Position, m_Hotspot;		// subtract HS from pos to get origin
	int m_ZOrder;						// the sort order of the sprite
	int m_Width, m_Height;				// dimensions of one frame
	int m_TextureWidth, m_TextureHeight;// dimensions of the entire texture
	int m_CurrentFrame, m_NumFrames;	// current frame and total frames
	bool m_HasColorKey;				// set to true if the sprite has a color key
	bool m_IsVisible;				// set to true if you want the sprite to draw

	// Members that control animation
	// ------------------------------
	bool m_IsPaused;			// set to true if the animation has been paused
	bool m_LoopingAnim;			// set to true if the animation loops
	int m_ElapsedTime;			// the measure of total elapsed time in ms
	int m_MSPerFrame;			// ms per frame (1000 / desired frames per second)

public:
	Sprite();
	virtual ~Sprite() { SAFE_RELEASE(m_d3dSprite); SAFE_RELEASE(m_pTexture); }

	virtual CPoint const VGetPos() const { return m_Position; } 
	virtual void VSetPos(const CPoint &loc) { m_Position = loc; }
	virtual void VSetHotspot(const CPoint &loc) { m_Hotspot = loc; }
	virtual void VSetSize(const CPoint &size) { m_Width=size.x; m_Height=size.y; }
	virtual CRect const VGetRect() const { return CRect(0,0,m_Width-1,m_Height-1); }
	virtual int const VGetWidth() const { return m_Width; }
	virtual int const VGetHeight() const { return m_Height; }

	// Implementation of the IScreenElement interface
	virtual void VOnUpdate(int deltaMS);
	virtual HRESULT VOnRender(double fTime, float fElapsedTime);
	virtual int VGetZOrder() const { return m_ZOrder; }
	virtual void VSetZOrder(int const zOrder) { m_ZOrder = zOrder; }
	virtual bool VIsVisible() const { return m_IsVisible; }
	virtual void VSetVisible(bool visible) { m_IsVisible = visible; }
	virtual LRESULT CALLBACK VOnMsgProc( AppMsg msg ) { return 0; }
	virtual HRESULT VOnRestore();

	void *GetSurface() { return m_d3dSprite; }

	// the method that sets the current frame
	void SetFrame(const int desiredFrame) { m_CurrentFrame = desiredFrame % m_NumFrames; }

	// the method that retrieves the current frame
	int GetFrame() const { return m_CurrentFrame; }

	// the method that returns the number of frames in this animation
	int GetFrameCount() const 	{ return m_NumFrames; }

	void SetAnim(bool isPaused=false, bool looping=true, int msPerFrame=67)
		{ m_IsPaused=isPaused; m_LoopingAnim=looping; m_MSPerFrame=msPerFrame; }
};


// Members added post-press
// --------------------------

class BitmapSprite : public Sprite
{
protected:
	std::string m_fileName;
	bool m_bInCache;

public:
	BitmapSprite(const std::string fileName, bool inCache, const int frames);

	// Implementation of the IScreenElement interface
	virtual HRESULT VOnRestore();
};


class TextSprite : public Sprite
{
protected:
	std::wstring m_text;				// the text to draw
	TextStyle m_style;					// the style of the text
	bool m_isCentered;					// justification
	optional<CPoint> m_size;			// a static surface size, or size to fit if size is invalid

public:
	TextSprite (
		std::wstring text,					// the text to draw
		TextStyle style, 					// the style of the text
		bool isCentered,					// justification
		const CPoint &position,				// position of this sprite on the parent surface
		optional<CPoint> size				// a static surface size, or size to fit if size is invalid
	);

	// Implementation of the IScreenElement interface
	virtual HRESULT VOnRestore();
	HRESULT VOnRender(double fTime, float fElapsedTime);

	void SetText(const std::wstring text);			// changes the text string
};


