find_path(FBXSDK_INCLUDE_DIR 
	NAMES "fbxsdk.h"
	PATHS "ExtLibs/FBXSDK/include"
)

if(WIN32)
	find_path(FBXSDK_LIBS_DIR
		NAMES "libfbxsdk-md.lib"
		PATHS
			"ExtLibs/FBXSDK/lib/vs2017/x64/release"
			"ExtLibs/FBXSDK/lib/vs2015/x64/release"
	)
	file(GLOB FBXSDK_LIBS "${FBXSDK_LIBS_DIR}/*-md.lib")
else()
	find_path(FBXSDK_LIBS_DIR
		NAMES "libfbxsdk.a"
		PATHS 
			"ExtLibs/FBXSDK/lib/clang/release"
			"ExtLibs/FBXSDK/lib/gcc/x64/release"
	)
	file(GLOB FBXSDK_LIBS "${FBXSDK_LIBS_DIR}/*.a")
endif()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
  FBXSDK DEFAULT_MSG FBXSDK_INCLUDE_DIR FBXSDK_LIBS
)
