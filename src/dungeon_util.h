#include "decl.h"

/* macros for accesing the dungeon levels by their old names */
#define oracle_level            (dungeon_topology.d_oracle_level)
#define bigroom_level           (dungeon_topology.d_bigroom_level)
#define medusa_level            (dungeon_topology.d_medusa_level)
#define stronghold_level        (dungeon_topology.d_stronghold_level)
#define valley_level            (dungeon_topology.d_valley_level)
#define wiz1_level              (dungeon_topology.d_wiz1_level)
#define wiz2_level              (dungeon_topology.d_wiz2_level)
#define wiz3_level              (dungeon_topology.d_wiz3_level)
#define juiblex_level           (dungeon_topology.d_juiblex_level)
#define orcus_level             (dungeon_topology.d_orcus_level)
#define baalzebub_level         (dungeon_topology.d_baalzebub_level)
#define asmodeus_level          (dungeon_topology.d_asmodeus_level)
#define portal_level            (dungeon_topology.d_portal_level)
#define sanctum_level           (dungeon_topology.d_sanctum_level)
#define earth_level             (dungeon_topology.d_earth_level)
#define water_level             (dungeon_topology.d_water_level)
#define fire_level              (dungeon_topology.d_fire_level)
#define air_level               (dungeon_topology.d_air_level)
#define astral_level            (dungeon_topology.d_astral_level)
#define tower_dnum              (dungeon_topology.d_tower_dnum)
#define sokoban_dnum            (dungeon_topology.d_sokoban_dnum)
#define mines_dnum              (dungeon_topology.d_mines_dnum)
#define quest_dnum              (dungeon_topology.d_quest_dnum)
#define qstart_level            (dungeon_topology.d_qstart_level)
#define qlocate_level           (dungeon_topology.d_qlocate_level)
#define nemesis_level           (dungeon_topology.d_nemesis_level)
#define knox_level              (dungeon_topology.d_knox_level)

#define xdnstair        (dnstair.sx)
#define ydnstair        (dnstair.sy)
#define xupstair        (upstair.sx)
#define yupstair        (upstair.sy)

#define xdnladder       (dnladder.sx)
#define ydnladder       (dnladder.sy)
#define xupladder       (upladder.sx)
#define yupladder       (upladder.sy)

#define dunlev_reached(x)       (dungeons[(x)->dnum].dunlev_ureached)

/* A particular dungeon contains num_dunlevs d_levels with dlevel 1..
 * num_dunlevs.  Ledger_start and depth_start are bases that are added
 * to the dlevel of a particular d_level to get the effective ledger_no
 * and depth for that d_level.
 *
 * Ledger_no is a bookkeeping number that gives a unique identifier for a
 * particular d_level (for level.?? files, e.g.).
 *
 * Depth corresponds to the number of floors below the surface.
 */
#define Is_astralevel(x)        (on_level(x, &astral_level))
#define Is_earthlevel(x)        (on_level(x, &earth_level))
#define Is_waterlevel(x)        (on_level(x, &water_level))
#define Is_firelevel(x)         (on_level(x, &fire_level))
#define Is_airlevel(x)          (on_level(x, &air_level))
#define Is_medusa_level(x)      (on_level(x, &medusa_level))
#define Is_oracle_level(x)      (on_level(x, &oracle_level))
#define Is_valley(x)            (on_level(x, &valley_level))
#define Is_juiblex_level(x)     (on_level(x, &juiblex_level))
#define Is_asmo_level(x)        (on_level(x, &asmodeus_level))
#define Is_baal_level(x)        (on_level(x, &baalzebub_level))
#define Is_wiz1_level(x)        (on_level(x, &wiz1_level))
#define Is_wiz2_level(x)        (on_level(x, &wiz2_level))
#define Is_wiz3_level(x)        (on_level(x, &wiz3_level))
#define Is_sanctum(x)           (on_level(x, &sanctum_level))
#define Is_portal_level(x)      (on_level(x, &portal_level))
#define Is_rogue_level(x)       (on_level(x, &rogue_level))
#define Is_stronghold(x)        (on_level(x, &stronghold_level))
#define Is_bigroom(x)           (on_level(x, &bigroom_level))
#define Is_qstart(x)            (on_level(x, &qstart_level))
#define Is_qlocate(x)           (on_level(x, &qlocate_level))
#define Is_nemesis(x)           (on_level(x, &nemesis_level))
#define Is_knox(x)              (on_level(x, &knox_level))

#define In_sokoban(x)           ((x)->dnum == sokoban_dnum)
#define Inhell                  In_hell(&u.uz)  /* now gehennom */
#define In_endgame(x)           ((x)->dnum == astral_level.dnum)

#define within_bounded_area(X,Y,LX,LY,HX,HY) \
                ((X) >= (LX) && (X) <= (HX) && (Y) >= (LY) && (Y) <= (HY))

#define IS_SHOP(x)      (rooms[x].rtype >= SHOPBASE)

