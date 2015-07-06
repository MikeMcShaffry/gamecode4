md ..\Lib\Win32Debug
md ..\Lib\Win32Release
md ..\Lib\Win32Profile
md ..\Lib\x64Debug
md ..\Lib\x64Release
md ..\Lib\x64Profile

echo f | xcopy /D /y DXUT11\Core\Debug\dxut.lib ..\..\Lib\Win32Debug\dxut.lib
echo f | xcopy /D /y DXUT11\Core\Debug\dxut.pdb ..\..\Lib\Win32Debug\dxut.pdb
echo f | xcopy /D /y DXUT11\Core\Release\dxut.lib ..\..\Lib\Win32Release\dxut.lib
echo f | xcopy /D /y DXUT11\Core\Release\dxut.pdb ..\..\Lib\Win32Release\dxut.pdb
echo f | xcopy /D /y DXUT11\Core\Profile\dxut.lib ..\..\Lib\Win32Profile\dxut.lib
echo f | xcopy /D /y DXUT11\Core\Profile\dxut.pdb ..\..\Lib\Win32Profile\dxut.pdb

echo f | xcopy /D /y DXUT11\Optional\Debug\dxutopt.lib ..\..\Lib\Win32Debug\dxutopt.lib
echo f | xcopy /D /y DXUT11\Optional\Debug\dxutopt.pdb ..\..\Lib\Win32Debug\dxutopt.pdb
echo f | xcopy /D /y DXUT11\Optional\Release\dxutopt.lib ..\..\Lib\Win32Release\dxutopt.lib
echo f | xcopy /D /y DXUT11\Optional\Release\dxutopt.pdb ..\..\Lib\Win32Release\dxutopt.pdb
echo f | xcopy /D /y DXUT11\Optional\Profile\dxutopt.lib ..\..\Lib\Win32Profile\dxutopt.lib
echo f | xcopy /D /y DXUT11\Optional\Profile\dxutopt.pdb ..\..\Lib\Win32Profile\dxutopt.pdb

echo f | xcopy /D /y DXUT11\Effects11\Debug\effects11.lib ..\..\Lib\Win32Debug\effects11.lib
echo f | xcopy /D /y DXUT11\Effects11\Debug\effects11.pdb ..\..\Lib\Win32Debug\effects11.pdb
echo f | xcopy /D /y DXUT11\Effects11\Release\effects11.lib ..\..\Lib\Win32Release\effects11.lib
echo f | xcopy /D /y DXUT11\Effects11\Release\effects11.pdb ..\..\Lib\Win32Release\effects11.pdb
echo f | xcopy /D /y DXUT11\Effects11\Profile\effects11.lib ..\..\Lib\Win32Profile\effects11.lib
echo f | xcopy /D /y DXUT11\Effects11\Profile\effects11.pdb ..\..\Lib\Win32Profile\effects11.pdb

echo f | xcopy /D /y DXUT11\Core\x64\Debug\dxut.lib ..\..\Lib\x64Debug\dxut.lib
echo f | xcopy /D /y DXUT11\Core\x64\Debug\dxut.pdb ..\..\Lib\x64Debug\dxut.pdb
echo f | xcopy /D /y DXUT11\Core\x64\Release\dxut.lib ..\..\Lib\x64Release\dxut.lib
echo f | xcopy /D /y DXUT11\Core\x64\Release\dxut.pdb ..\..\Lib\x64Release\dxut.pdb
echo f | xcopy /D /y DXUT11\Core\x64\Profile\dxut.lib ..\..\Lib\x64Profile\dxut.lib
echo f | xcopy /D /y DXUT11\Core\x64\Profile\dxut.pdb ..\..\Lib\x64Profile\dxut.pdb

