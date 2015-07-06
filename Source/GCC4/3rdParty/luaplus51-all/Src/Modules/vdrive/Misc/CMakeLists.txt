SET(PCH_SRCS
		AnsiString.cpp
		AnsiString.h
		Array.h
		DiskFile.cpp
		DiskFile.h
		File.h
		FixedSizeAllocator.h
		HeapAllocator.h
		List.h
		Map.h
		MemFile.cpp
		MemFile.h
		Misc.cpp
		Misc.h
		Misc_InternalPch.h
		OwnedPtr.h
		TypeTraits.h
		VirtualDrive.cpp
		VirtualDrive.h
		VirtualFile.cpp
		VirtualFile.h
)

SET(AES_SRCS
		aes/aes.h
		aes/aescrypt.c
		aes/aeskey.c
		aes/aesopt.h
		aes/aestab.c
		aes/fileenc.c
		aes/fileenc.h
		aes/hmac.c
		aes/hmac.h
		aes/prng.c
		aes/prng.h
		aes/pwd2key.c
		aes/pwd2key.h
		aes/sha1.c
		aes/sha1.h
		aes/sha2.c
		aes/sha2.h
)

SET(ZLIB_SRCS
		../zlib/adler32.c
		../zlib/compress.c
		../zlib/crc32.c
		../zlib/crc32.h
		../zlib/deflate.c
		../zlib/deflate.h
		../zlib/infback.c
		../zlib/inffast.c
		../zlib/inffast.h
		../zlib/inffixed.h
		../zlib/inflate.c
		../zlib/inflate.h
		../zlib/inftrees.c
		../zlib/inftrees.h
		../zlib/trees.c
		../zlib/trees.h
		../zlib/uncompr.c
		../zlib/zconf.h
		../zlib/zlib.h
		../zlib/zutil.c
		../zlib/zutil.h
)

SET(SRCS
		${PCH_SRCS}
		trio.c
		trio.h
		triodef.h
		trionan.c
		trionan.h
		triop.h
		triostr.c
		triostr.h
		zconf.h
		zlib.h
		${AES_SRCS}
		${ZLIB_SRCS}
)

SOURCE_GROUP(aes FILES ${AES_SRCS})
SOURCE_GROUP(zlib FILES ${ZLIB_SRCS})

IF(MSVC)
	ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE)
ENDIF(MSVC)

INCLUDE_DIRECTORIES(.)
ADD_LIBRARY(Misc STATIC Misc_InternalPch.cpp ${SRCS})
#CREATE_PRECOMPILED_HEADER(Misc Misc_InternalPch)
#USE_PRECOMPILED_HEADER(Misc Misc_InternalPch ${PCH_SRCS})
