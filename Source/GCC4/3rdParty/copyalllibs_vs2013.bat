md ..\..\Lib\vs2013\Win32Debug
md ..\..\Lib\vs2013\Win32Release
md ..\..\Lib\vs2013\Win32Profile
md ..\..\Lib\vs2013\x64Debug
md ..\..\Lib\vs2013\x64Release
md ..\..\Lib\vs2013\x64Profile

echo f | xcopy /D /y DXUT11\Core\Debug\dxut.lib ..\..\Lib\vs2013\Win32Debug\dxut.lib
echo f | xcopy /D /y DXUT11\Core\Debug\dxut.pdb ..\..\Lib\vs2013\Win32Debug\dxut.pdb
echo f | xcopy /D /y DXUT11\Core\Release\dxut.lib ..\..\Lib\vs2013\Win32Release\dxut.lib
echo f | xcopy /D /y DXUT11\Core\Release\dxut.pdb ..\..\Lib\vs2013\Win32Release\dxut.pdb
echo f | xcopy /D /y DXUT11\Core\Profile\dxut.lib ..\..\Lib\vs2013\Win32Profile\dxut.lib
echo f | xcopy /D /y DXUT11\Core\Profile\dxut.pdb ..\..\Lib\vs2013\Win32Profile\dxut.pdb

echo f | xcopy /D /y DXUT11\Optional\Debug\dxutopt.lib ..\..\Lib\vs2013\Win32Debug\dxutopt.lib
echo f | xcopy /D /y DXUT11\Optional\Debug\dxutopt.pdb ..\..\Lib\vs2013\Win32Debug\dxutopt.pdb
echo f | xcopy /D /y DXUT11\Optional\Release\dxutopt.lib ..\..\Lib\vs2013\Win32Release\dxutopt.lib
echo f | xcopy /D /y DXUT11\Optional\Release\dxutopt.pdb ..\..\Lib\vs2013\Win32Release\dxutopt.pdb
echo f | xcopy /D /y DXUT11\Optional\Profile\dxutopt.lib ..\..\Lib\vs2013\Win32Profile\dxutopt.lib
echo f | xcopy /D /y DXUT11\Optional\Profile\dxutopt.pdb ..\..\Lib\vs2013\Win32Profile\dxutopt.pdb

echo f | xcopy /D /y DXUT11\Effects11\Debug\effects11.lib ..\..\Lib\vs2013\Win32Debug\effects11.lib
echo f | xcopy /D /y DXUT11\Effects11\Debug\effects11.pdb ..\..\Lib\vs2013\Win32Debug\effects11.pdb
echo f | xcopy /D /y DXUT11\Effects11\Release\effects11.lib ..\..\Lib\vs2013\Win32Release\effects11.lib
echo f | xcopy /D /y DXUT11\Effects11\Release\effects11.pdb ..\..\Lib\vs2013\Win32Release\effects11.pdb
echo f | xcopy /D /y DXUT11\Effects11\Profile\effects11.lib ..\..\Lib\vs2013\Win32Profile\effects11.lib
echo f | xcopy /D /y DXUT11\Effects11\Profile\effects11.pdb ..\..\Lib\vs2013\Win32Profile\effects11.pdb

echo f | xcopy /D /y DXUT11\Core\x64\Debug\dxut.lib ..\..\Lib\vs2013\x64Debug\dxut.lib
echo f | xcopy /D /y DXUT11\Core\x64\Debug\dxut.pdb ..\..\Lib\vs2013\x64Debug\dxut.pdb
echo f | xcopy /D /y DXUT11\Core\x64\Release\dxut.lib ..\..\Lib\vs2013\x64Release\dxut.lib
echo f | xcopy /D /y DXUT11\Core\x64\Release\dxut.pdb ..\..\Lib\vs2013\x64Release\dxut.pdb
echo f | xcopy /D /y DXUT11\Core\x64\Profile\dxut.lib ..\..\Lib\vs2013\x64Profile\dxut.lib
echo f | xcopy /D /y DXUT11\Core\x64\Profile\dxut.pdb ..\..\Lib\vs2013\x64Profile\dxut.pdb

