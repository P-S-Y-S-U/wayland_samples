# Find Mali
# 
# MALI_LIBRARY
# MALI_FOUND

FIND_LIBRARY(MALI_LIBRARY NAMES mali)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
    Mali
    FOUND_VAR
        Mali_FOUND
    REQUIRED_VARS
        MALI_LIBRARY
)

MARK_AS_ADVANCED(MALI_LIBRARY)