echo f | xcopy /D /y DXUT11\Optional\x64\Debug\dxutopt.lib ..\..\Lib\x64Debug\dxutopt.lib
echo f | xcopy /D /y DXUT11\Optional\x64\Debug\dxutopt.pdb ..\..\Lib\x64Debug\dxutopt.pdb
echo f | xcopy /D /y DXUT11\Optional\x64\Release\dxutopt.lib ..\..\Lib\x64Release\dxutopt.lib
echo f | xcopy /D /y DXUT11\Optional\x64\Release\dxutopt.pdb ..\..\Lib\x64Release\dxutopt.pdb
echo f | xcopy /D /y DXUT11\Optional\x64\Profile\dxutopt.lib ..\..\Lib\x64Profile\dxutopt.lib
echo f | xcopy /D /y DXUT11\Optional\x64\Profile\dxutopt.pdb ..\..\Lib\x64Profile\dxutopt.pdb

echo f | xcopy /D /y DXUT11\Effects11\x64\Debug\effects11.lib ..\..\Lib\x64Debug\effects11.lib
echo f | xcopy /D /y DXUT11\Effects11\x64\Debug\effects11.pdb ..\..\Lib\x64Debug\effects11.pdb
echo f | xcopy /D /y DXUT11\Effects11\x64\Release\effects11.lib ..\..\Lib\x64Release\effects11.lib
echo f | xcopy /D /y DXUT11\Effects11\x64\Release\effects11.pdb ..\..\Lib\x64Release\effects11.pdb
echo f | xcopy /D /y DXUT11\Effects11\x64\Profile\effects11.lib ..\..\Lib\x64Profile\effects11.lib
echo f | xcopy /D /y DXUT11\Effects11\x64\Profile\effects11.pdb ..\..\Lib\x64Profile\effects11.pdb

echo d | xcopy /D /y bullet-2.79\lib\BulletCollision.lib ..\..\Lib\Win32Release
echo d | xcopy /D /y bullet-2.79\lib\BulletCollision.pdb ..\..\Lib\Win32Release
echo d | xcopy /D /y bullet-2.79\lib\BulletCollision.lib ..\..\Lib\Win32Profile
echo d | xcopy /D /y bullet-2.79\lib\BulletCollision.pdb ..\..\Lib\Win32Profile
echo d | xcopy /D /y bullet-2.79\lib\BulletCollision_debug.lib ..\..\Lib\Win32Debug
echo d | xcopy /D /y bullet-2.79\lib\BulletCollision_debug.pdb ..\..\Lib\Win32Debug
echo d | xcopy /D /y bullet-2.79\lib\BulletCollision_x64.lib ..\..\Lib\x64Release
echo d | xcopy /D /y bullet-2.79\lib\BulletCollision_x64.pdb ..\..\Lib\x64Release
echo d | xcopy /D /y bullet-2.79\lib\BulletCollision_x64.lib ..\..\Lib\x64Profile
echo d | xcopy /D /y bullet-2.79\lib\BulletCollision_x64.pdb ..\..\Lib\x64Profile
echo d | xcopy /D /y bullet-2.79\lib\BulletCollision_x64_debug.lib ..\..\Lib\x64Debug
echo d | xcopy /D /y bullet-2.79\lib\BulletCollision_x64_debug.pdb ..\..\Lib\x64Debug

echo d | xcopy /D /y bullet-2.79\lib\BulletDynamics.lib ..\..\Lib\Win32Release
echo d | xcopy /D /y bullet-2.79\lib\BulletDynamics.pdb ..\..\Lib\Win32Release
echo d | xcopy /D /y bullet-2.79\lib\BulletDynamics.lib ..\..\Lib\Win32Profile
echo d | xcopy /D /y bullet-2.79\lib\BulletDynamics.pdb ..\..\Lib\Win32Profile
echo d | xcopy /D /y bullet-2.79\lib\BulletDynamics_debug.lib ..\..\Lib\Win32Debug
echo d | xcopy /D /y bullet-2.79\lib\BulletDynamics_debug.pdb ..\..\Lib\Win32Debug
echo d | xcopy /D /y bullet-2.79\lib\BulletDynamics_x64.lib ..\..\Lib\x64Release
echo d | xcopy /D /y bullet-2.79\lib\BulletDynamics_x64.pdf ..\..\Lib\x64Release
echo d | xcopy /D /y bullet-2.79\lib\BulletDynamics_x64.lib ..\..\Lib\x64Profile
echo d | xcopy /D /y bullet-2.79\lib\BulletDynamics_x64.pdb ..\..\Lib\x64Profile
echo d | xcopy /D /y bullet-2.79\lib\BulletDynamics_x64_debug.lib ..\..\Lib\x64Debug
echo d | xcopy /D /y bullet-2.79\lib\BulletDynamics_x64_debug.pdb ..\..\Lib\x64Debug

