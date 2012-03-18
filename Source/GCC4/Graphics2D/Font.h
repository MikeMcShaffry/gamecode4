#pragma once
//========================================================================
// Font.h : Useful classes for drawing and formatting text strings
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
//  enum FontEnum		- Chapter 10, page 329
//  struct TextShadow   - Chapter 10, page 329
//  struct TextStyle    - Chapter 10, page 329
//  class Font			- Chapter 10, page 330
//  class FontHandler   - Chapter 10, page 331
//


/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Forward References
//////////////////////////////////////////////////////////////////////

class Sprite;

enum FontEnum
{
	FONT_DEFAULT,
	FONT_SYSTEM = FONT_DEFAULT,		//0
	FONT_ARIAL_BOLD_8,				//1
	FONT_ARIAL_BOLD_14,				//2
	FONT_ARIAL_BLACK_12,			//3          
	FONT_ARIAL_BLACK_24,			//4
	FONT_WINGDING_10,				//5
	FONT_COURIERNEW_18,				//6
	FONT_PALATINO_SMALL,			//7
	FONT_PALATINO_MEDIUM,			//8
	FONT_PALATINO_LARGE,			//9
	FONT_LARGE_LED_DISPLAY,			//10
	FONT_TREBUCHET_10,				//11
	FONT_TREBUCHET_MED,				//12
	FONT_TAHOMA_MED,				//13
	FONT_TAHOMA_LARGE,				//14

	// Define more font enumerations here....and add definitions into 
	// FontHandler::m_FontRecords in Font.cpp

	MAX_FONTS						//15
};



struct TextShadow
{
	Color m_color;
	CPoint m_offset;

	TextShadow(Color const &color, CPoint const &offset=CPoint(1,1))
		: m_color(color), m_offset(offset) { }
};

struct TextStyle
{
	FontEnum m_font;
	Color m_foreground;
	Color m_background;
	int m_format;
	optional<TextShadow> m_shadow;

	TextStyle(
		FontEnum font, 
		Color foreground, 
		Color background=g_Transparent, 
		int format = DT_TOP | DT_LEFT,
		optional<TextShadow> shadow=optional_empty()
	);

	TextStyle & operator = (const TextStyle &sp);
};
	

//////////////////////////////////////////////////////////////////////
// Class Font
//////////////////////////////////////////////////////////////////////

// FUTURE WORK: This should inherit from an interface class, like IFont, to remove OS dependancy!!!

class Font
{
	friend class FontHandler;
public:
	Font(ID3DXFont *font) { m_d3dFont = font;  }
	virtual ~Font() { SAFE_RELEASE(m_d3dFont); }

private:
	ID3DXFont *m_d3dFont;
};


class FontHandler
{
protected:
	static D3DXFONT_DESC m_FontRecords[];
	Font *m_pFonts[MAX_FONTS];

	CSize GetStringSize(std::wstring const &s, Font *font, int format) const;

public:

	FontHandler();
	virtual ~FontHandler();

	CSize const GetStringSize(std::wstring const &s, TextStyle const style) const;
	int	GetHeight(TextStyle const style) const;
	int	GetStringWidth(std::wstring const &s, TextStyle style) const;
	std::wstring const ParseMessage(std::wstring const &s, TextStyle style, const CSize & size ) const;

	HRESULT FontHandler::DrawText(
		Sprite *pSprite,
		const std::wstring strText,
		const TextStyle style,
		const CPoint & origin
		) const;
};




