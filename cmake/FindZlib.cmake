find_library(ZLIB_LIBS
	NAMES "zlib"
	PATHS "ExtLibs/zlib"
)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
  ZLIB DEFAULT_MSG ZLIB_LIBS
)
