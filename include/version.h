#ifndef ENGINE_VERSION_H
#define ENGINE_VERSION_H

#ifdef __cplusplus
extern "C" {
#endif

// @module version
// @desc Version numbers and string; version() helper.

// @const VERSION_MAJOR
// @desc Major version number.
#define VERSION_MAJOR 0

// @const VERSION_MINOR
// @desc Minor version number.
#define VERSION_MINOR 6

// @const VERSION_PATCH
// @desc Patch version number.
#define VERSION_PATCH 1

#define STR(x) #x
#define TOSTRING(x) STR(x)

// @const VERSION_STRING
// @desc Version as string "major.minor.patch".
#define VERSION_STRING TOSTRING(VERSION_MAJOR) "." TOSTRING(VERSION_MINOR) "." TOSTRING(VERSION_PATCH)

// @const VERSION_HEX
// @desc Version as single integer (major*10000 + minor*100 + patch).
#define VERSION_HEX ((VERSION_MAJOR * 10000) + (VERSION_MINOR * 100) + VERSION_PATCH)

// @function version
// @desc Fills in the provided pointers with the current version numbers.
// @param major Pointer to store major version (or NULL).
// @param minor Pointer to store minor version (or NULL).
// @param patch Pointer to store patch version (or NULL).
static inline void version(int* major, int* minor, int* patch) {
    if (major) *major = VERSION_MAJOR;
    if (minor) *minor = VERSION_MINOR;
    if (patch) *patch = VERSION_PATCH;
}

#ifdef __cplusplus
}
#endif

#endif // ENGINE_VERSION_H
