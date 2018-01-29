# - Try to find libatomic
# Once done this will define
# ATOMIC_FOUND - System has ATOMIC
# ATOMIC_LIBRARY

#find_path(ATOMIC_INCLUDE_DIR atomic_ops/sysdeps/gcc/aarch64.h)

find_library(ATOMIC_LIBRARY NAMES atomic)

include(FindPackageHandleStandardArgs)
#find_package_handle_standard_args(atomic DEFAULT_MSG ATOMIC_INCLUDE_DIR ATOMIC_LIBRARY)
find_package_handle_standard_args(atomic DEFAULT_MSG ATOMIC_LIBRARY)

set(ATOMIC_FOUND)
#mark_as_advanced(ATOMIC_LIBRARY ATOMIC_INCLUDE_DIR)
mark_as_advanced(ATOMIC_LIBRARY)
