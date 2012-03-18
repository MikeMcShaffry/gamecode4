//========================================================================
// XmlResource.cpp : API to use load Xml files from the Resource Cache
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

#include "GameCodeStd.h"
#include "XmlResource.h"

void XmlResourceExtraData::ParseXml(char* pRawBuffer)
{
    m_xmlDocument.Parse(pRawBuffer);
}


bool XmlResourceLoader::VLoadResource(char *rawBuffer, unsigned int rawSize, shared_ptr<ResHandle> handle)
{
    if (rawSize <= 0)
        return false;

    shared_ptr<XmlResourceExtraData> pExtraData = shared_ptr<XmlResourceExtraData>(GCC_NEW XmlResourceExtraData());
    pExtraData->ParseXml(rawBuffer);

    handle->SetExtra(shared_ptr<XmlResourceExtraData>(pExtraData));

    return true;
}


shared_ptr<IResourceLoader> CreateXmlResourceLoader()
{
    return shared_ptr<IResourceLoader>(GCC_NEW XmlResourceLoader());
}


TiXmlElement* XmlResourceLoader::LoadAndReturnRootXmlElement(const char* resourceString)
{
    Resource resource(resourceString);
    shared_ptr<ResHandle> pResourceHandle = g_pApp->m_ResCache->GetHandle(&resource);  // this actually loads the XML file from the zip file
    shared_ptr<XmlResourceExtraData> pExtraData = static_pointer_cast<XmlResourceExtraData>(pResourceHandle->GetExtra());
    return pExtraData->GetRoot();
}


