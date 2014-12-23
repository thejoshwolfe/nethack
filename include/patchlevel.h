#define VERSION_MAJOR    3
#define VERSION_MINOR    4
#define PATCHLEVEL       3

// Incrementing EDITLEVEL can be used to force invalidation of old bones
// and save files.
#define EDITLEVEL 0

#define COPYRIGHT_BANNER_A \
"NetHack, Copyright 1985-2003"

#define COPYRIGHT_BANNER_B \
"         By Stichting Mathematisch Centrum and M. Stephenson."

#define COPYRIGHT_BANNER_C \
"         See license for details."

/*
 * If two or more successive releases have compatible data files, define
 * this with the version number of the oldest such release so that the
 * new release will accept old save and bones files.  The format is
 *    0xMMmmPPeeL
 * 0x = literal prefix "0x", MM = major version, mm = minor version,
 * PP = patch level, ee = edit level, L = literal suffix "L",
 * with all four numbers specified as two hexadecimal digits.
 */
#define VERSION_COMPATIBILITY 0x03040000L    /* 3.4.0-0 */
