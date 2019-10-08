find_path(FBXSDK_INCLUDE_DIR 
	NAMES "fbxsdk.h"
	PATHS "ExtLibs/FBXSDK/include"
)

find_library(FBXSDK_LIBS
	NAMES "libfbxsdk-md.lib" "libfbxsdk.a"
	PATHS
		"ExtLibs/FBXSDK/lib/vs2015/x64/release"
		"ExtLibs/FBXSDK/lib/clang/release"
)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
  FBXSDK DEFAULT_MSG FBXSDK_INCLUDE_DIR FBXSDK_LIBS
)
