project(dem-bones)

cmake_minimum_required(VERSION 3.15)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_CONFIGURATION_TYPES "Release;" CACHE STRING "" FORCE)

include_directories("${PROJECT_SOURCE_DIR}/include")
find_package(Eigen REQUIRED)
include_directories("${EIGEN_INCLUDE_DIR}")

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}" CACHE PATH "..." FORCE)
endif()

find_package(OpenMP)
if (OPENMP_FOUND)
	set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
	set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

file(GLOB CMD_SOURCE
	"src/command/*.h"
	"src/command/*.cpp"
)
add_executable(DemBonesCmd "${CMD_SOURCE}")
set_target_properties(DemBonesCmd PROPERTIES OUTPUT_NAME "DemBones")

find_package(TCLAP)
find_package(Alembic)
find_package(FBXSDK)

target_include_directories(DemBonesCmd PUBLIC
	"${TCLAP_INCLUDE_DIR}"
	"${FBXSDK_INCLUDE_DIR}"
	"${ALEMBIC_INCLUDE_DIR}"
	"${ALEMBIC_INCLUDE_DIR}/AlembicPrivate/ilmbase"
)

target_link_libraries(DemBonesCmd PUBLIC 
	"${FBXSDK_LIBS}"
	"${ALEMBIC_LIBS}"
)

if(WIN32)
	set_target_properties(DemBonesCmd PROPERTIES LINK_FLAGS "/force:multiple")
	install(TARGETS DemBonesCmd DESTINATION "bin/Windows")
elseif(APPLE)
	set_target_properties(DemBonesCmd PROPERTIES LINK_FLAGS "-Wl,-F/Library/Frameworks")
	target_link_libraries(DemBonesCmd PUBLIC "-framework CoreFoundation -lxml2 -lz -liconv")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -Wno-switch -Wno-return-stack-address")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wno-switch -Wno-return-stack-address")
	install(TARGETS DemBonesCmd DESTINATION "bin/MacOS")
else()
	target_link_libraries(DemBonesCmd PUBLIC "-lxml2 -lz -ldl")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -D_GLIBCXX_USE_CXX11_ABI=0")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -D_GLIBCXX_USE_CXX11_ABI=0")
	install(TARGETS DemBonesCmd DESTINATION "bin/Linux")
endif()

