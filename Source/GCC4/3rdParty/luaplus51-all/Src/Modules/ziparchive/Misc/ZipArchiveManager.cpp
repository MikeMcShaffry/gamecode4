///////////////////////////////////////////////////////////////////////////////
// $Workfile: VirtualDriveManager.cpp $
// $Archive: /WorkspaceWhiz/Src/Shared/VirtualDriveManager.cpp $
// $Date: 2003/01/05 $ $Revision: #5 $ $Author: Joshua $
///////////////////////////////////////////////////////////////////////////////
// This source file is part of the Workspace Whiz source distribution and
// is Copyright 1997-2003 by Joshua C. Jensen.  (http://workspacewhiz.com/)
//
// The code presented in this file may be freely used and modified for all
// non-commercial and commercial purposes so long as due credit is given and
// this header is left intact.
///////////////////////////////////////////////////////////////////////////////
#include "Misc_InternalPch.h"
#include "VirtualDriveManager.h"
#include "VirtualDrive.h"

namespace Misc {

VirtualDriveManager* VirtualDriveManager::s_instance;

VirtualDriveManager* VirtualDriveManager::GetInstance()
{
    return s_instance ? s_instance : new VirtualDriveManager;
}

///////////////////////////////////////////////////////////////////////////////
/**
	The constructor.
**/
VirtualDriveManager::VirtualDriveManager()
{
	assert(!s_instance);
	s_instance = this;
} // VirtualDriveManager()


///////////////////////////////////////////////////////////////////////////////
/**
**/
VirtualDriveManager::~VirtualDriveManager()
{
	CloseAll();
} // ~VirtualDriveManager()


///////////////////////////////////////////////////////////////////////////////
/**
**/
VirtualDrive* VirtualDriveManager::Open(const AnsiString& fileName, bool create)
{
	// Save the fileName.
	DriveInfo driveInfo;
	driveInfo.m_fileName = fileName;
//	driveInfo.m_fileName = driveInfo.m_fileName.Lower();

	// Search the list.  See if fileName is already open.
	AnsiString lowerFileName = fileName.Replace('\\', '/');
    DriveInfoMap::Node* node = this->openDriveInfo.Find(lowerFileName);
	if (node)
	{
		DriveInfo& testInfo = this->openDriveInfo.Value(node);
		return testInfo.m_drive;
	}

	// It isn't open.  Make a new entry.
	driveInfo.m_drive.Own(new VirtualDrive);
	if (!driveInfo.m_drive->Open(driveInfo.m_fileName, false))
	{
		if (create)
		{
			if (!driveInfo.m_drive->Create(driveInfo.m_fileName))
			{
				return NULL;
			}
		}
		else
			return NULL;
	}

	VirtualDrive* drive = driveInfo.m_drive;
	this->openDriveInfo[lowerFileName] = driveInfo;

	// Opened successfully!
	return drive;
} // Open()


///////////////////////////////////////////////////////////////////////////////
/**
	Close all open virtual drives.
**/
bool VirtualDriveManager::CloseAll()
{
	this->openDriveInfo.Clear();

	return true;
} // CloseAll()


/**
	Close all open virtual drives.
**/
void VirtualDriveManager::PackAll()
{
    for (DriveInfoMap::Node* node = this->openDriveInfo.Head(); node; node = this->openDriveInfo.Next(node))
	{
		DriveInfo& driveInfo = this->openDriveInfo.Value(node);
		driveInfo.m_drive->Flush();
		driveInfo.m_drive->Pack();
	}
}


void VirtualDriveManager::ParseFilename(const AnsiString& fileName,
			AnsiString& virtualDriveName, AnsiString& virtualFileName)
{
	int pipePos = fileName.Find('|');
	if (pipePos == -1)
	{
		virtualDriveName.Clear();
		virtualFileName.Clear();

		return;
	}

	virtualDriveName = fileName.Sub(0, pipePos);
	virtualFileName = fileName.Sub(pipePos + 1);
} // ParseFilename()

} // namespace Misc
