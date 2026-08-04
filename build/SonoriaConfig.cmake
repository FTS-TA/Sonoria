
####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was SonoriaConfig.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################

include(CMakeFindDependencyMacro)

# Find required dependencies
find_dependency(Eigen3 3.3.4)

# Optional dependencies - only warn if not found
find_package(FFTW QUIET)
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(AVCODEC libavcodec QUIET)
    pkg_check_modules(AVFORMAT libavformat QUIET)
    pkg_check_modules(AVUTIL libavutil QUIET)
    pkg_check_modules(SWRESAMPLE libswresample QUIET)
endif()

if(NOT TARGET Sonoria::sonoria)
    include("${CMAKE_CURRENT_LIST_DIR}/SonoriaTargets.cmake")
endif()

check_required_components(Sonoria)
