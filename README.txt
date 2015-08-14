Welcome to the Game Coding Complete 4th Edition Source Code!

Introduction
===============================================================================
GameCode4 is the sample application that encapsulates much of the source code
discussed in "Game Coding Complete - 4th Edition" by Mike McShaffry and David
"Rez" Graham, published by Charles River Media. 
ISBN-10: 1133776574 | ISBN-13: 978-1133776574

If this source code has found it's way to you, and you think it has helped you
in any way, do the authors a favor and buy a new copy of the book - there are 
detailed explanations in it that compliment this code well. Buy a copy at Amazon.com
by clicking here: 
   http://www.amazon.com/gp/product/1133776574/ref=olp_product_details?ie=UTF8&me=&seller=

There's a companion web site at http://www.mcshaffry.com/GameCode/

The source code is managed and maintained through Google Code: 
   http://code.google.com/p/gamecode4/

   (c) Copyright 2012 Michael L. McShaffry and David Graham

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser GPL v3
as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See 
http://www.gnu.org/licenses/lgpl-3.0.txt for more details.

You should have received a copy of the GNU Lesser GPL v3
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

 
Getting Started
===============================================================================

Requirements:
  1. Visual Studio 2013 (and some knowledge on how to build, run, and debug C++/C# projects)
  2. Microsoft DirectX SDK (June 2010 or later) 
  3. The source code from Google Code - (TortoiseSVN is a good client to use!)
        svn checkout http://gamecode4.googlecode.com/svn/trunk/ gamecode4-read-only
  4. 3rdParty.zip from http://code.google.com/p/gamecode4/downloads/list
        Unzip it into the root directory of the project - it will add directories to 
        Source.
  
Building Teapot Wars:
  1. Make sure you can build a DirectX SDK sample and run it!
  2. Make sure your directory structure for GameCode4 looks like this:
          GameCode4
		  |-- Dev (some people also name this "trunk" it depends on how you set up SVN for this project)
                      |-- Assets (with Actors, Art, Audio, etc. - all of the source assets for TeapotWars)
                      |-- Extra (contains UtilityDemo, an AI playground)
                      |-- Game (contains the files needed to run TeapotWars)
                      |-- Source (the entire source tree, including GCC4\3rdParty directory from the ZIP file)
  3. Open Source\TeapotWars\Msvc\TeapotWars_2013.sln and rebuild everything.
  4. In the Solution Explorer in VS2013, right click on TeapotWars_2013 and....
      a. ...set it as your startup project. Then, right click on it again and...
	  b. open the Properties dialog, and...
	  c. set the Configuration to All Configurations and Platform to All Platforms, and...
	  d. Click on Configuration Properties->Debugging, and set the Working Directory to ..\..\..\Game
	  e. Hit OK
  5. Set the Target to Debug, and Platform to Win32 (up in toolbar beneath the menu)
  6. Select Build->Rebuild Solution from the menu.
  7. Run.

  IMPORTANT! If your machine does not support D3D11, you must edit Game\PlayerOptions.xml, and change
  renderer="Direct3D 11"  to renderer="Direct3D 9" 

Running TeapotWarsEditorApp_2013 project
  1. Make sure you ran Teapot Wars first!
  2. Right click on the project and set is as your startup project.
  3. Right click on the project and select Properties.
  4. In the Properties window, select the Debug tab, and set the Working Directory to the "Game" directory.
  5. Run.
  
  


  




