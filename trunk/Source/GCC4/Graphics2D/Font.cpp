//========================================================================
// Font.cpp : Useful classes for drawing and formatting text strings
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
//  TextStyle - a class to encapsulate font creation parameters. Using text
//              styles in your game is similar to using CSS on a web site - it
//              becomes very easy to change the entire look of the game.
//
//  FontHandler - a useful class that preloads font definitions. The wise
//                programmer would make this class read from a data file...
//                It is also responsible for drawing fonts onto surfaces.
// 
//========================================================================


#include "GameCodeStd.h"
#include "../GameCode4/GameCode.h"

#include "Font.h"
#include "Sprite.h"
#include "../Utilities/String.h"

TextStyle::TextStyle(
	FontEnum font, 
	Color foreground, 
	Color background, 
	int format,
	optional<TextShadow> shadow )
{
	m_font = font;
	m_foreground = foreground;
	m_background = background; 
	m_format = format;
	m_shadow = shadow;
}

TextStyle & TextStyle::operator = (const TextStyle &other) 
{
	m_shadow.clear();
	m_font = other.m_font;
	m_foreground = other.m_foreground;
	m_background = other.m_background; 
	m_format = other.m_format;
	if (other.m_shadow.valid())
		m_shadow = TextShadow((*(other.m_shadow)).m_color, (*(other.m_shadow)).m_offset) ;
	
	return *this;
}


// 
// FontHandler::m_FontRecords - Chapter 10, page 332
//
D3DXFONT_DESC FontHandler::m_FontRecords[] = 
{
	//  h   w  weight      mip,itl   char. set       outPrecisions     quality                 pitch/family                    typeface
	{ -14,	0,	FW_NORMAL,	1,	0,	ANSI_CHARSET,	OUT_STROKE_PRECIS, ANTIALIASED_QUALITY,	FF_SWISS | VARIABLE_PITCH,		_T("(Arial)") },
	{ -11,	0,	FW_BOLD,	1,	0,	ANSI_CHARSET,	OUT_STROKE_PRECIS, ANTIALIASED_QUALITY,	FF_SWISS | VARIABLE_PITCH,		_T("(Arial)") },	
	{ -19,	0,	FW_BOLD,	1,	0,	ANSI_CHARSET,	OUT_STROKE_PRECIS, ANTIALIASED_QUALITY,	FF_SWISS | VARIABLE_PITCH,		_T("(Arial)") },		
	{ -16,	0,	FW_BOLD,	1,	0,	ANSI_CHARSET,	OUT_STROKE_PRECIS, ANTIALIASED_QUALITY,	FF_SWISS | VARIABLE_PITCH,		_T("(Arial)") },		
	{ -37,	0,	FW_NORMAL,	1,	0,	ANSI_CHARSET,	OUT_STROKE_PRECIS, ANTIALIASED_QUALITY,	FF_SWISS | VARIABLE_PITCH,		_T("(Impact)") },			
	{ -17,	0,	FW_NORMAL,	1,	0,	SYMBOL_CHARSET,	OUT_STROKE_PRECIS, ANTIALIASED_QUALITY,	FF_DONTCARE | VARIABLE_PITCH,	_T("(Wingdings)") },			
	{ -21,	0,	FW_NORMAL,	1,	0,	ANSI_CHARSET,	OUT_STROKE_PRECIS, ANTIALIASED_QUALITY,	FF_SCRIPT | VARIABLE_PITCH,		_T("(Forte)") },
	{ -11,	0,	FW_BOLD,	1,	0,	ANSI_CHARSET,	OUT_STROKE_PRECIS, ANTIALIASED_QUALITY,	FF_ROMAN | VARIABLE_PITCH,		_T("(Palatino Linotype)") },
	{ -13,	0,	FW_BOLD,	1,	0,	ANSI_CHARSET,	OUT_STROKE_PRECIS, ANTIALIASED_QUALITY,	FF_ROMAN  | VARIABLE_PITCH,		_T("(Palatino Linotype)") },
	{ -16,	0,	FW_BOLD,	1,	0,	ANSI_CHARSET,	OUT_STROKE_PRECIS, ANTIALIASED_QUALITY,	FF_ROMAN | VARIABLE_PITCH,		_T("(Palatino Linotype)") },
	{ -35,	0,	FW_NORMAL,	1,	0,	ANSI_CHARSET,	OUT_STROKE_PRECIS, ANTIALIASED_QUALITY,	FF_DECORATIVE | FIXED_PITCH,	_T("(Quartz)") },
	{ -13,	0,	FW_BOLD,	1,	0,	ANSI_CHARSET,	OUT_STROKE_PRECIS, ANTIALIASED_QUALITY,	FF_SWISS | VARIABLE_PITCH,		_T("(Trebuchet MS)") },
	{ -16,	0,	FW_BOLD,	1,	0,	ANSI_CHARSET,	OUT_STROKE_PRECIS, ANTIALIASED_QUALITY,	FF_SWISS | VARIABLE_PITCH,		_T("(Trebuchet MS)") },
	{ -16,	0,	FW_BOLD,	1,	0,	ANSI_CHARSET,	OUT_STROKE_PRECIS, ANTIALIASED_QUALITY,	FF_SWISS | VARIABLE_PITCH,		_T("(Tahoma)") },
	{ -21,	0,	FW_BOLD,	1,	0,	ANSI_CHARSET,	OUT_STROKE_PRECIS, ANTIALIASED_QUALITY,	FF_SWISS | VARIABLE_PITCH,		_T("(Tahoma)") }
};