echo f | xcopy /D /y DXUT11\Optional\x64\Debug\dxutopt.lib ..\..\Lib\vs2013\x64Debug\dxutopt.lib
echo f | xcopy /D /y DXUT11\Optional\x64\Debug\dxutopt.pdb ..\..\Lib\vs2013\x64Debug\dxutopt.pdb
echo f | xcopy /D /y DXUT11\Optional\x64\Release\dxutopt.lib ..\..\Lib\vs2013\x64Release\dxutopt.lib
echo f | xcopy /D /y DXUT11\Optional\x64\Release\dxutopt.pdb ..\..\Lib\vs2013\x64Release\dxutopt.pdb
echo f | xcopy /D /y DXUT11\Optional\x64\Profile\dxutopt.lib ..\..\Lib\vs2013\x64Profile\dxutopt.lib
echo f | xcopy /D /y DXUT11\Optional\x64\Profile\dxutopt.pdb ..\..\Lib\vs2013\x64Profile\dxutopt.pdb

echo f | xcopy /D /y DXUT11\Effects11\x64\Debug\effects11.lib ..\..\Lib\vs2013\x64Debug\effects11.lib
echo f | xcopy /D /y DXUT11\Effects11\x64\Debug\effects11.pdb ..\..\Lib\vs2013\x64Debug\effects11.pdb
echo f | xcopy /D /y DXUT11\Effects11\x64\Release\effects11.lib ..\..\Lib\vs2013\x64Release\effects11.lib
echo f | xcopy /D /y DXUT11\Effects11\x64\Release\effects11.pdb ..\..\Lib\vs2013\x64Release\effects11.pdb
echo f | xcopy /D /y DXUT11\Effects11\x64\Profile\effects11.lib ..\..\Lib\vs2013\x64Profile\effects11.lib
echo f | xcopy /D /y DXUT11\Effects11\x64\Profile\effects11.pdb ..\..\Lib\vs2013\x64Profile\effects11.pdb

echo f | xcopy /D /y bullet-2.82\lib\BulletCollision_vs2013.lib ..\..\Lib\vs2013\Win32Release\BulletCollision.lib
echo f | xcopy /D /y bullet-2.82\lib\BulletCollision_vs2013.pdb ..\..\Lib\vs2013\Win32Release\BulletCollision.pdb
echo f | xcopy /D /y bullet-2.82\lib\BulletCollision_vs2013.lib ..\..\Lib\vs2013\Win32Profile\BulletCollision.lib
echo f | xcopy /D /y bullet-2.82\lib\BulletCollision_vs2013.pdb ..\..\Lib\vs2013\Win32Profile\BulletCollision.pdb
echo f | xcopy /D /y bullet-2.82\lib\BulletCollision_vs2013_debug.lib ..\..\Lib\vs2013\Win32Debug\BulletCollision.lib
echo f | xcopy /D /y bullet-2.82\lib\BulletCollision_vs2013_debug.pdb ..\..\Lib\vs2013\Win32Debug\BulletCollision.pdb
echo f | xcopy /D /y bullet-2.82\lib\BulletCollision_vs2013_x64_release.lib ..\..\Lib\vs2013\x64Release\BulletCollision.lib
echo f | xcopy /D /y bullet-2.82\lib\BulletCollision_vs2013_x64_release.pdb ..\..\Lib\vs2013\x64Release\BulletCollision.pdb
echo f | xcopy /D /y bullet-2.82\lib\BulletCollision_vs2013_x64_release.lib ..\..\Lib\vs2013\x64Profile\BulletCollision.lib
echo f | xcopy /D /y bullet-2.82\lib\BulletCollision_vs2013_x64_release.pdb ..\..\Lib\vs2013\x64Profile\BulletCollision.pdb
echo f | xcopy /D /y bullet-2.82\lib\BulletCollision_vs2013_x64_debug.lib ..\..\Lib\vs2013\x64Debug\BulletCollision.lib
echo f | xcopy /D /y bullet-2.82\lib\BulletCollision_vs2013_x64_debug.pdb ..\..\Lib\vs2013\x64Debug\BulletCollision.pdb