echo d | xcopy /D /y bullet-2.79\lib\LinearMath.lib ..\..\Lib\Win32Release
echo d | xcopy /D /y bullet-2.79\lib\LinearMath.pdb ..\..\Lib\Win32Release
echo d | xcopy /D /y bullet-2.79\lib\LinearMath.lib ..\..\Lib\Win32Profile
echo d | xcopy /D /y bullet-2.79\lib\LinearMath.pdb ..\..\Lib\Win32Profile
echo d | xcopy /D /y bullet-2.79\lib\LinearMath_debug.lib ..\..\Lib\Win32Debug
echo d | xcopy /D /y bullet-2.79\lib\LinearMath_debug.pdb ..\..\Lib\Win32Debug
echo d | xcopy /D /y bullet-2.79\lib\LinearMath_x64.lib ..\..\Lib\x64Release
echo d | xcopy /D /y bullet-2.79\lib\LinearMath_x64.pdb ..\..\Lib\x64Release
echo d | xcopy /D /y bullet-2.79\lib\LinearMath_x64.lib ..\..\Lib\x64Profile
echo d | xcopy /D /y bullet-2.79\lib\LinearMath_x64.pdb ..\..\Lib\x64Profile
echo d | xcopy /D /y bullet-2.79\lib\LinearMath_x64_debug.lib ..\..\Lib\x64Debug
echo d | xcopy /D /y bullet-2.79\lib\LinearMath_x64_debug.pdb ..\..\Lib\x64Debug

echo f | xcopy /D /y libogg-1.3.0\win32\VS2010\Win32\Debug\libogg_static.lib ..\..\Lib\Win32Debug\libogg_static.lib
echo f | xcopy /D /y libogg-1.3.0\win32\VS2010\Win32\Release\libogg_static.lib ..\..\Lib\Win32Release\libogg_static.lib
echo f | xcopy /D /y libogg-1.3.0\win32\VS2010\Win32\Release\libogg_static.lib ..\..\Lib\Win32Profile\libogg_static.lib
echo f | xcopy /D /y libogg-1.3.0\win32\VS2010\x64\Debug\libogg_static.lib ..\Lib\x64Debug\libogg_static.lib
echo f | xcopy /D /y libogg-1.3.0\win32\VS2010\x64\Release\libogg_static.lib ..\..\Lib\x64Release\libogg_static.lib 
echo f | xcopy /D /y libogg-1.3.0\win32\VS2010\x64\Release\libogg_static.lib ..\..\Lib\x64Profile\libogg_static.lib 

echo f | xcopy /D /y libogg-1.3.0\win32\VS2010\Win32\Debug\libogg_static.pdb ..\..\Lib\Win32Debug\libogg_static.pdb
echo f | xcopy /D /y libogg-1.3.0\win32\VS2010\Win32\Release\libogg_static.pdb ..\..\Lib\Win32Release\libogg_static.pdb
echo f | xcopy /D /y libogg-1.3.0\win32\VS2010\Win32\Release\libogg_static.pdb ..\..\Lib\Win32Profile\libogg_static.pdb
echo f | xcopy /D /y libogg-1.3.0\win32\VS2010\x64\Debug\libogg_static.pdb ..\Lib\x64Debug\libogg_static.pdb
echo f | xcopy /D /y libogg-1.3.0\win32\VS2010\x64\Release\libogg_static.pdb ..\..\Lib\x64Release\libogg_static.pdb 
echo f | xcopy /D /y libogg-1.3.0\win32\VS2010\x64\Release\libogg_static.pdb ..\..\Lib\x64Profile\libogg_static.pdb


