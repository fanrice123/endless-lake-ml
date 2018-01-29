# - Try to find CAFFE2
# Once done this will define
# CAFFE2_FOUND - System has CAFFE2
# CAFFE2_INCLUDE_DIR
# CAFFE2_LIBRARY

find_path(CAFFE2_INCLUDE_DIR caffe2/core/workspace.h)

find_library(CAFFE2_LIBRARY NAMES Caffe2_CPU)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Caffe2 DEFAULT_MSG
                                  CAFFE2_LIBRARY CAFFE2_INCLUDE_DIR)
set(CAFFE2_FOUND)

mark_as_advanced(CAFFE2_INCLUDE_DIR CAFFE2_LIBRARY)

