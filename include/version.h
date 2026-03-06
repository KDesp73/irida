#ifndef ENGINE_VERSION_H
#define ENGINE_VERSION_H

#ifdef __cplusplus
extern "C" {
#endif

#define VERSION_MAJOR 0
#define VERSION_MINOR 4
#define VERSION_PATCH 0

#define STR(x) #x
#define TOSTRING(x) STR(x)

#define VERSION_STRING TOSTRING(VERSION_MAJOR) "." TOSTRING(VERSION_MINOR) "." TOSTRING(VERSION_PATCH)

#define VERSION_HEX ((VERSION_MAJOR * 10000) + (VERSION_MINOR * 100) + VERSION_PATCH)

/** Default engine name; override with -DENGINE_NAME=\"YourName\" when compiling. */
#ifndef ENGINE_NAME
#define ENGINE_NAME "chess-engine"
#endif

/**
 * @brief Fills in the provided pointers with the current version numbers.
 * 
 * @param major Pointer to an int to store the major version
 * @param minor Pointer to an int to store the minor version
 * @param patch Pointer to an int to store the patch version
 */
static inline void version(int* major, int* minor, int* patch) {
    if (major) *major = VERSION_MAJOR;
    if (minor) *minor = VERSION_MINOR;
    if (patch) *patch = VERSION_PATCH;
}

#ifdef __cplusplus
}
#endif

#endif // ENGINE_VERSION_H