echo f | xcopy /D /y libvorbis-1.3.2\win32\VS2010\Win32\Debug\libvorbis_static.lib ..\..\Lib\Win32Debug\libvorbis_static.lib
echo f | xcopy /D /y libvorbis-1.3.2\win32\VS2010\Win32\Release\libvorbis_static.lib ..\..\Lib\Win32Release\libvorbis_static.lib
echo f | xcopy /D /y libvorbis-1.3.2\win32\VS2010\Win32\Release\libvorbis_static.lib ..\..\Lib\Win32Profile\libvorbis_static.lib
echo f | xcopy /D /y libvorbis-1.3.2\win32\VS2010\x64\Debug\libvorbis_static.lib ..\..\Lib\x64Debug\libvorbis_static.lib
echo f | xcopy /D /y libvorbis-1.3.2\win32\VS2010\x64\Release\libvorbis_static.lib ..\..\Lib\x64Release\libvorbis_static.lib
echo f | xcopy /D /y libvorbis-1.3.2\win32\VS2010\x64\Release\libvorbis_static.lib ..\..\Lib\x64Profile\libvorbis_static.lib

echo f | xcopy /D /y libvorbis-1.3.2\win32\VS2010\Win32\Debug\libvorbis_static.pdb ..\..\Lib\Win32Debug\libvorbis_static.pdb
echo f | xcopy /D /y libvorbis-1.3.2\win32\VS2010\Win32\Release\libvorbis_static.pdb ..\..\Lib\Win32Release\libvorbis_static.pdb
echo f | xcopy /D /y libvorbis-1.3.2\win32\VS2010\Win32\Release\libvorbis_static.pdb ..\..\Lib\Win32Profile\libvorbis_static.pdb
echo f | xcopy /D /y libvorbis-1.3.2\win32\VS2010\x64\Debug\libvorbis_static.pdb ..\..\Lib\x64Debug\libvorbis_static.pdb
echo f | xcopy /D /y libvorbis-1.3.2\win32\VS2010\x64\Release\libvorbis_static.pdb ..\..\Lib\x64Release\libvorbis_static.pdb
echo f | xcopy /D /y libvorbis-1.3.2\win32\VS2010\x64\Release\libvorbis_static.pdb ..\..\Lib\x64Profile\libvorbis_static.pdb

echo f | xcopy /D /y libvorbis-1.3.2\win32\VS2010\Win32\Debug\libvorbisfile_static.lib ..\..\Lib\Win32Debug\libvorbisfile_static.lib
echo f | xcopy /D /y libvorbis-1.3.2\win32\VS2010\Win32\Release\libvorbisfile_static.lib ..\..\Lib\Win32Release\libvorbisfile_static.lib
echo f | xcopy /D /y libvorbis-1.3.2\win32\VS2010\Win32\Release\libvorbisfile_static.lib ..\..\Lib\Win32Profile\libvorbisfile_static.lib
echo f | xcopy /D /y libvorbis-1.3.2\win32\VS2010\x64\Debug\libvorbisfile_static.lib ..\..\Lib\x64Debug\libvorbisfile_static.lib
echo f | xcopy /D /y libvorbis-1.3.2\win32\VS2010\x64\Release\libvorbisfile_static.lib ..\..\Lib\x64Release\libvorbisfile_static.lib
echo f | xcopy /D /y libvorbis-1.3.2\win32\VS2010\x64\Release\libvorbisfile_static.lib ..\..\Lib\x64Profile\libvorbisfile_static.lib

