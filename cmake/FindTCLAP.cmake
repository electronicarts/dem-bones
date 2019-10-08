find_path(TCLAP_INCLUDE_DIR 
	NAMES "tclap/CmdLine.h"
	PATHS "ExtLibs/tclap/include"
)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
  TCLAP DEFAULT_MSG TCLAP_INCLUDE_DIR
)
