
#
# copyalllibs_vs2013.ps1
#
# Copies all of the 3rdParty library files from their compile targets in the 3rdParty directory to
# the Libs directory. The Libs directory includes both vs2010 and vs2013 versions. vs2010 has a batch file
# called copyalllibs_vs2010.bat that is used for libraries compiled under vs2010.
#
# Libs\vs2013 contains directories for Win32 and x64 platforms, in each of Debug, Release, and Profile configurations.
# GameCode 4 expects all of the .lib files to have the same name regardless of platform or configuration, as this makes 
# maintaining the project a little easier. 
#
# When new libraries are added, it is the intention of this script to allow the library source to compile to whatever targets it
# was originally designed to, and this script will perform the copy and if necessary rename operation on library files and
# companion pdb files.
#
# The only changes typically made to library projects is to create a companion pdb file to go with the library, as the default in 
# Visual Studio is to name it vs120.pdb, which isn't terribly useful.  
#
#


function CopyFiles($baseName, $targetName, $cfg, $win32src, $x64src)
{
	"Copy $baseName"
	$exts = ".lib", ".pdb" 
	foreach ($ext in $exts)
	{
		Copy-Item -Path $win32src\$baseName$ext -Destination Lib\Win32$cfg\$targetName$ext
		Copy-Item -Path $x64src\$baseName$ext Lib\x64$cfg\$targetName$ext
		
		# always copy the Release version of libraries to the Profile configuration in GCC
		if ($cfg -eq "Release")
		{
			Copy-Item -Path $win32src\$baseName$ext -Destination Lib\Win32Profile\$targetName$ext
			Copy-Item -Path $x64src\$baseName$ext Lib\x64Profile\$targetName$ext		
		}
	}
}


function CopyFile($src, $target)
{
	$exts = ".lib", ".pdb" 
	foreach ($ext in $exts)
	{
		Copy-Item -Path $src$ext $target$ext
	}
}


#
# Main Routine
#

# Profile is a special confinguration in GameCode that is just a copy of the Release libraries.
$configurations = "Debug", "Release"
$platforms = "Win32", "x64"

# Create the directories under Libs if they do not exist.
foreach ($cfg in $configurations)
{
	foreach ($platform in $platforms)
	{
		New-Item -ErrorAction SilentlyContinue  -ItemType directory Lib\$platform$cfg	
		if ($cfg -eq "Release")
		{
			New-Item -ErrorAction SilentlyContinue  -ItemType directory Lib\$($platform)Profile		
		}
	}
}

#
# Most libraries compile to different directories for each platform (Win32, x64) and configuration (Debug, Release).
# This loop uses the utility function CopyFiles to copy the .lib .pdb from the library's compiled directory to the right spot in Libs.
#
foreach ($cfg in $configurations)
{
	# DirectX
	CopyFiles "dxut" "dxut" $cfg DXUT11\Core\$cfg DXUT11\Core\x64\$cfg
	CopyFiles "dxutopt" "dxutopt" $cfg DXUT11\Optional\$cfg DXUT11\Optional\x64\$cfg
	CopyFiles "effects11" "effects11" $cfg DXUT11\Effects11\$cfg DXUT11\Effects11\x64\$cfg

	# Ogg and Vorbis
	CopyFiles libogg_static libogg_static $cfg libogg-1.3.0\win32\VS2013\Win32\$cfg libogg-1.3.0\win32\VS2013\x64\$cfg
	CopyFiles libvorbis_static libvorbis_static $cfg libvorbis-1.3.2\win32\VS2013\Win32\$cfg  libvorbis-1.3.2\win32\VS2013\x64\$cfg
	CopyFiles libvorbisfile_static libvorbisfile_static $cfg libvorbis-1.3.2\win32\VS2013\Win32\$cfg  libvorbis-1.3.2\win32\VS2013\x64\$cfg
	
	# TinyXML
	CopyFiles tinyxml tinyxml $cfg tinyxml_2_6_2\Win32\$cfg  tinyxml_2_6_2\x64\$cfg

	# ZLib
	CopyFiles zlibstat zlibstat $cfg zlib-1.2.8\contrib\vstudio\vc12\x86\ZlibStat$cfg zlib-1.2.8\contrib\vstudio\vc12\x64\ZlibStat$cfg 

	# LuaPlus
	CopyFiles luaplus luaplus $cfg luaplus51-all\Src\LuaPlus_2013\$cfg luaplus51-all\Src\LuaPlus_2013\x64\$cfg	
}
	
# Bullet is a little ill bebahved, since they have only one library directory and 
# rename all the library targets depending on which platform and configuration you are using.
# This requires us to copy and rename the files. The CopyFile utility method assumes the lib and pdb are named the same.

CopyFile  bullet-2.82\lib\BulletCollision_vs2013 Lib\Win32Release\BulletCollision
CopyFile  bullet-2.82\lib\BulletCollision_vs2013 Lib\Win32Profile\BulletCollision
CopyFile  bullet-2.82\lib\BulletCollision_vs2013_debug Lib\Win32Debug\BulletCollision
CopyFile bullet-2.82\lib\BulletCollision_vs2013_x64_release Lib\x64Release\BulletCollision
CopyFile  bullet-2.82\lib\BulletCollision_vs2013_x64_release Lib\x64Profile\BulletCollision
CopyFile bullet-2.82\lib\BulletCollision_vs2013_x64_debug Lib\x64Debug\BulletCollision

CopyFile  bullet-2.82\lib\BulletDynamics_vs2013 Lib\Win32Release\BulletDynamics
CopyFile  bullet-2.82\lib\BulletDynamics_vs2013 Lib\Win32Profile\BulletDynamics
CopyFile  bullet-2.82\lib\BulletDynamics_vs2013_debug Lib\Win32Debug\BulletDynamics
CopyFile  bullet-2.82\lib\BulletDynamics_vs2013_x64_release Lib\x64Release\BulletDynamics
CopyFile  bullet-2.82\lib\BulletDynamics_vs2013_x64_release Lib\x64Profile\BulletDynamics
CopyFile  bullet-2.82\lib\BulletDynamics_vs2013_x64_debug Lib\x64Debug\BulletDynamics

CopyFile  bullet-2.82\lib\LinearMath_vs2013 Lib\Win32Release\LinearMath
CopyFile  bullet-2.82\lib\LinearMath_vs2013 Lib\Win32Profile\LinearMath
CopyFile  bullet-2.82\lib\LinearMath_vs2013_debug Lib\Win32Debug\LinearMath
CopyFile  bullet-2.82\lib\LinearMath_vs2013_x64_release Lib\x64Release\LinearMath
CopyFile  bullet-2.82\lib\LinearMath_vs2013_x64_release Lib\x64Profile\LinearMath
CopyFile  bullet-2.82\lib\LinearMath_vs2013_x64_debug Lib\x64Debug\LinearMath