echo f | xcopy /D /y libvorbis-1.3.2\win32\VS2010\Win32\Debug\libvorbisfile_static.pdb ..\..\Lib\Win32Debug\libvorbisfile_static.pdb
echo f | xcopy /D /y libvorbis-1.3.2\win32\VS2010\Win32\Release\libvorbisfile_static.pdb ..\..\Lib\Win32Release\libvorbisfile_static.pdb
echo f | xcopy /D /y libvorbis-1.3.2\win32\VS2010\Win32\Release\libvorbisfile_static.pdb ..\..\Lib\Win32Profile\libvorbisfile_static.pdb
echo f | xcopy /D /y libvorbis-1.3.2\win32\VS2010\x64\Debug\libvorbisfile_static.pdb ..\..\Lib\x64Debug\libvorbisfile_static.pdb
echo f | xcopy /D /y libvorbis-1.3.2\win32\VS2010\x64\Release\libvorbisfile_static.pdb ..\..\Lib\x64Release\libvorbisfile_static.pdb
echo f | xcopy /D /y libvorbis-1.3.2\win32\VS2010\x64\Release\libvorbisfile_static.pdb ..\..\Lib\x64Profile\libvorbisfile_static.pdb

echo f | xcopy /D /y tinyxml_2_6_2\Debugtinyxml\tinyxml.lib ..\..\Lib\Win32Debug\tinyxml.lib
echo f | xcopy /D /y tinyxml_2_6_2\Releasetinyxml\tinyxml.lib ..\..\Lib\Win32Release\tinyxml.lib
echo f | xcopy /D /y tinyxml_2_6_2\Releasetinyxml\tinyxml.lib ..\..\Lib\Win32Profile\tinyxml.lib
echo f | xcopy /D /y tinyxml_2_6_2\Debugtinyxml\tinyxml.pdb ..\..\Lib\Win32Debug\tinyxml.pdb
echo f | xcopy /D /y tinyxml_2_6_2\Releasetinyxml\tinyxml.pdb ..\..\Lib\Win32Release\tinyxml.pdb
echo f | xcopy /D /y tinyxml_2_6_2\Releasetinyxml\tinyxml.pdb ..\..\Lib\Win32Profile\tinyxml.pdb


echo f | xcopy /y zlib-1.2.5\contrib\vstudio\vc10\x86\ZlibStatDebug\zlibstat.lib ..\..\Lib\Win32Debug\zlibstat.lib
echo f | xcopy /y zlib-1.2.5\contrib\vstudio\vc10\x86\ZlibStatRelease\zlibstat.lib ..\..\Lib\Win32Release\zlibstat.lib
echo f | xcopy /y zlib-1.2.5\contrib\vstudio\vc10\x86\ZlibStatRelease\zlibstat.lib ..\..\Lib\Win32Profile\zlibstat.lib

echo f | xcopy /y luaplus51-all\bin.vs2010.win32\luaplus51-1201.lib ..\..\Lib\Win32Debug\luaplus51-1201.lib
echo f | xcopy /y luaplus51-all\bin.vs2010.win32\luaplus51-1201.lib ..\..\Lib\Win32Release\luaplus51-1201.lib
echo f | xcopy /y luaplus51-all\bin.vs2010.win32\luaplus51-1201.lib ..\..\Lib\Win32Profile\luaplus51-1201.lib

echo f | xcopy /y luaplus51-all\bin.vs2010.win32\luaplus51-1201.lib ..\..\Lib\Win32Debug\luaplus51-1201.pdb
echo f | xcopy /y luaplus51-all\bin.vs2010.win32\luaplus51-1201.lib ..\..\Lib\Win32Release\luaplus51-1201.pdb
echo f | xcopy /y luaplus51-all\bin.vs2010.win32\luaplus51-1201.lib ..\..\Lib\Win32Profile\luaplus51-1201.pdb

echo f | xcopy /y luaplus51-all\bin.vs2010.win32\luaplus51-1201.dll ..\..\Lib\Win32Debug\luaplus51-1201.dll
echo f | xcopy /y luaplus51-all\bin.vs2010.win32\luaplus51-1201.dll ..\..\Lib\Win32Release\luaplus51-1201.dll
echo f | xcopy /y luaplus51-all\bin.vs2010.win32\luaplus51-1201.dll ..\..\Lib\Win32Profile\luaplus51-1201.dll



