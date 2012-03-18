//========================================================================
// Sprite.cpp : Defines sprites
//
// Part of the GameCode2 Application
//
// GameCode2 is the sample application that encapsulates much of the source code
// discussed in "Game Coding Complete - 2nd Edition" by Mike McShaffry, published by
// Paraglyph Press. ISBN: 1-932111-91-3
//
// If this source code has found it's way to you, and you think it has helped you
// in any way, do the author a favor and buy a new copy of the book - there are 
// detailed explanations in it that compliment this code well. Buy a copy at Amazon.com
// by clicking here: http://www.amazon.com/exec/obidos/ASIN/1932111913/gamecodecompl-20/
//
// There's also a companion web site at http://www.mcshaffry.com/GameCode/portal.php
//
// (c) Copyright 2005 Michael L. McShaffry
//
// This work is licensed under the Creative Commons Attribution-ShareAlike License. 
// To view a copy of this license, visit http://creativecommons.org/licenses/by-sa/1.0/ 
// or send a letter to:
//      Creative Commons
//      559 Nathan Abbott Way
//      Stanford, California 94305, USA.
//
//========================================================================

//========================================================================
//  Content References in Game Coding Complete 2nd Edition
// 
// 
// class Sprite			- Chapter 10, page 313
//
//========================================================================

#include "GameCodeStd.h"
#include "../GameCode4/GameCode.h"
#include "../Utilities/String.h"
#include "Sprite.h"
//#include "CopyAlpha.h"
#include "Font.h"


// 
// Sprite::Sprite - Chapter 10, page 315
//
Sprite::Sprite()
{
	m_d3dSprite = NULL;
	m_Position = m_Hotspot = CPoint(0,0);
	m_Width = m_Height = 0;
	m_TextureWidth = m_TextureHeight = 0;
	m_CurrentFrame = m_NumFrames = 0;
	m_HasColorKey = true;

	m_IsPaused = true;
	m_LoopingAnim = false;;
	m_ElapsedTime = 0;
	m_MSPerFrame = 1000 / 15;				// this should never be zero
	m_IsVisible = true;
	m_pTexture = NULL;
	m_ZOrder = ZORDER_LAYER2;
};

// 
// Sprite::VOnRestore - Chapter 10, page 315
//
HRESULT Sprite::VOnRestore()
{
	SAFE_RELEASE(m_d3dSprite);
	return (D3DXCreateSprite(DXUTGetD3D9Device(), &m_d3dSprite)==S_OK);
}

// 
// Sprite::VOnRestore - Chapter 10, page 319
//
HRESULT Sprite::VOnRender(double fTime, float fElapsedTime)
{
	CPoint actualPos( m_Position - m_Hotspot );
	CRect sourceRect( CPoint(0,0), CPoint(m_Width-1, m_Height-1 ) );

	// Adjust for the frame
	sourceRect.top += ( m_CurrentFrame * m_TextureHeight );
	sourceRect.bottom += ( m_CurrentFrame * m_TextureHeight );

	Vec3 center(1.0f, 1.0f, 0.0f);
	Vec3 translation((const float)actualPos.x, (const float)actualPos.y, 0.0f);

	if (m_d3dSprite && m_pTexture)
	{
		m_d3dSprite->Begin(D3DXSPRITE_ALPHABLEND);
		m_d3dSprite->Draw(m_pTexture, &sourceRect, &center, &translation, g_White);
		m_d3dSprite->End();
	}

	return S_OK;
}

// 
// Sprite::VOnRestore - Chapter 10, page 321
//
void Sprite::VOnUpdate(int deltaMS)
{
	if (m_IsPaused)
	{
		return;	
	}

	m_ElapsedTime += deltaMS;

	// Only call SetFrame() if we have to.
	// We're guaranteed to have to advance at least one frame...

	if (m_ElapsedTime >= m_MSPerFrame)
	{
		DWORD const numFramesToAdvance = (m_ElapsedTime / m_MSPerFrame);

		m_ElapsedTime -= (numFramesToAdvance * m_MSPerFrame);

		int desiredFrame = GetFrame() + numFramesToAdvance;

		//Check if we're looping...
		if ((false==m_LoopingAnim) && (desiredFrame >= GetFrameCount()))
		{
			desiredFrame = GetFrameCount() - 1;	//Stay on last frame...
		}
		
		//Now advance our frame properly...
		SetFrame(desiredFrame);		
	}
}