echo f | xcopy /D /y bullet-2.82\lib\BulletDynamics_vs2013.lib ..\..\Lib\vs2013\Win32Release\BulletDynamics.lib
echo f | xcopy /D /y bullet-2.82\lib\BulletDynamics_vs2013.pdb ..\..\Lib\vs2013\Win32Release\BulletDynamics.pdb
echo f | xcopy /D /y bullet-2.82\lib\BulletDynamics_vs2013.lib ..\..\Lib\vs2013\Win32Profile\BulletDynamics.lib
echo f | xcopy /D /y bullet-2.82\lib\BulletDynamics_vs2013.pdb ..\..\Lib\vs2013\Win32Profile\BulletDynamics.pdb
echo f | xcopy /D /y bullet-2.82\lib\BulletDynamics_vs2013_debug.lib ..\..\Lib\vs2013\Win32Debug\BulletDynamics.lib
echo f | xcopy /D /y bullet-2.82\lib\BulletDynamics_vs2013_debug.pdb ..\..\Lib\vs2013\Win32Debug\BulletDynamics.pdb
echo f | xcopy /D /y bullet-2.82\lib\BulletDynamics_vs2013_x64_release.lib ..\..\Lib\vs2013\x64Release\BulletDynamics.lib
echo f | xcopy /D /y bullet-2.82\lib\BulletDynamics_vs2013_x64_release.pdb ..\..\Lib\vs2013\x64Release\BulletDynamics.pdb
echo f | xcopy /D /y bullet-2.82\lib\BulletDynamics_vs2013_x64_release.lib ..\..\Lib\vs2013\x64Profile\BulletDynamics.lib
echo f | xcopy /D /y bullet-2.82\lib\BulletDynamics_vs2013_x64_release.pdb ..\..\Lib\vs2013\x64Profile\BulletDynamics.pdb
echo f | xcopy /D /y bullet-2.82\lib\BulletDynamics_vs2013_x64_debug.lib ..\..\Lib\vs2013\x64Debug\BulletDynamics.lib
echo f | xcopy /D /y bullet-2.82\lib\BulletDynamics_vs2013_x64_debug.pdb ..\..\Lib\vs2013\x64Debug\BulletDynamics.pdb

echo f | xcopy /D /y bullet-2.82\lib\LinearMath_vs2013.lib ..\..\Lib\vs2013\Win32Release\LinearMath.lib
echo f | xcopy /D /y bullet-2.82\lib\LinearMath_vs2013.pdb ..\..\Lib\vs2013\Win32Release\LinearMath.pdb
echo f | xcopy /D /y bullet-2.82\lib\LinearMath_vs2013.lib ..\..\Lib\vs2013\Win32Profile\LinearMath.lib
echo f | xcopy /D /y bullet-2.82\lib\LinearMath_vs2013.pdb ..\..\Lib\vs2013\Win32Profile\LinearMath.pdb
echo f | xcopy /D /y bullet-2.82\lib\LinearMath_vs2013_debug.lib ..\..\Lib\vs2013\Win32Debug\LinearMath.lib
echo f | xcopy /D /y bullet-2.82\lib\LinearMath_vs2013_debug.pdb ..\..\Lib\vs2013\Win32Debug\LinearMath.pdb
echo f | xcopy /D /y bullet-2.82\lib\LinearMath_vs2013_x64_release.lib ..\..\Lib\vs2013\x64Release\LinearMath.lib
echo f | xcopy /D /y bullet-2.82\lib\LinearMath_vs2013_x64_release.pdb ..\..\Lib\vs2013\x64Release\LinearMath.pdb
echo f | xcopy /D /y bullet-2.82\lib\LinearMath_vs2013_x64_release.lib ..\..\Lib\vs2013\x64Profile\LinearMath.lib
echo f | xcopy /D /y bullet-2.82\lib\LinearMath_vs2013_x64_release.pdb ..\..\Lib\vs2013\x64Profile\LinearMath.pdb
echo f | xcopy /D /y bullet-2.82\lib\LinearMath_vs2013_x64_debug.lib ..\..\Lib\vs2013\x64Debug\LinearMath.lib
echo f | xcopy /D /y bullet-2.82\lib\LinearMath_vs2013_x64_debug.pdb ..\..\Lib\vs2013\x64Debug\LinearMath.pdb

