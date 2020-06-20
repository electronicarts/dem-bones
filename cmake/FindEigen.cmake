find_path(EIGEN_INCLUDE_DIR
	NAMES
		"Eigen/Dense"
		"Eigen/Sparse"
		"Eigen/StdVector"
	PATHS "ExtLibs/Eigen"
)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
  Eigen DEFAULT_MSG EIGEN_INCLUDE_DIR
)