// 
// BitmapSprite::BitmapSprite - Chapter 10, page 322
//
BitmapSprite::BitmapSprite(const std::string fileName, bool inCache, const int frames)
{
	m_fileName = fileName;
	m_NumFrames = frames;
	m_bInCache = inCache;
}

// 
// BitmapSprite::VOnRestore - Chapter 10, page 322
//
HRESULT BitmapSprite::VOnRestore()
{
	if (FAILED (Sprite::VOnRestore() ) )
	{
		return E_FAIL;
	}

	SAFE_RELEASE(m_pTexture);

    D3DXIMAGE_INFO imageInfo;
	ZeroMemory(&imageInfo, sizeof(D3DXIMAGE_INFO));

	if (m_bInCache)
	{
		Resource resource(m_fileName.c_str());
		shared_ptr<ResHandle> texture = g_pApp->m_ResCache->GetHandle(&resource);
		if ( FAILED ( D3DXCreateTextureFromFileInMemoryEx( 
			DXUTGetD3D9Device(), // device
			texture->Buffer(),		// source data
			texture->Size(),	// source data size
			D3DX_DEFAULT,		// width - get it from the file
			D3DX_DEFAULT,		// height - get it from the file
			D3DX_DEFAULT,		// miplevels - get it from the file
			0,					// useage - default (not a render target or dynamic)
			D3DFMT_UNKNOWN,		// format - get it from the file
			D3DPOOL_DEFAULT,	// pool - use default
			D3DX_FILTER_NONE,	// filter - don't use any filter
			D3DX_DEFAULT,		// mip filter - use default
			g_Transparent,		// color key - use our transparent color
			&imageInfo,			// grab the source info
			NULL,				// palleteindex - we don't need it
			&m_pTexture ) ) )	// the texture pointer
			return false;
	}
	else
	{
		if ( FAILED ( D3DXCreateTextureFromFileExA ( 
			DXUTGetD3D9Device(), 
			m_fileName.c_str(),	// source file name
			D3DX_DEFAULT,		// width - get it from the file
			D3DX_DEFAULT,		// height - get it from the file
			D3DX_DEFAULT,		// miplevels - get it from the file
			0,					// useage - default (not a render target or dynamic)
			D3DFMT_UNKNOWN,		// format - get it from the file
			D3DPOOL_DEFAULT,	// pool - use default
			D3DX_FILTER_NONE,	// filter - don't use any filter
			D3DX_DEFAULT,		// mip filter - use default
			g_Transparent,		// color key - use our transparent color
			&imageInfo,			// grab the source info
			NULL,				// palleteindex - we don't need it
			&m_pTexture ) ) )	// the texture pointer
			return false;
	}

	m_TextureWidth = m_Width = imageInfo.Width;
	m_TextureHeight = m_Height = imageInfo.Height / m_NumFrames;
	GCC_ASSERT(m_Height * m_NumFrames == imageInfo.Height && _T("Sprite height error"));

	return true;
}




TextSprite::TextSprite(
	std::wstring text,			// the text to draw
	TextStyle style, 			// the style of the text
	const bool isCentered,		// justification
	const CPoint &position,		// position of this sprite on the parent surface
	optional<CPoint> size		// a static surface size, or size to fit if size is invalid
) : m_style(style)
{
	m_text = text;
	m_isCentered = isCentered;

	m_size.clear();
	m_size = size;

	m_Position = position;
	m_NumFrames = 1;
};