echo f | xcopy /D /y libogg-1.3.0\win32\VS2013\Win32\Debug\libogg_static.lib ..\..\Lib\vs2013\Win32Debug\libogg_static.lib
echo f | xcopy /D /y libogg-1.3.0\win32\VS2013\Win32\Release\libogg_static.lib ..\..\Lib\vs2013\Win32Release\libogg_static.lib
echo f | xcopy /D /y libogg-1.3.0\win32\VS2013\Win32\Release\libogg_static.lib ..\..\Lib\vs2013\Win32Profile\libogg_static.lib
echo f | xcopy /D /y libogg-1.3.0\win32\VS2013\x64\Debug\libogg_static.lib ..\..\Lib\vs2013\x64Debug\libogg_static.lib
echo f | xcopy /D /y libogg-1.3.0\win32\VS2013\x64\Release\libogg_static.lib ..\..\Lib\vs2013\x64Release\libogg_static.lib 
echo f | xcopy /D /y libogg-1.3.0\win32\VS2013\x64\Release\libogg_static.lib ..\..\Lib\vs2013\x64Profile\libogg_static.lib 

echo f | xcopy /D /y libogg-1.3.0\win32\VS2013\Win32\Debug\libogg_static.pdb ..\..\Lib\vs2013\Win32Debug\libogg_static.pdb
echo f | xcopy /D /y libogg-1.3.0\win32\VS2013\Win32\Release\libogg_static.pdb ..\..\Lib\vs2013\Win32Release\libogg_static.pdb
echo f | xcopy /D /y libogg-1.3.0\win32\VS2013\Win32\Release\libogg_static.pdb ..\..\Lib\vs2013\Win32Profile\libogg_static.pdb
echo f | xcopy /D /y libogg-1.3.0\win32\VS2013\x64\Debug\libogg_static.pdb ..\..\Lib\vs2013\x64Debug\libogg_static.pdb
echo f | xcopy /D /y libogg-1.3.0\win32\VS2013\x64\Release\libogg_static.pdb ..\..\Lib\vs2013\x64Release\libogg_static.pdb 
echo f | xcopy /D /y libogg-1.3.0\win32\VS2013\x64\Release\libogg_static.pdb ..\..\Lib\vs2013\x64Profile\libogg_static.pdb


echo f | xcopy /D /y libvorbis-1.3.2\win32\VS2013\Win32\Debug\libvorbis_static.lib ..\..\Lib\vs2013\Win32Debug\libvorbis_static.lib
echo f | xcopy /D /y libvorbis-1.3.2\win32\VS2013\Win32\Release\libvorbis_static.lib ..\..\Lib\vs2013\Win32Release\libvorbis_static.lib
echo f | xcopy /D /y libvorbis-1.3.2\win32\VS2013\Win32\Release\libvorbis_static.lib ..\..\Lib\vs2013\Win32Profile\libvorbis_static.lib
echo f | xcopy /D /y libvorbis-1.3.2\win32\VS2013\x64\Debug\libvorbis_static.lib ..\..\Lib\vs2013\x64Debug\libvorbis_static.lib
echo f | xcopy /D /y libvorbis-1.3.2\win32\VS2013\x64\Release\libvorbis_static.lib ..\..\Lib\vs2013\x64Release\libvorbis_static.lib
echo f | xcopy /D /y libvorbis-1.3.2\win32\VS2013\x64\Release\libvorbis_static.lib ..\..\Lib\vs2013\x64Profile\libvorbis_static.lib

echo f | xcopy /D /y libvorbis-1.3.2\win32\VS2013\Win32\Debug\libvorbis_static.pdb ..\..\Lib\vs2013\Win32Debug\libvorbis_static.pdb
echo f | xcopy /D /y libvorbis-1.3.2\win32\VS2013\Win32\Release\libvorbis_static.pdb ..\..\Lib\vs2013\Win32Release\libvorbis_static.pdb
echo f | xcopy /D /y libvorbis-1.3.2\win32\VS2013\Win32\Release\libvorbis_static.pdb ..\..\Lib\vs2013\Win32Profile\libvorbis_static.pdb
echo f | xcopy /D /y libvorbis-1.3.2\win32\VS2013\x64\Debug\libvorbis_static.pdb ..\..\Lib\vs2013\x64Debug\libvorbis_static.pdb
echo f | xcopy /D /y libvorbis-1.3.2\win32\VS2013\x64\Release\libvorbis_static.pdb ..\..\Lib\vs2013\x64Release\libvorbis_static.pdb
echo f | xcopy /D /y libvorbis-1.3.2\win32\VS2013\x64\Release\libvorbis_static.pdb ..\..\Lib\vs2013\x64Profile\libvorbis_static.pdb

