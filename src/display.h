/* See LICENSE in the root of this project for change info */

#ifndef DISPLAY_H
#define DISPLAY_H

#include "trap.h"
#include "obj.h"
#include "pm.h"
#include "rm.h"
#include "onames.h"

void magic_map_background(signed char,signed char,int);
void map_background(signed char,signed char,int);
void map_trap(struct trap *,int);
void map_object(struct obj *,int);
void map_invisible(signed char,signed char);
void unmap_object(int,int);
void map_location(int,int,int);
void feel_location(signed char,signed char);
void newsym(int,int);
void shieldeff(signed char,signed char);
void tmp_at(int,int);
void swallowed(int);
void under_ground(int);
void under_water(int);
void see_monsters(void);
void set_mimic_blocking(void);
void see_objects(void);
void see_traps(void);
void curs_on_u(void);
int doredraw(void);
void docrt(void);
void show_glyph(int,int,int);
void clear_glyph_buffer(void);
void row_refresh(int,int,int);
void cls(void);
void flush_screen(int);
int back_to_glyph(signed char,signed char);
int zapdir_to_glyph(int,int,int);
int glyph_at(signed char,signed char);
void set_wall_state(void);

/*
 * tmp_at() control calls.
 */
#define DISP_BEAM    (-1)  /* Keep all glyphs showing & clean up at end. */
#define DISP_FLASH   (-2)  /* Clean up each glyph before displaying new one. */
#define DISP_ALWAYS  (-3)  /* Like flash, but still displayed if not visible. */
#define DISP_CHANGE  (-4)  /* Change glyph. */
#define DISP_END     (-5)  /* Clean up. */
#define DISP_FREEMEM (-6)  /* Free all memory during exit only. */


/* Total number of cmap indices in the sheild_static[] array. */
#define SHIELD_COUNT 21


/*
 * A glyph is an abstraction that represents a _unique_ monster, object,
 * dungeon part, or effect.  The uniqueness is important.  For example,
 * It is not enough to have four (one for each "direction") zap beam glyphs,
 * we need a set of four for each beam type.  Why go to so much trouble?
 * Because it is possible that any given window dependent display driver
 * [print_glyph()] can produce something different for each type of glyph.
 * That is, a beam of cold and a beam of fire would not only be different
 * colors, but would also be represented by different symbols.
 *
 * Glyphs are grouped for easy accessibility:
 *
 * monster      Represents all the wild (not tame) monsters.  Count: NUMMONS.
 *
 * pet          Represents all of the tame monsters.  Count: NUMMONS
 *
 * invisible    Invisible monster placeholder.  Count: 1
 *
 * detect       Represents all detected monsters.  Count: NUMMONS
 *
 * corpse       One for each monster.  Count: NUMMONS
 *
 * ridden       Represents all monsters being ridden.  Count: NUMMONS
 *
 * object       One for each object.  Count: NUM_OBJECTS
 *
 * cmap         One for each entry in the character map.  The character map
 *              is the dungeon features and other miscellaneous things.
 *              Count: MAXPCHARS
 *
 * explosions   A set of nine for each of the following seven explosion types:
 *                   dark, noxious, muddy, wet, magical, fiery, frosty.
 *              The nine positions represent those surrounding the hero.
 *              Count: MAXEXPCHARS * EXPL_MAX (EXPL_MAX is defined in hack.h)
 *
 * zap beam     A set of four (there are four directions) for each beam type.
 *              The beam type is shifted over 2 positions and the direction
 *              is stored in the lower 2 bits.  Count: NUM_ZAP << 2
 *
 * swallow      A set of eight for each monster.  The eight positions rep-
 *              resent those surrounding the hero.  The monster number is
 *              shifted over 3 positions and the swallow position is stored
 *              in the lower three bits.  Count: NUMMONS << 3
 *
 * warning      A set of six representing the different warning levels.
 *
 * The following are offsets used to convert to and from a glyph.
 */
#define NUM_ZAP 8       /* number of zap beam types */

#define GLYPH_MON_OFF           0
#define GLYPH_PET_OFF           (NUMMONS        + GLYPH_MON_OFF)
#define GLYPH_INVIS_OFF         (NUMMONS        + GLYPH_PET_OFF)
#define GLYPH_DETECT_OFF        (1              + GLYPH_INVIS_OFF)
#define GLYPH_BODY_OFF          (NUMMONS        + GLYPH_DETECT_OFF)
#define GLYPH_RIDDEN_OFF        (NUMMONS        + GLYPH_BODY_OFF)
#define GLYPH_OBJ_OFF           (NUMMONS        + GLYPH_RIDDEN_OFF)
#define GLYPH_CMAP_OFF          (NUM_OBJECTS    + GLYPH_OBJ_OFF)
#define GLYPH_EXPLODE_OFF       ((MAXPCHARS - MAXEXPCHARS) + GLYPH_CMAP_OFF)
#define GLYPH_ZAP_OFF           ((MAXEXPCHARS * EXPL_MAX) + GLYPH_EXPLODE_OFF)
#define GLYPH_SWALLOW_OFF       ((NUM_ZAP << 2) + GLYPH_ZAP_OFF)
#define GLYPH_WARNING_OFF       ((NUMMONS << 3) + GLYPH_SWALLOW_OFF)
#define MAX_GLYPH               (WARNCOUNT      + GLYPH_WARNING_OFF)

#define NO_GLYPH MAX_GLYPH

#define GLYPH_INVISIBLE GLYPH_INVIS_OFF

#endif /* DISPLAY_H */
