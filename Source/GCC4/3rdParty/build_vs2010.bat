devenv bullet-2.79\msvc\vs2010\0BulletSolution.sln /build Debug /project BulletCollision /projectconfig "Debug|Win32" 
devenv bullet-2.79\msvc\vs2010\0BulletSolution.sln /build Debug /project BulletDynamics /projectconfig "Debug|Win32" 
devenv bullet-2.79\msvc\vs2010\0BulletSolution.sln /build Debug /project LinearMath /projectconfig "Debug|Win32" 

devenv DXUT11\Core\DXUT_2010.sln /build Debug /project DXUT /projectconfig "Debug|Win32"
devenv DXUT11\Optional\DXUTOpt_2010.sln /build Debug /project DXUTOpt /projectconfig "Debug|Win32"
devenv DXUT11\Effects11\Effects11_2010.sln /build Debug /project Effects11 /projectconfig "Debug|Win32"

devenv libogg-1.3.0\win32\VS2010\libogg_static.sln /build Debug /project libogg_static /projectconfig "Debug|Win32"
devenv libvorbis-1.3.2\win32\VS2010\vorbis_static.sln /build Debug /project libvorbisfile /projectconfig "Debug|Win32"

devenv tinyxml_2_6_2\tinyxml.sln /build Debug /project tinyxml /projectconfig "Debug|Win32"

devenv zlib-1.2.5\contrib\vstudio\vc10\zlibvc.sln /build Debug /project zlibvc  /projectconfig "Debug|Win32"

copyalllibs_vs2010