echo f | xcopy /D /y libvorbis-1.3.2\win32\VS2013\Win32\Debug\libvorbisfile_static.lib ..\..\Lib\vs2013\Win32Debug\libvorbisfile_static.lib
echo f | xcopy /D /y libvorbis-1.3.2\win32\VS2013\Win32\Release\libvorbisfile_static.lib ..\..\Lib\vs2013\Win32Release\libvorbisfile_static.lib
echo f | xcopy /D /y libvorbis-1.3.2\win32\VS2013\Win32\Release\libvorbisfile_static.lib ..\..\Lib\vs2013\Win32Profile\libvorbisfile_static.lib
echo f | xcopy /D /y libvorbis-1.3.2\win32\VS2013\x64\Debug\libvorbisfile_static.lib ..\..\Lib\vs2013\x64Debug\libvorbisfile_static.lib
echo f | xcopy /D /y libvorbis-1.3.2\win32\VS2013\x64\Release\libvorbisfile_static.lib ..\..\Lib\vs2013\x64Release\libvorbisfile_static.lib
echo f | xcopy /D /y libvorbis-1.3.2\win32\VS2013\x64\Release\libvorbisfile_static.lib ..\..\Lib\vs2013\x64Profile\libvorbisfile_static.lib

echo f | xcopy /D /y libvorbis-1.3.2\win32\VS2013\Win32\Debug\libvorbisfile_static.pdb ..\..\Lib\vs2013\Win32Debug\libvorbisfile_static.pdb
echo f | xcopy /D /y libvorbis-1.3.2\win32\VS2013\Win32\Release\libvorbisfile_static.pdb ..\..\Lib\vs2013\Win32Release\libvorbisfile_static.pdb
echo f | xcopy /D /y libvorbis-1.3.2\win32\VS2013\Win32\Release\libvorbisfile_static.pdb ..\..\Lib\vs2013\Win32Profile\libvorbisfile_static.pdb
echo f | xcopy /D /y libvorbis-1.3.2\win32\VS2013\x64\Debug\libvorbisfile_static.pdb ..\..\Lib\vs2013\x64Debug\libvorbisfile_static.pdb
echo f | xcopy /D /y libvorbis-1.3.2\win32\VS2013\x64\Release\libvorbisfile_static.pdb ..\..\Lib\vs2013\x64Release\libvorbisfile_static.pdb
echo f | xcopy /D /y libvorbis-1.3.2\win32\VS2013\x64\Release\libvorbisfile_static.pdb ..\..\Lib\vs2013\x64Profile\libvorbisfile_static.pdb

echo f | xcopy /D /y tinyxml_2_6_2\Win32\Debug\tinyxml.lib ..\..\Lib\vs2013\Win32Debug
echo f | xcopy /D /y tinyxml_2_6_2\Win32\Debug\tinyxml.pdb ..\..\Lib\vs2013\Win32Debug
echo f | xcopy /D /y tinyxml_2_6_2\Win32\Release\tinyxml.lib ..\..\Lib\vs2013\Win32Release
echo f | xcopy /D /y tinyxml_2_6_2\Win32\Release\tinyxml.pdb ..\..\Lib\vs2013\Win32Release
echo f | xcopy /D /y tinyxml_2_6_2\Win32\Release\tinyxml.lib ..\..\Lib\vs2013\Win32Profile
echo f | xcopy /D /y tinyxml_2_6_2\Win32\Release\tinyxml.pdb ..\..\Lib\vs2013\Win32Profile

echo f | xcopy /D /y tinyxml_2_6_2\x64\Debug\tinyxml.lib ..\..\Lib\vs2013\x64Debug
echo f | xcopy /D /y tinyxml_2_6_2\x64\Debug\tinyxml.pdb ..\..\Lib\vs2013\x64Debug
echo f | xcopy /D /y tinyxml_2_6_2\x64\Release\tinyxml.lib ..\..\Lib\vs2013\x64Release
echo f | xcopy /D /y tinyxml_2_6_2\x64\Release\tinyxml.pdb ..\..\Lib\vs2013\x64Release
echo f | xcopy /D /y tinyxml_2_6_2\x64\Release\tinyxml.lib ..\..\Lib\vs2013\x64Profile
echo f | xcopy /D /y tinyxml_2_6_2\x64\Release\tinyxml.pdb ..\..\Lib\vs2013\x64Profile

