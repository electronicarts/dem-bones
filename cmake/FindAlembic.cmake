find_path(ALEMBIC_INCLUDE_DIR
	NAMES
		"Alembic/AbcCoreAbstract/All.h"
		"Alembic/AbcCoreFactory/All.h"
		"Alembic/AbcGeom/All.h"
		"AlembicPrivate/ilmbase"
	PATHS "ExtLibs/Alembic/include"
)

if(WIN32)
	find_path(ALEMBIC_LIB_DIR
		NAMES "Alembic.lib"
		PATHS "ExtLibs/Alembic/lib"
	)
	file(GLOB ALEMBIC_LIBS "${ALEMBIC_LIB_DIR}/*.lib")
else()
	find_path(ALEMBIC_LIB_DIR
		NAMES "libAlembic.a"
		PATHS "ExtLibs/Alembic/lib"
	)
	file(GLOB ALEMBIC_LIBS "${ALEMBIC_LIB_DIR}/*.a")
endif()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
  Alembic DEFAULT_MSG ALEMBIC_INCLUDE_DIR ALEMBIC_LIBS
)
