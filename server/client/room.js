exports.roomFeatures = [
  ' ', // S_stone         = 0,
  '-', // S_vwall         = 1,
  '|', // S_hwall         = 2,
  '-', // S_tlcorn        = 3,
  '-', // S_trcorn        = 4,
  '-', // S_blcorn        = 5,
  '-', // S_brcorn        = 6,
  '?', // S_crwall        = 7,
  '?', // S_tuwall        = 8,
  '?', // S_tdwall        = 9,
  '?', // S_tlwall        = 10,
  '?', // S_trwall        = 11,
  '+', // S_ndoor         = 12,
  '-', // S_vodoor        = 13,
  '-', // S_hodoor        = 14,
  '+', // S_vcdoor        = 15,      /* closed door, vertical wall */
  '+', // S_hcdoor        = 16,      /* closed door, horizontal wall */
  '=', // S_bars          = 17,      /* KMH -- iron bars */
  '#', // S_tree          = 18,      /* KMH */
  '.', // S_room          = 19,
  '#', // S_corr          = 20,
  '#', // S_litcorr       = 21,
  '<', // S_upstair       = 22,
  '>', // S_dnstair       = 23,
  '<', // S_upladder      = 24,
  '>', // S_dnladder      = 25,
  '_', // S_altar         = 26,
  '|', // S_grave         = 27,
  '?', // S_throne        = 28,
  '#', // S_sink          = 29,
  '}', // S_fountain      = 30,
  '?', // S_pool          = 31,
  '?', // S_ice           = 32,
  '?', // S_lava          = 33,
  '?', // S_vodbridge     = 34,
  '?', // S_hodbridge     = 35,
  '?', // S_vcdbridge     = 36,      /* closed drawbridge, vertical wall */
  '?', // S_hcdbridge     = 37,      /* closed drawbridge, horizontal wall */
  '?', // S_air           = 38,
  '?', // S_cloud         = 39,
  '?', // S_water         = 40,

  // end dungeon characters, begin traps */

  '^', // S_arrow_trap            = 41,
  '^', // S_dart_trap             = 42,
  '^', // S_falling_rock_trap     = 43,
  '^', // S_squeaky_board         = 44,
  '^', // S_bear_trap             = 45,
  '^', // S_land_mine             = 46,
  '^', // S_rolling_boulder_trap  = 47,
  '^', // S_sleeping_gas_trap     = 48,
  '^', // S_rust_trap             = 49,
  '^', // S_fire_trap             = 50,
  '^', // S_pit                   = 51,
  '^', // S_spiked_pit            = 52,
  '^', // S_hole                  = 53,
  '^', // S_trap_door             = 54,
  '^', // S_teleportation_trap    = 55,
  '^', // S_level_teleporter      = 56,
  '^', // S_magic_portal          = 57,
  '^', // S_web                   = 58,
  '^', // S_statue_trap           = 59,
  '?', // S_magic_trap            = 60,
  '?', // S_anti_magic_trap       = 61,
  '?', // S_polymorph_trap        = 62,

  // end traps, begin special effects */

  '|', // S_vbeam        = 63,      /* The 4 zap beam symbols.  Do NOT separate. */
  '-', // S_hbeam        = 64,      /* To change order or add, see function     */
  '/', // S_lslant       = 65,      /* zapdir_to_glyph() in display.c.          */
  '\\', // S_rslant       = 66,
  '*', // S_digbeam      = 67,      /* dig beam symbol */
  '#', // S_flashbeam    = 68,      /* camera flash symbol */
  ')', // S_boomleft     = 69,      /* thrown boomerang, open left, e.g ')'    */
  '(', // S_boomright    = 70,      /* thrown boomerand, open right, e.g. '('  */
  '?', // S_ss1          = 71,      /* 4 magic shield glyphs */
  '?', // S_ss2          = 72,
  '?', // S_ss3          = 73,
  '?', // S_ss4          = 74,

  // The 8 swallow symbols.  Do NOT separate.  To change order or add, see */
  // the function swallow_to_glyph() in display.c.                         */
  '/', // S_sw_tl        = 75,      /* swallow top left [1]                 */
  '-', // S_sw_tc        = 76,      /* swallow top center [2]       Order:  */
  '\\', // S_sw_tr        = 77,      /* swallow top right [3]                */
  '|', // S_sw_ml        = 78,      /* swallow middle left [4]      1 2 3   */
  '|', // S_sw_mr        = 79,      /* swallow middle right [6]     4 5 6   */
  '\\', // S_sw_bl        = 80,      /* swallow bottom left [7]      7 8 9   */
  '-', // S_sw_bc        = 81,      /* swallow bottom center [8]            */
  '/', // S_sw_br        = 82,      /* swallow bottom right [9]             */

  '?', // S_explode1     = 83,      /* explosion top left                   */
  '?', // S_explode2     = 84,      /* explosion top center                 */
  '?', // S_explode3     = 85,      /* explosion top right           Ex.    */
  '?', // S_explode4     = 86,      /* explosion middle left                */
  '?', // S_explode5     = 87,      /* explosion middle center       /-\    */
  '?', // S_explode6     = 88,      /* explosion middle right        |@|    */
  '?', // S_explode7     = 89,      /* explosion bottom left         \-/    */
  '?', // S_explode8     = 90,      /* explosion bottom center              */
  '?', // S_explode9     = 91,      /* explosion bottom right               */
];