echo f | xcopy /y zlib-1.2.8\contrib\vstudio\vc12\x86\ZlibStatDebug\zlibstat.lib ..\..\Lib\vs2013\Win32Debug
echo f | xcopy /y zlib-1.2.8\contrib\vstudio\vc12\x86\ZlibStatDebug\zlibstat.pdb ..\..\Lib\vs2013\Win32Debug
echo f | xcopy /y zlib-1.2.8\contrib\vstudio\vc12\x86\ZlibStatRelease\zlibstat.lib ..\..\Lib\vs2013\Win32Release
echo f | xcopy /y zlib-1.2.8\contrib\vstudio\vc12\x86\ZlibStatRelease\zlibstat.pdb ..\..\Lib\vs2013\Win32Release
echo f | xcopy /y zlib-1.2.8\contrib\vstudio\vc12\x86\ZlibStatRelease\zlibstat.lib ..\..\Lib\vs2013\Win32Profile
echo f | xcopy /y zlib-1.2.8\contrib\vstudio\vc12\x86\ZlibStatRelease\zlibstat.pdb ..\..\Lib\vs2013\Win32Profile

echo f | xcopy /y zlib-1.2.8\contrib\vstudio\vc12\x64\ZlibStatDebug\zlibstat.lib ..\..\Lib\vs2013\x64Debug
echo f | xcopy /y zlib-1.2.8\contrib\vstudio\vc12\x64\ZlibStatDebug\zlibstat.pdb ..\..\Lib\vs2013\x64Debug
echo f | xcopy /y zlib-1.2.8\contrib\vstudio\vc12\x64\ZlibStatRelease\zlibstat.lib ..\..\Lib\vs2013\x64Release
echo f | xcopy /y zlib-1.2.8\contrib\vstudio\vc12\x64\ZlibStatRelease\zlibstat.pdb ..\..\Lib\vs2013\x64Release
echo f | xcopy /y zlib-1.2.8\contrib\vstudio\vc12\x64\ZlibStatRelease\zlibstat.lib ..\..\Lib\vs2013\x64Profile
echo f | xcopy /y zlib-1.2.8\contrib\vstudio\vc12\x64\ZlibStatRelease\zlibstat.pdb ..\..\Lib\vs2013\x64Profile


echo f | xcopy /y luaplus51-all\bin.vs2013.win32\luaplus51-1201.lib ..\..\Lib\vs2013\Win32Debug\luaplus51-1201.lib
echo f | xcopy /y luaplus51-all\bin.vs2013.win32\luaplus51-1201.lib ..\..\Lib\vs2013\Win32Release\luaplus51-1201.lib
echo f | xcopy /y luaplus51-all\bin.vs2013.win32\luaplus51-1201.lib ..\..\Lib\vs2013\Win32Profile\luaplus51-1201.lib

echo f | xcopy /y luaplus51-all\bin.vs2013.win32\luaplus51-1201.lib ..\..\Lib\vs2013\Win32Debug\luaplus51-1201.pdb
echo f | xcopy /y luaplus51-all\bin.vs2013.win32\luaplus51-1201.lib ..\..\Lib\vs2013\Win32Release\luaplus51-1201.pdb
echo f | xcopy /y luaplus51-all\bin.vs2013.win32\luaplus51-1201.lib ..\..\Lib\vs2013\Win32Profile\luaplus51-1201.pdb

echo f | xcopy /y luaplus51-all\bin.vs2013.win32\luaplus51-1201.dll ..\..\Lib\vs2013\Win32Debug\luaplus51-1201.dll
echo f | xcopy /y luaplus51-all\bin.vs2013.win32\luaplus51-1201.dll ..\..\Lib\vs2013\Win32Release\luaplus51-1201.dll
echo f | xcopy /y luaplus51-all\bin.vs2013.win32\luaplus51-1201.dll ..\..\Lib\vs2013\Win32Profile\luaplus51-1201.dll



