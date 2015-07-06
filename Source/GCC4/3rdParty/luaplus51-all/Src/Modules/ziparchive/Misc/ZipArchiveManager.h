///////////////////////////////////////////////////////////////////////////////
// $Workfile: VirtualDriveManager.h $
// $Archive: /WorkspaceWhiz/Src/Shared/VirtualDriveManager.h $
// $Date: 2003/01/05 $ $Revision: #5 $ $Author: Joshua $
///////////////////////////////////////////////////////////////////////////////
// This source file is part of the Workspace Whiz source distribution and
// is Copyright 1997-2003 by Joshua C. Jensen.  (http://workspacewhiz.com/)
//
// The code presented in this file may be freely used and modified for all
// non-commercial and commercial purposes so long as due credit is given and
// this header is left intact.
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Misc.h"
#include "AnsiString.h"
#include "Map.h"
#include "OwnedPtr.h"

namespace Misc {

class VirtualDrive;

/**
**/
class VirtualDriveManager
{
public:
	static VirtualDriveManager* GetInstance();

	VirtualDriveManager();
	virtual ~VirtualDriveManager();

    virtual void ParseFilename(const AnsiString& fileName, AnsiString& virtualDriveName, AnsiString& virtualFileName);

	VirtualDrive* Open(const AnsiString& fileName, bool create = false);
	bool CloseAll();
	void PackAll();

private:
	class DriveInfo
	{
	public:
		AnsiString m_fileName;
		OwnedPtr<VirtualDrive> m_drive;
	};

    typedef Map<AnsiString, DriveInfo> DriveInfoMap;
    DriveInfoMap openDriveInfo;

	static VirtualDriveManager* s_instance;
};

} // namespace Misc