// 
// FontHandler::FontHandler - Chapter 10, page 333
//
FontHandler::FontHandler()
{
	for (int i=0; i<MAX_FONTS; ++i)
	{
		HRESULT hr;
		ID3DXFont *pFont = NULL;
		hr = D3DXCreateFontIndirect(DXUTGetD3D9Device(),&m_FontRecords[i], &pFont);
		if (hr==S_OK && pFont) 
		{
			m_pFonts[i] = GCC_NEW Font(pFont);
		}
		else
		{
			TCHAR msg[256];
			_tcssprintf(msg, _T("Font error: Couldn't initialize %s"), m_FontRecords[i].FaceName);
			MessageBox(g_pApp->GetHwnd(), msg, _T("Error"), MB_OK);
			GCC_ASSERT(0 && msg);
			m_pFonts[i] = NULL;
		}
	}
}


// 
// FontHandler::~FontHandler - Chapter 10, page 333
//
FontHandler::~FontHandler()
{
	for (int i=0; i<MAX_FONTS; ++i)
	{
		SAFE_DELETE(m_pFonts[i]);
	}
}

// 
// FontHandler::GetStringSize - Chapter 10, page 333
//
CSize FontHandler::GetStringSize(std::wstring const &s, Font *font, int format) const
{
	CRect rect(0,0,0,0);
	if (font->m_d3dFont->DrawText(NULL, s.c_str(), -1, &rect, DT_CALCRECT | format, 0)==0)
	{
		return CSize(0,0);
	}

	return rect.BottomRight();
}

// 
// FontHandler::GetStringSize - Chapter 10, page 334
//
CSize const FontHandler::GetStringSize(std::wstring const &text, TextStyle const style) const 
{
	GCC_ASSERT(text.length() && "Empty text sprites are not allowed!");

	CSize sizeText( 0, 0 );

	sizeText = GetStringSize(text, m_pFonts[style.m_font], style.m_format);

	if (style.m_shadow.valid())
	{
		CPoint offset = (*(style.m_shadow)).m_offset;
		sizeText.cx += abs( offset.x );
		sizeText.cy += abs( offset.y );
	}

	return sizeText;
}


// 
// FontHandler::GetHeight - Chapter 10, page 334
//
int	FontHandler::GetHeight( const TextStyle style) const
{
	std::wstring space = _T(" ");
	CSize size = GetStringSize(space, style);
	return size.cy;
}

// 
// FontHandler::GetStringWidth - Chapter 10, page 334
//
int	FontHandler::GetStringWidth(std::wstring const &s,  const TextStyle style) const
{
	return GetStringSize(s, style).cx;
}

// 
// FontHandler::ParseMessage - Chapter 10, page 334
//
std::wstring const FontHandler::ParseMessage( std::wstring const & nmessage, const TextStyle style, const CSize & size ) const
{
	//Initialize
	std::wstring line;
	std::wstring word;
	std::wstring multilineMessage;
	std::wstring message = nmessage;

	std::wstring potentialLine;

	TCHAR ch;

	// seperators
	static const int MAX_SEPS( 5 );
	TCHAR seps[MAX_SEPS+1] = _T("\n\t,- ");

	while (message.length())
	{
		//Remove the first single character from 'message'
		ch = message[ 0 ];
		message = message.substr(1);

		//Append that character to the word, making sure not to append lineBreak
		if(ch!=seps[0])
		{
			word += ch;
		}

		potentialLine = line + word;

		//If the potentialLine is too wide, then end this line
		if(GetStringWidth(potentialLine,style)>size.cx)
		{
			TrimLeft(line);
			multilineMessage += line;
			multilineMessage += '\n';
			line = _T("");
		}

		//End a word when it is complete
		for ( int i = 0; i < MAX_SEPS; i++ )
		{
			if ( ch == seps[ i ] )
			{
				line += word;
				word = _T("");

				//Make sure the lineBreaks transfer over
				if(ch==seps[0])
				{
					multilineMessage += line;
					multilineMessage += seps[0];
					line = _T("");
				}
				break;
			}
		}
	}//end while loop

	//Append what is left of the remaining line and word
	line += word;
	TrimLeft(line);
	multilineMessage += line;

	return multilineMessage;
}//end ParseMessage




// 
// FontHandler::DrawText - Chapter 10, page 336
//
HRESULT FontHandler::DrawText(
	Sprite *sprite,
	const std::wstring strText,
	const TextStyle style,
	const CPoint & origin
	) const
{
	Font *pFont = m_pFonts[style.m_font];
	ID3DXSprite *xSprite = (ID3DXSprite *)sprite->GetSurface();
	CRect rect(0,0,0,0);
	pFont->m_d3dFont->DrawText(xSprite, strText.c_str(), -1, &rect, DT_CALCRECT | style.m_format, style.m_foreground);

	if(style.m_shadow.valid())//Draw the shadow if you must
	{
		TextShadow shadow = *(style.m_shadow);
		//The shadow text gets drawn first, with the background color intact

		CRect shadowRect = rect;
		shadowRect.MoveToXY(rect.left + shadow.m_offset.x, rect.top + shadow.m_offset.y);
		pFont->m_d3dFont->DrawText(xSprite, strText.c_str(), -1, &shadowRect, style.m_format, shadow.m_color);
	}

	pFont->m_d3dFont->DrawText(xSprite, strText.c_str(), -1, &rect, style.m_format, style.m_foreground);

    return S_OK;
}