HRESULT TextSprite::VOnRestore()
{
	// Two situations:
	// 1) String should be broken into a multiline entity to fit into a specific area
	// 2) String sent it need should take up as much space H&V that it needs

	// Set the width and height
	CSize sizeText;
	CSize sizeTotal;

	std::wstring multiline;

	if ( m_size.valid() )
	{
		// This is the total to fit into	
		sizeTotal = *m_size;

		// Fit the original text inside the bounds
		multiline = g_pApp->GetFontHandler()->ParseMessage( m_text, m_style, sizeTotal);
		sizeText = g_pApp->GetFontHandler()->GetStringSize(multiline, m_style);
	}
	else
	{
		// Set the width and height
		sizeText = g_pApp->GetFontHandler()->GetStringSize(m_text, m_style);
        sizeTotal = sizeText;

		multiline = m_text;
	}

	int numLines = CountLines(multiline);

	GCC_ASSERT(sizeTotal.cx > 0 && sizeTotal.cy > 0 && "About to create sprite with no width or height, is this what you want?");

	m_TextureWidth = m_Width = sizeTotal.cx;
	m_TextureHeight = m_Height = sizeTotal.cy;
	
	UINT texW = static_cast<UINT>(m_TextureWidth);
	UINT texH = static_cast<UINT>(m_TextureHeight);
	UINT numMipLevels = D3DX_DEFAULT;
	D3DFORMAT format = D3DFMT_UNKNOWN;
	
	// This call will reset width, height, and miplevels to the 
	// correct values.
	if (FAILED (D3DXCheckTextureRequirements(
		DXUTGetD3D9Device(), 
		&texW,
		&texH,
		&numMipLevels,
		0,
		&format,
		D3DPOOL_DEFAULT	) ) )
	{
		return false;
	}

	// Create the texture
	SAFE_RELEASE(m_pTexture);
	if ( FAILED ( DXUTGetD3D9Device()->CreateTexture(
		texW,		// width 
		texH,	// height 
		numMipLevels,		// miplevels - use default
		0,					// useage - default (not a render target or dynamic)
		format,				// format - unknown format
		D3DPOOL_DEFAULT,	// pool - use default
		&m_pTexture,		// the texture pointer
		NULL ) ) )
	{
		return false;
	}

	m_TextureWidth = texW;
	m_TextureHeight = texH;

	// Reinitialize the utility string to count the line breaks
	std::wstring utilString = multiline;

	// Size of border
	CSize sizeBorder( sizeTotal - sizeText );

	int initialX = sizeBorder.cx / 2;
	int initialY = sizeBorder.cy / 2;
	int deltaY = sizeText.cy / numLines;
	
	if (m_isCentered)
	{	
		// Determine how the lines will be spaced vertically
		std::wstring firstLine;
		RemoveFirstLine(utilString, firstLine);
		CSize const firstLineSize = g_pApp->GetFontHandler()->GetStringSize( firstLine, m_style );
		CSize printArea = sizeTotal - sizeBorder;
		bool const fitsVertically = printArea.cy >= firstLineSize.cy * numLines;

		if (fitsVertically)
		{	// Enough room, place lines in the center of the print area
			initialY += (printArea.cy - firstLineSize.cy * numLines) / 2;
			deltaY = firstLineSize.cy;
		}
		else
		{	// Not enough room, squeeze lines on top of each other
			deltaY = printArea.cy / numLines;
			initialY += (deltaY - firstLineSize.cy) / 2;
		}
	}
	else
	{
		deltaY = g_pApp->GetFontHandler()->GetHeight( m_style );
	}

	//Draw each line one at a time
	for(int i=0; i<numLines; ++i)
	{
		std::wstring line;
		RemoveFirstLine(multiline, line);
		
		int x = initialX;
		int y = initialY + i * deltaY;

		if (line.length())
		{
			if ( m_isCentered )
			{
				CSize sizeText = g_pApp->GetFontHandler()->GetStringSize( line, m_style );
				x = x + ( sizeTotal.cx - sizeBorder.cx - sizeText.cx ) / 2;
			}

			if(FAILED(g_pApp->GetFontHandler()->DrawText( this, line, m_style, CPoint( x, y ) ) ) )
				return false;
		}
	}

    return true;
}


HRESULT TextSprite::VOnRender(double fTime, float fElapsedTime)
{
	CPoint actualPos( m_Position - m_Hotspot );
	CRect sourceRect( CPoint(0,0), CPoint(m_Width-1, m_Height-1 ) );

	// Adjust for the frame
	sourceRect.top += ( m_CurrentFrame * m_TextureHeight );
	sourceRect.bottom += ( m_CurrentFrame * m_TextureHeight );

	Vec3 center(1.0f, 1.0f, 0.0f);
	Vec3 translation((const float)actualPos.x, (const float)actualPos.y, 0.0f);

	if (m_d3dSprite && m_pTexture)
	{
		m_d3dSprite->Begin(D3DXSPRITE_ALPHABLEND);
		VOnRestore();
		m_d3dSprite->End();
	}

	return S_OK;
}

void TextSprite::SetText(const std::wstring text)
{
	if (text != m_text)
	{
		m_text = text;
		VOnRestore();
	}
}



