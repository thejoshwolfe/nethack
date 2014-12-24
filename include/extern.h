#ifndef EXTERN_H
#define EXTERN_H

/* ### alloc.c ### */

extern char *FDECL(fmt_ptr, (const void *,char *));

/* This next pre-processor directive covers almost the entire file,
 * interrupted only occasionally to pick up specific functions as needed. */
#if !defined(MAKEDEFS_C) && !defined(LEV_LEX_C)

/* ### allmain.c ### */

extern void NDECL(moveloop);
extern void NDECL(stop_occupation);
extern void NDECL(display_gamewindows);
extern void NDECL(newgame);
extern void FDECL(welcome, (boolean));

/* ### apply.c ### */

extern int NDECL(doapply);
extern int NDECL(dorub);
extern int NDECL(dojump);
extern int FDECL(jump, (int));
extern int NDECL(number_leashed);
extern void FDECL(o_unleash, (struct obj *));
extern void FDECL(m_unleash, (struct monst *,boolean));
extern void NDECL(unleash_all);
extern boolean NDECL(next_to_u);
extern struct obj *FDECL(get_mleash, (struct monst *));
extern void FDECL(check_leash, (signed char,signed char));
extern boolean FDECL(um_dist, (signed char,signed char,signed char));
extern boolean FDECL(snuff_candle, (struct obj *));
extern boolean FDECL(snuff_lit, (struct obj *));
extern boolean FDECL(catch_lit, (struct obj *));
extern void FDECL(use_unicorn_horn, (struct obj *));
extern boolean FDECL(tinnable, (struct obj *));
extern void NDECL(reset_trapset);
extern void FDECL(fig_transform, (void *, long));
extern int FDECL(unfixable_trouble_count,(boolean));

/* ### artifact.c ### */

extern void NDECL(init_artifacts);
extern void FDECL(save_artifacts, (int));
extern void FDECL(restore_artifacts, (int));
extern const char *FDECL(artiname, (int));
extern struct obj *FDECL(mk_artifact, (struct obj *,aligntyp));
extern const char *FDECL(artifact_name, (const char *,short *));
extern boolean FDECL(exist_artifact, (int,const char *));
extern void FDECL(artifact_exists, (struct obj *,const char *,boolean));
extern int NDECL(nartifact_exist);
extern boolean FDECL(spec_ability, (struct obj *,unsigned long));
extern boolean FDECL(confers_luck, (struct obj *));
extern boolean FDECL(arti_reflects, (struct obj *));
extern boolean FDECL(restrict_name, (struct obj *,const char *));
extern boolean FDECL(defends, (int,struct obj *));
extern boolean FDECL(protects, (int,struct obj *));
extern void FDECL(set_artifact_intrinsic, (struct obj *,boolean,long));
extern int FDECL(touch_artifact, (struct obj *,struct monst *));
extern int FDECL(spec_abon, (struct obj *,struct monst *));
extern int FDECL(spec_dbon, (struct obj *,struct monst *,int));
extern void FDECL(discover_artifact, (signed char));
extern boolean FDECL(undiscovered_artifact, (signed char));
extern int FDECL(disp_artifact_discoveries, (winid));
extern boolean FDECL(artifact_hit, (struct monst *,struct monst *,
				struct obj *,int *,int));
extern int NDECL(doinvoke);
extern void FDECL(arti_speak, (struct obj *));
extern boolean FDECL(artifact_light, (struct obj *));
extern long FDECL(spec_m2, (struct obj *));
extern boolean FDECL(artifact_has_invprop, (struct obj *,unsigned char));
extern long FDECL(arti_cost, (struct obj *));

/* ### attrib.c ### */

extern boolean FDECL(adjattrib, (int,int,int));
extern void FDECL(change_luck, (signed char));
extern int FDECL(stone_luck, (boolean));
extern void NDECL(set_moreluck);
extern void FDECL(gainstr, (struct obj *,int));
extern void FDECL(losestr, (int));
extern void NDECL(restore_attrib);
extern void FDECL(exercise, (int,boolean));
extern void NDECL(exerchk);
extern void NDECL(reset_attribute_clock);
extern void FDECL(init_attr, (int));
extern void NDECL(redist_attr);
extern void FDECL(adjabil, (int,int));
extern int NDECL(newhp);
extern signed char FDECL(acurr, (int));
extern signed char NDECL(acurrstr);
extern void FDECL(adjalign, (int));

/* ### ball.c ### */

extern void NDECL(ballfall);
extern void NDECL(placebc);
extern void NDECL(unplacebc);
extern void FDECL(set_bc, (int));
extern void FDECL(move_bc, (int,int,signed char,signed char,signed char,signed char));
extern boolean FDECL(drag_ball, (signed char,signed char,
		int *,signed char *,signed char *,signed char *,signed char *, boolean *,boolean));
extern void FDECL(drop_ball, (signed char,signed char));
extern void NDECL(drag_down);

/* ### bones.c ### */

extern boolean NDECL(can_make_bones);
extern void FDECL(savebones, (struct obj *));
extern int NDECL(getbones);

/* ### botl.c ### */

extern int FDECL(xlev_to_rank, (int));
extern int FDECL(title_to_mon, (const char *,int *,int *));
extern void NDECL(max_rank_sz);
#ifdef SCORE_ON_BOTL
extern long NDECL(botl_score);
#endif
extern int FDECL(describe_level, (char *));
extern const char *FDECL(rank_of, (int,short,boolean));
extern void NDECL(bot);
#ifdef DUMP_LOG
extern void FDECL(bot1str, (char *));
extern void FDECL(bot2str, (char *));
#endif

/* ### cmd.c ### */

#ifdef USE_TRAMPOLI
extern int NDECL(doextcmd);
extern int NDECL(domonability);
extern int NDECL(doprev_message);
extern int NDECL(timed_occupation);
extern int NDECL(wiz_attributes);
extern int NDECL(enter_explore_mode);
# ifdef WIZARD
extern int NDECL(wiz_detect);
extern int NDECL(wiz_genesis);
extern int NDECL(wiz_identify);
extern int NDECL(wiz_level_tele);
extern int NDECL(wiz_map);
extern int NDECL(wiz_where);
extern int NDECL(wiz_wish);
# endif /* WIZARD */
#endif /* USE_TRAMPOLI */
extern void NDECL(reset_occupations);
extern void FDECL(set_occupation, (int (*)(void),const char *,int));
#ifdef REDO
extern char NDECL(pgetchar);
extern void FDECL(pushch, (char));
extern void FDECL(savech, (char));
#endif
#ifdef WIZARD
extern void NDECL(add_debug_extended_commands);
#endif /* WIZARD */
extern void FDECL(rhack, (char *));
extern int NDECL(doextlist);
extern int NDECL(extcmd_via_menu);
extern void FDECL(enlightenment, (int));
extern void FDECL(show_conduct, (int));
#ifdef DUMP_LOG
extern void FDECL(dump_enlightenment, (int));
extern void FDECL(dump_conduct, (int));
#endif
extern int FDECL(xytod, (signed char,signed char));
extern void FDECL(dtoxy, (coord *,int));
extern int FDECL(movecmd, (char));
extern int FDECL(getdir, (const char *));
extern void NDECL(confdir);
extern int FDECL(isok, (int,int));
extern int FDECL(get_adjacent_loc, (const char *, const char *, signed char, signed char, coord *));
extern const char *FDECL(click_to_cmd, (int,int,int));
extern char NDECL(readchar);
#ifdef WIZARD
extern void NDECL(sanity_check);
#endif
extern char FDECL(yn_function, (const char *, const char *, char));

/* ### dbridge.c ### */

extern boolean FDECL(is_pool, (int,int));
extern boolean FDECL(is_lava, (int,int));
extern boolean FDECL(is_ice, (int,int));
extern int FDECL(is_drawbridge_wall, (int,int));
extern boolean FDECL(is_db_wall, (int,int));
extern boolean FDECL(find_drawbridge, (int *,int*));
extern boolean FDECL(create_drawbridge, (int,int,int,boolean));
extern void FDECL(open_drawbridge, (int,int));
extern void FDECL(close_drawbridge, (int,int));
extern void FDECL(destroy_drawbridge, (int,int));

/* ### decl.c ### */

extern void NDECL(decl_init);

/* ### detect.c ### */

extern struct obj *FDECL(o_in, (struct obj*,char));
extern struct obj *FDECL(o_material, (struct obj*,unsigned));
extern int FDECL(gold_detect, (struct obj *));
extern int FDECL(food_detect, (struct obj *));
extern int FDECL(object_detect, (struct obj *,int));
extern int FDECL(monster_detect, (struct obj *,int));
extern int FDECL(trap_detect, (struct obj *));
extern const char *FDECL(level_distance, (d_level *));
extern void FDECL(use_crystal_ball, (struct obj *));
extern void NDECL(do_mapping);
extern void NDECL(do_vicinity_map);
extern void FDECL(cvt_sdoor_to_door, (struct rm *));
#ifdef USE_TRAMPOLI
extern void FDECL(findone, (int,int,void *));
extern void FDECL(openone, (int,int,void *));
#endif
extern int NDECL(findit);
extern int NDECL(openit);
extern void FDECL(find_trap, (struct trap *));
extern int FDECL(dosearch0, (int));
extern int NDECL(dosearch);
extern void NDECL(sokoban_detect);

/* ### dig.c ### */

extern boolean NDECL(is_digging);
#ifdef USE_TRAMPOLI
extern int NDECL(dig);
#endif
extern int NDECL(holetime);
extern boolean FDECL(dig_check, (struct monst *, boolean, int, int));
extern void FDECL(digactualhole, (int,int,struct monst *,int));
extern boolean FDECL(dighole, (boolean));
extern int FDECL(use_pick_axe, (struct obj *));
extern int FDECL(use_pick_axe2, (struct obj *));
extern boolean FDECL(mdig_tunnel, (struct monst *));
extern void FDECL(watch_dig, (struct monst *,signed char,signed char,boolean));
extern void NDECL(zap_dig);
extern struct obj *FDECL(bury_an_obj, (struct obj *));
extern void FDECL(bury_objs, (int,int));
extern void FDECL(unearth_objs, (int,int));
extern void FDECL(rot_organic, (void *, long));
extern void FDECL(rot_corpse, (void *, long));

/* ### display.c ### */

#ifdef INVISIBLE_OBJECTS
extern struct obj * FDECL(vobj_at, (signed char,signed char));
#endif /* INVISIBLE_OBJECTS */
extern void FDECL(magic_map_background, (signed char,signed char,int));
extern void FDECL(map_background, (signed char,signed char,int));
extern void FDECL(map_trap, (struct trap *,int));
extern void FDECL(map_object, (struct obj *,int));
extern void FDECL(map_invisible, (signed char,signed char));
extern void FDECL(unmap_object, (int,int));
extern void FDECL(map_location, (int,int,int));
extern void FDECL(feel_location, (signed char,signed char));
extern void FDECL(newsym, (int,int));
extern void FDECL(shieldeff, (signed char,signed char));
extern void FDECL(tmp_at, (int,int));
extern void FDECL(swallowed, (int));
extern void FDECL(under_ground, (int));
extern void FDECL(under_water, (int));
extern void NDECL(see_monsters);
extern void NDECL(set_mimic_blocking);
extern void NDECL(see_objects);
extern void NDECL(see_traps);
extern void NDECL(curs_on_u);
extern int NDECL(doredraw);
extern void NDECL(docrt);
extern void FDECL(show_glyph, (int,int,int));
extern void NDECL(clear_glyph_buffer);
extern void FDECL(row_refresh, (int,int,int));
extern void NDECL(cls);
extern void FDECL(flush_screen, (int));
#ifdef DUMP_LOG
extern void NDECL(dump_screen);
#endif
extern int FDECL(back_to_glyph, (signed char,signed char));
extern int FDECL(zapdir_to_glyph, (int,int,int));
extern int FDECL(glyph_at, (signed char,signed char));
extern void NDECL(set_wall_state);

/* ### do.c ### */

#ifdef USE_TRAMPOLI
extern int FDECL(drop, (struct obj *));
extern int NDECL(wipeoff);
#endif
extern int NDECL(dodrop);
extern boolean FDECL(boulder_hits_pool, (struct obj *,int,int,boolean));
extern boolean FDECL(flooreffects, (struct obj *,int,int,const char *));
extern void FDECL(doaltarobj, (struct obj *));
extern boolean FDECL(canletgo, (struct obj *,const char *));
extern void FDECL(dropx, (struct obj *));
extern void FDECL(dropy, (struct obj *));
extern void FDECL(obj_no_longer_held, (struct obj *));
extern int NDECL(doddrop);
extern int NDECL(dodown);
extern int NDECL(doup);
#ifdef INSURANCE
extern void NDECL(save_currentstate);
#endif
extern void FDECL(goto_level, (d_level *,boolean,boolean,boolean));
extern void FDECL(schedule_goto, (d_level *,boolean,boolean,int,
			     const char *,const char *));
extern void NDECL(deferred_goto);
extern boolean FDECL(revive_corpse, (struct obj *));
extern void FDECL(revive_mon, (void *, long));
extern int NDECL(donull);
extern int NDECL(dowipe);
extern void FDECL(set_wounded_legs, (long,int));
extern void NDECL(heal_legs);

/* ### do_name.c ### */

extern int FDECL(getpos, (coord *,boolean,const char *));
extern struct monst *FDECL(christen_monst, (struct monst *,const char *));
extern int NDECL(do_mname);
extern struct obj *FDECL(oname, (struct obj *,const char *));
extern int NDECL(ddocall);
extern void FDECL(docall, (struct obj *));
extern const char *NDECL(rndghostname);
extern char *FDECL(x_monnam, (struct monst *,int,const char *,int,boolean));
extern char *FDECL(l_monnam, (struct monst *));
extern char *FDECL(mon_nam, (struct monst *));
extern char *FDECL(noit_mon_nam, (struct monst *));
extern char *FDECL(Monnam, (struct monst *));
extern char *FDECL(noit_Monnam, (struct monst *));
extern char *FDECL(m_monnam, (struct monst *));
extern char *FDECL(y_monnam, (struct monst *));
extern char *FDECL(Adjmonnam, (struct monst *,const char *));
extern char *FDECL(Amonnam, (struct monst *));
extern char *FDECL(a_monnam, (struct monst *));
extern char *FDECL(distant_monnam, (struct monst *,int,char *));
extern const char *NDECL(rndmonnam);
extern const char *FDECL(hcolor, (const char *));
extern const char *NDECL(rndcolor);
#ifdef REINCARNATION
extern const char *NDECL(roguename);
#endif
extern struct obj *FDECL(realloc_obj,
		(struct obj *, int, void *, int, const char *));
extern char *FDECL(coyotename, (struct monst *,char *));

/* ### do_wear.c ### */

#ifdef USE_TRAMPOLI
extern int NDECL(Armor_on);
extern int NDECL(Boots_on);
extern int NDECL(Gloves_on);
extern int NDECL(Helmet_on);
extern int FDECL(select_off, (struct obj *));
extern int NDECL(take_off);
#endif
extern void FDECL(off_msg, (struct obj *));
extern void NDECL(set_wear);
extern boolean FDECL(donning, (struct obj *));
extern void NDECL(cancel_don);
extern int NDECL(Armor_off);
extern int NDECL(Armor_gone);
extern int NDECL(Helmet_off);
extern int NDECL(Gloves_off);
extern int NDECL(Boots_off);
extern int NDECL(Cloak_off);
extern int NDECL(Shield_off);
#ifdef TOURIST
extern int NDECL(Shirt_off);
#endif
extern void NDECL(Amulet_off);
extern void FDECL(Ring_on, (struct obj *));
extern void FDECL(Ring_off, (struct obj *));
extern void FDECL(Ring_gone, (struct obj *));
extern void FDECL(Blindf_on, (struct obj *));
extern void FDECL(Blindf_off, (struct obj *));
extern int NDECL(dotakeoff);
extern int NDECL(doremring);
extern int FDECL(cursed, (struct obj *));
extern int FDECL(armoroff, (struct obj *));
extern int FDECL(canwearobj, (struct obj *, long *, boolean));
extern int NDECL(dowear);
extern int NDECL(doputon);
extern void NDECL(find_ac);
extern void NDECL(glibr);
extern struct obj *FDECL(some_armor,(struct monst *));
extern void FDECL(erode_armor, (struct monst *,boolean));
extern struct obj *FDECL(stuck_ring, (struct obj *,int));
extern struct obj *NDECL(unchanger);
extern void NDECL(reset_remarm);
extern int NDECL(doddoremarm);
extern int FDECL(destroy_arm, (struct obj *));
extern void FDECL(adj_abon, (struct obj *,signed char));

/* ### dog.c ### */

extern void FDECL(initedog, (struct monst *));
extern struct monst *FDECL(make_familiar, (struct obj *,signed char,signed char,boolean));
extern struct monst *NDECL(makedog);
extern void NDECL(update_mlstmv);
extern void NDECL(losedogs);
extern void FDECL(mon_arrive, (struct monst *,boolean));
extern void FDECL(mon_catchup_elapsed_time, (struct monst *,long));
extern void FDECL(keepdogs, (boolean));
extern void FDECL(migrate_to_level, (struct monst *,signed char,signed char,coord *));
extern int FDECL(dogfood, (struct monst *,struct obj *));
extern struct monst *FDECL(tamedog, (struct monst *,struct obj *));
extern void FDECL(abuse_dog, (struct monst *));
extern void FDECL(wary_dog, (struct monst *, boolean));

/* ### dogmove.c ### */

extern int FDECL(dog_nutrition, (struct monst *,struct obj *));
extern int FDECL(dog_eat, (struct monst *,struct obj *,int,int,boolean));
extern int FDECL(dog_move, (struct monst *,int));
#ifdef USE_TRAMPOLI
extern void FDECL(wantdoor, (int,int,void *));
#endif

/* ### dokick.c ### */

extern boolean FDECL(ghitm, (struct monst *,struct obj *));
extern void FDECL(container_impact_dmg, (struct obj *));
extern int NDECL(dokick);
extern boolean FDECL(ship_object, (struct obj *,signed char,signed char,boolean));
extern void NDECL(obj_delivery);
extern signed char FDECL(down_gate, (signed char,signed char));
extern void FDECL(impact_drop, (struct obj *,signed char,signed char,signed char));

/* ### dothrow.c ### */

extern int NDECL(dothrow);
extern int NDECL(dofire);
extern void FDECL(hitfloor, (struct obj *));
extern void FDECL(hurtle, (int,int,int,boolean));
extern void FDECL(mhurtle, (struct monst *,int,int,int));
extern void FDECL(throwit, (struct obj *,long,boolean));
extern int FDECL(omon_adj, (struct monst *,struct obj *,boolean));
extern int FDECL(thitmonst, (struct monst *,struct obj *));
extern int FDECL(hero_breaks, (struct obj *,signed char,signed char,boolean));
extern int FDECL(breaks, (struct obj *,signed char,signed char));
extern boolean FDECL(breaktest, (struct obj *));
extern boolean FDECL(walk_path, (coord *, coord *, boolean (*)(void *,int,int), void *));
extern boolean FDECL(hurtle_step, (void *, int, int));

/* ### drawing.c ### */
#endif /* !MAKEDEFS_C && !LEV_LEX_C */
extern int FDECL(def_char_to_objclass, (char));
extern int FDECL(def_char_to_monclass, (char));
#if !defined(MAKEDEFS_C) && !defined(LEV_LEX_C)
extern void FDECL(assign_graphics, (unsigned char *,int,int,int));
extern void FDECL(switch_graphics, (int));
#ifdef REINCARNATION
extern void FDECL(assign_rogue_graphics, (boolean));
#endif

/* ### dungeon.c ### */

extern void FDECL(save_dungeon, (int,boolean,boolean));
extern void FDECL(restore_dungeon, (int));
extern void FDECL(insert_branch, (branch *,boolean));
extern void NDECL(init_dungeons);
extern s_level *FDECL(find_level, (const char *));
extern s_level *FDECL(Is_special, (d_level *));
extern branch *FDECL(Is_branchlev, (d_level *));
extern signed char FDECL(ledger_no, (d_level *));
extern signed char NDECL(maxledgerno);
extern signed char FDECL(depth, (d_level *));
extern signed char FDECL(dunlev, (d_level *));
extern signed char FDECL(dunlevs_in_dungeon, (d_level *));
extern signed char FDECL(ledger_to_dnum, (signed char));
extern signed char FDECL(ledger_to_dlev, (signed char));
extern signed char FDECL(deepest_lev_reached, (boolean));
extern boolean FDECL(on_level, (d_level *,d_level *));
extern void FDECL(next_level, (boolean));
extern void FDECL(prev_level, (boolean));
extern void FDECL(u_on_newpos, (int,int));
extern void NDECL(u_on_sstairs);
extern void NDECL(u_on_upstairs);
extern void NDECL(u_on_dnstairs);
extern boolean FDECL(On_stairs, (signed char,signed char));
extern void FDECL(get_level, (d_level *,int));
extern boolean FDECL(Is_botlevel, (d_level *));
extern boolean FDECL(Can_fall_thru, (d_level *));
extern boolean FDECL(Can_dig_down, (d_level *));
extern boolean FDECL(Can_rise_up, (int,int,d_level *));
extern boolean FDECL(In_quest, (d_level *));
extern boolean FDECL(In_mines, (d_level *));
extern branch *FDECL(dungeon_branch, (const char *));
extern boolean FDECL(at_dgn_entrance, (const char *));
extern boolean FDECL(In_hell, (d_level *));
extern boolean FDECL(In_V_tower, (d_level *));
extern boolean FDECL(On_W_tower_level, (d_level *));
extern boolean FDECL(In_W_tower, (int,int,d_level *));
extern void FDECL(find_hell, (d_level *));
extern void FDECL(goto_hell, (boolean,boolean));
extern void FDECL(assign_level, (d_level *,d_level *));
extern void FDECL(assign_rnd_level, (d_level *,d_level *,int));
extern int FDECL(induced_align, (int));
extern boolean FDECL(Invocation_lev, (d_level *));
extern signed char NDECL(level_difficulty);
extern signed char FDECL(lev_by_name, (const char *));
#ifdef WIZARD
extern signed char FDECL(print_dungeon, (boolean,signed char *,signed char *));
#endif

/* ### eat.c ### */

#ifdef USE_TRAMPOLI
extern int NDECL(eatmdone);
extern int NDECL(eatfood);
extern int NDECL(opentin);
extern int NDECL(unfaint);
#endif
extern boolean FDECL(is_edible, (struct obj *));
extern void NDECL(init_uhunger);
extern int NDECL(Hear_again);
extern void NDECL(reset_eat);
extern int NDECL(doeat);
extern void NDECL(gethungry);
extern void FDECL(morehungry, (int));
extern void FDECL(lesshungry, (int));
extern boolean NDECL(is_fainted);
extern void NDECL(reset_faint);
extern void NDECL(violated_vegetarian);
extern void FDECL(newuhs, (boolean));
extern struct obj *FDECL(floorfood, (const char *,int));
extern void NDECL(vomit);
extern int FDECL(eaten_stat, (int,struct obj *));
extern void FDECL(food_disappears, (struct obj *));
extern void FDECL(food_substitution, (struct obj *,struct obj *));
extern void NDECL(fix_petrification);
extern void FDECL(consume_oeaten, (struct obj *,int));
extern boolean FDECL(maybe_finished_meal, (boolean));

/* ### end.c ### */

extern void FDECL(done1, (int));
extern int NDECL(done2);
#ifdef USE_TRAMPOLI
extern void FDECL(done_intr, (int));
#endif
extern void FDECL(done_in_by, (struct monst *));
#endif /* !MAKEDEFS_C && !LEV_LEX_C */
extern void VDECL(panic, (const char *,...)) PRINTF_F(1,2);
#if !defined(MAKEDEFS_C) && !defined(LEV_LEX_C)
extern void FDECL(done, (int));
extern void FDECL(container_contents, (struct obj *,boolean,boolean));
#ifdef DUMP_LOG
extern void FDECL(dump, (char *, char *));
extern void FDECL(do_containerconts, (struct obj *,boolean,boolean,boolean));
#endif
extern void FDECL(terminate, (int));
extern int NDECL(num_genocides);

/* ### engrave.c ### */

extern char *FDECL(random_engraving, (char *));
extern void FDECL(wipeout_text, (char *,int,unsigned));
extern boolean NDECL(can_reach_floor);
extern const char *FDECL(surface, (int,int));
extern const char *FDECL(ceiling, (int,int));
extern struct engr *FDECL(engr_at, (signed char,signed char));
#ifdef ELBERETH
extern int FDECL(sengr_at, (const char *,signed char,signed char));
#endif
extern void FDECL(u_wipe_engr, (int));
extern void FDECL(wipe_engr_at, (signed char,signed char,signed char));
extern void FDECL(read_engr_at, (int,int));
extern void FDECL(make_engr_at, (int,int,const char *,long,signed char));
extern void FDECL(del_engr_at, (int,int));
extern int NDECL(freehand);
extern int NDECL(doengrave);
extern void FDECL(save_engravings, (int,int));
extern void FDECL(rest_engravings, (int));
extern void FDECL(del_engr, (struct engr *));
extern void FDECL(rloc_engr, (struct engr *));
extern void FDECL(make_grave, (int,int,const char *));

/* ### exper.c ### */

extern int FDECL(experience, (struct monst *,int));
extern void FDECL(more_experienced, (int,int));
extern void FDECL(losexp, (const char *));
extern void NDECL(newexplevel);
extern void FDECL(pluslvl, (boolean));
extern long FDECL(rndexp, (boolean));

/* ### explode.c ### */

extern void FDECL(explode, (int,int,int,int,char,int));
extern long FDECL(scatter, (int, int, int, unsigned int, struct obj *));
extern void FDECL(splatter_burning_oil, (int, int));

/* ### extralev.c ### */

#ifdef REINCARNATION
extern void NDECL(makeroguerooms);
extern void FDECL(corr, (int,int));
extern void NDECL(makerogueghost);
#endif

/* ### files.c ### */

extern char *FDECL(fname_encode, (const char *, char, char *, char *, int));
extern char *FDECL(fname_decode, (char, char *, char *, int));
extern const char *FDECL(fqname, (const char *, int, int));
extern FILE *FDECL(fopen_datafile, (const char *,const char *,int));
extern boolean FDECL(uptodate, (int,const char *));
extern void FDECL(store_version, (int));
extern void FDECL(set_levelfile_name, (char *,int));
extern int FDECL(create_levelfile, (int,char *));
extern int FDECL(open_levelfile, (int,char *));
extern void FDECL(delete_levelfile, (int));
extern void NDECL(clearlocks);
extern int FDECL(create_bonesfile, (d_level*,char **, char *));
extern void FDECL(commit_bonesfile, (d_level *));
extern int FDECL(open_bonesfile, (d_level*,char **));
extern int FDECL(delete_bonesfile, (d_level*));
extern void NDECL(compress_bonesfile);
extern void NDECL(set_savefile_name);
#ifdef INSURANCE
extern void FDECL(save_savefile_name, (int));
#endif
#if defined(WIZARD)
extern void NDECL(set_error_savefile);
#endif
extern int NDECL(create_savefile);
extern int NDECL(open_savefile);
extern int NDECL(delete_savefile);
extern int NDECL(restore_saved_game);
extern void FDECL(compress, (const char *));
extern void FDECL(uncompress, (const char *));
extern boolean FDECL(lock_file, (const char *,int,int));
extern void FDECL(unlock_file, (const char *));
#ifdef USER_SOUNDS
extern boolean FDECL(can_read_file, (const char *));
#endif
extern void FDECL(read_config_file, (const char *));
extern void FDECL(check_recordfile, (const char *));
#if defined(WIZARD)
extern void NDECL(read_wizkit);
#endif
extern void FDECL(paniclog, (const char *, const char *));
extern int FDECL(validate_prefix_locations, (char *));
extern char** NDECL(get_saved_games);
extern void FDECL(free_saved_games, (char**));
#ifdef SELF_RECOVER
extern boolean NDECL(recover_savefile);
#endif
#ifdef HOLD_LOCKFILE_OPEN
extern void NDECL(really_close);
#endif

/* ### fountain.c ### */

extern void FDECL(floating_above, (const char *));
extern void FDECL(dogushforth, (int));
# ifdef USE_TRAMPOLI
extern void FDECL(gush, (int,int,void *));
# endif
extern void FDECL(dryup, (signed char,signed char, boolean));
extern void NDECL(drinkfountain);
extern void FDECL(dipfountain, (struct obj *));
#ifdef SINKS
extern void FDECL(breaksink, (int,int));
extern void NDECL(drinksink);
#endif

/* ### hack.c ### */

extern boolean FDECL(revive_nasty, (int,int,const char*));
extern void FDECL(movobj, (struct obj *,signed char,signed char));
extern boolean FDECL(may_dig, (signed char,signed char));
extern boolean FDECL(may_passwall, (signed char,signed char));
extern boolean FDECL(bad_rock, (struct permonst *,signed char,signed char));
extern boolean FDECL(invocation_pos, (signed char,signed char));
extern boolean FDECL(test_move, (int, int, int, int, int));
extern void NDECL(domove);
extern void NDECL(invocation_message);
extern void FDECL(spoteffects, (boolean));
extern char *FDECL(in_rooms, (signed char,signed char,int));
extern boolean FDECL(in_town, (int,int));
extern void FDECL(check_special_room, (boolean));
extern int NDECL(dopickup);
extern void NDECL(lookaround);
extern int NDECL(monster_nearby);
extern void FDECL(nomul, (int));
extern void FDECL(unmul, (const char *));
extern void FDECL(losehp, (int,const char *,boolean));
extern int NDECL(weight_cap);
extern int NDECL(inv_weight);
extern int NDECL(near_capacity);
extern int FDECL(calc_capacity, (int));
extern int NDECL(max_capacity);
extern boolean FDECL(check_capacity, (const char *));
extern int NDECL(inv_cnt);
#ifdef GOLDOBJ
extern long FDECL(money_cnt, (struct obj *));
#endif

/* ### hacklib.c ### */

extern boolean FDECL(digit, (char));
extern boolean FDECL(letter, (char));
extern char FDECL(highc, (char));
extern char FDECL(lowc, (char));
extern char *FDECL(lcase, (char *));
extern char *FDECL(upstart, (char *));
extern char *FDECL(mungspaces, (char *));
extern char *FDECL(eos, (char *));
extern char *FDECL(strkitten, (char *,char));
extern char *FDECL(s_suffix, (const char *));
extern char *FDECL(xcrypt, (const char *,char *));
extern boolean FDECL(onlyspace, (const char *));
extern char *FDECL(tabexpand, (char *));
extern char *FDECL(visctrl, (char));
extern const char *FDECL(ordin, (int));
extern char *FDECL(sitoa, (int));
extern int FDECL(sgn, (int));
extern int FDECL(rounddiv, (long,int));
extern int FDECL(dist2, (int,int,int,int));
extern int FDECL(distmin, (int,int,int,int));
extern boolean FDECL(online2, (int,int,int,int));
extern boolean FDECL(pmatch, (const char *,const char *));
#ifndef STRNCMPI
extern int FDECL(strncmpi, (const char *,const char *,int));
#endif
#ifndef STRSTRI
extern char *FDECL(strstri, (const char *,const char *));
#endif
extern boolean FDECL(fuzzymatch, (const char *,const char *,const char *,boolean));
extern void NDECL(setrandom);
extern int NDECL(getyear);
extern long FDECL(yyyymmdd, (time_t));
extern int NDECL(phase_of_the_moon);
extern boolean NDECL(friday_13th);
extern int NDECL(night);
extern int NDECL(midnight);

/* ### invent.c ### */

extern void FDECL(assigninvlet, (struct obj *));
extern struct obj *FDECL(merge_choice, (struct obj *,struct obj *));
extern int FDECL(merged, (struct obj **,struct obj **));
#ifdef USE_TRAMPOLI
extern int FDECL(ckunpaid, (struct obj *));
#endif
extern void FDECL(addinv_core1, (struct obj *));
extern void FDECL(addinv_core2, (struct obj *));
extern struct obj *FDECL(addinv, (struct obj *));
extern struct obj *FDECL(hold_another_object,
			(struct obj *,const char *,const char *,const char *));
extern void FDECL(useupall, (struct obj *));
extern void FDECL(useup, (struct obj *));
extern void FDECL(consume_obj_charge, (struct obj *,boolean));
extern void FDECL(freeinv_core, (struct obj *));
extern void FDECL(freeinv, (struct obj *));
extern void FDECL(delallobj, (int,int));
extern void FDECL(delobj, (struct obj *));
extern struct obj *FDECL(sobj_at, (int,int,int));
extern struct obj *FDECL(carrying, (int));
extern boolean NDECL(have_lizard);
extern struct obj *FDECL(o_on, (unsigned int,struct obj *));
extern boolean FDECL(obj_here, (struct obj *,int,int));
extern boolean NDECL(wearing_armor);
extern boolean FDECL(is_worn, (struct obj *));
extern struct obj *FDECL(g_at, (int,int));
extern struct obj *FDECL(mkgoldobj, (long));
extern struct obj *FDECL(getobj, (const char *,const char *));
extern int FDECL(ggetobj, (const char *,int (*)(OBJ_P),int,boolean,unsigned *));
extern void FDECL(fully_identify_obj, (struct obj *));
extern int FDECL(identify, (struct obj *));
extern void FDECL(identify_pack, (int));
extern int FDECL(askchain, (struct obj **,const char *,int,int (*)(OBJ_P),
			int (*)(OBJ_P),int,const char *));
extern void FDECL(prinv, (const char *,struct obj *,long));
extern char *FDECL(xprname, (struct obj *,const char *,char,boolean,long,long));
extern int NDECL(ddoinv);
extern char FDECL(display_inventory, (const char *,boolean));
#ifdef DUMP_LOG
extern char FDECL(dump_inventory, (const char *,boolean));
#endif
extern int FDECL(display_binventory, (int,int,boolean));
extern struct obj *FDECL(display_cinventory,(struct obj *));
extern struct obj *FDECL(display_minventory,(struct monst *,int,char *));
extern int NDECL(dotypeinv);
extern const char *FDECL(dfeature_at, (int,int,char *));
extern int FDECL(look_here, (int,boolean));
extern int NDECL(dolook);
extern boolean FDECL(will_feel_cockatrice, (struct obj *,boolean));
extern void FDECL(feel_cockatrice, (struct obj *,boolean));
extern void FDECL(stackobj, (struct obj *));
extern int NDECL(doprgold);
extern int NDECL(doprwep);
extern int NDECL(doprarm);
extern int NDECL(doprring);
extern int NDECL(dopramulet);
extern int NDECL(doprtool);
extern int NDECL(doprinuse);
extern void FDECL(useupf, (struct obj *,long));
extern char *FDECL(let_to_name, (char,boolean));
extern void NDECL(free_invbuf);
extern void NDECL(reassign);
extern int NDECL(doorganize);
extern int FDECL(count_unpaid, (struct obj *));
extern int FDECL(count_buc, (struct obj *,int));
extern void FDECL(carry_obj_effects, (struct obj *));
extern const char *FDECL(currency, (long));
extern void FDECL(silly_thing, (const char *,struct obj *));

/* ### ioctl.c ### */

extern void NDECL(getwindowsz);
extern void NDECL(getioctls);
extern void NDECL(setioctls);

/* ### light.c ### */

extern void FDECL(new_light_source, (signed char, signed char, int, int, void *));
extern void FDECL(del_light_source, (int, void *));
extern void FDECL(do_light_sources, (char **));
extern struct monst *FDECL(find_mid, (unsigned, unsigned));
extern void FDECL(save_light_sources, (int, int, int));
extern void FDECL(restore_light_sources, (int));
extern void FDECL(relink_light_sources, (boolean));
extern void FDECL(obj_move_light_source, (struct obj *, struct obj *));
extern boolean NDECL(any_light_source);
extern void FDECL(snuff_light_source, (int, int));
extern boolean FDECL(obj_sheds_light, (struct obj *));
extern boolean FDECL(obj_is_burning, (struct obj *));
extern void FDECL(obj_split_light_source, (struct obj *, struct obj *));
extern void FDECL(obj_merge_light_sources, (struct obj *,struct obj *));
extern int FDECL(candle_light_range, (struct obj *));
#ifdef WIZARD
extern int NDECL(wiz_light_sources);
#endif

/* ### lock.c ### */

#ifdef USE_TRAMPOLI
extern int NDECL(forcelock);
extern int NDECL(picklock);
#endif
extern boolean FDECL(picking_lock, (int *,int *));
extern boolean FDECL(picking_at, (int,int));
extern void NDECL(reset_pick);
extern int FDECL(pick_lock, (struct obj *));
extern int NDECL(doforce);
extern boolean FDECL(boxlock, (struct obj *,struct obj *));
extern boolean FDECL(doorlock, (struct obj *,int,int));
extern int NDECL(doopen);
extern int NDECL(doclose);

/* ### mail.c ### */

#ifdef MAIL
# ifdef UNIX
extern void NDECL(getmailstatus);
# endif
extern void NDECL(ckmailstatus);
extern void FDECL(readmail, (struct obj *));
#endif /* MAIL */

/* ### makemon.c ### */

extern boolean FDECL(is_home_elemental, (struct permonst *));
extern struct monst *FDECL(clone_mon, (struct monst *,signed char,signed char));
extern struct monst *FDECL(makemon, (struct permonst *,int,int,int));
extern boolean FDECL(create_critters, (int,struct permonst *));
extern struct permonst *NDECL(rndmonst);
extern void FDECL(reset_rndmonst, (int));
extern struct permonst *FDECL(mkclass, (char,int));
extern int FDECL(adj_lev, (struct permonst *));
extern struct permonst *FDECL(grow_up, (struct monst *,struct monst *));
extern int FDECL(mongets, (struct monst *,int));
extern int FDECL(golemhp, (int));
extern boolean FDECL(peace_minded, (struct permonst *));
extern void FDECL(set_malign, (struct monst *));
extern void FDECL(set_mimic_sym, (struct monst *));
extern int FDECL(mbirth_limit, (int));
extern void FDECL(mimic_hit_msg, (struct monst *, short));
#ifdef GOLDOBJ
extern void FDECL(mkmonmoney, (struct monst *, long));
#endif
extern void FDECL(bagotricks, (struct obj *));
extern boolean FDECL(propagate, (int, boolean,boolean));

/* ### mapglyph.c ### */

extern void FDECL(mapglyph, (int, int *, int *, unsigned *, int, int));

/* ### mcastu.c ### */

extern int FDECL(castmu, (struct monst *,struct attack *,boolean,boolean));
extern int FDECL(buzzmu, (struct monst *,struct attack *));

/* ### mhitm.c ### */

extern int FDECL(fightm, (struct monst *));
extern int FDECL(mattackm, (struct monst *,struct monst *));
extern int FDECL(noattacks, (struct permonst *));
extern int FDECL(sleep_monst, (struct monst *,int,int));
extern void FDECL(slept_monst, (struct monst *));
extern long FDECL(attk_protection, (int));

/* ### mhitu.c ### */

extern const char *FDECL(mpoisons_subj, (struct monst *,struct attack *));
extern void NDECL(u_slow_down);
extern struct monst *NDECL(cloneu);
extern void FDECL(expels, (struct monst *,struct permonst *,boolean));
extern struct attack *FDECL(getmattk, (struct permonst *,int,int *,struct attack *));
extern int FDECL(mattacku, (struct monst *));
extern int FDECL(magic_negation, (struct monst *));
extern int FDECL(gazemu, (struct monst *,struct attack *));
extern void FDECL(mdamageu, (struct monst *,int));
extern int FDECL(could_seduce, (struct monst *,struct monst *,struct attack *));
#ifdef SEDUCE
extern int FDECL(doseduce, (struct monst *));
#endif

/* ### minion.c ### */

extern void FDECL(msummon, (struct monst *));
extern void FDECL(summon_minion, (aligntyp,boolean));
extern int FDECL(demon_talk, (struct monst *));
extern long FDECL(bribe, (struct monst *));
extern int FDECL(dprince, (aligntyp));
extern int FDECL(dlord, (aligntyp));
extern int NDECL(llord);
extern int FDECL(ndemon, (aligntyp));
extern int NDECL(lminion);

/* ### mklev.c ### */

#ifdef USE_TRAMPOLI
extern int FDECL(do_comp, (void *,void *));
#endif
extern void NDECL(sort_rooms);
extern void FDECL(add_room, (int,int,int,int,boolean,signed char,boolean));
extern void FDECL(add_subroom, (struct mkroom *,int,int,int,int,
			   boolean,signed char,boolean));
extern void NDECL(makecorridors);
extern void FDECL(add_door, (int,int,struct mkroom *));
extern void NDECL(mklev);
#ifdef SPECIALIZATION
extern void FDECL(topologize, (struct mkroom *,boolean));
#else
extern void FDECL(topologize, (struct mkroom *));
#endif
extern void FDECL(place_branch, (branch *,signed char,signed char));
extern boolean FDECL(occupied, (signed char,signed char));
extern int FDECL(okdoor, (signed char,signed char));
extern void FDECL(dodoor, (int,int,struct mkroom *));
extern void FDECL(mktrap, (int,int,struct mkroom *,coord*));
extern void FDECL(mkstairs, (signed char,signed char,char,struct mkroom *));
extern void NDECL(mkinvokearea);

/* ### mkmap.c ### */

void FDECL(flood_fill_rm, (int,int,int,boolean,boolean));
void FDECL(remove_rooms, (int,int,int,int));

/* ### mkmaze.c ### */

extern void FDECL(wallification, (int,int,int,int));
extern void FDECL(walkfrom, (int,int));
extern void FDECL(makemaz, (const char *));
extern void FDECL(mazexy, (coord *));
extern void NDECL(bound_digging);
extern void FDECL(mkportal, (signed char,signed char,signed char,signed char));
extern boolean FDECL(bad_location, (signed char,signed char,signed char,signed char,signed char,signed char));
extern void FDECL(place_lregion, (signed char,signed char,signed char,signed char,
			     signed char,signed char,signed char,signed char,
			     signed char,d_level *));
extern void NDECL(movebubbles);
extern void NDECL(water_friction);
extern void FDECL(save_waterlevel, (int,int));
extern void FDECL(restore_waterlevel, (int));
extern const char *FDECL(waterbody_name, (signed char,signed char));

/* ### mkobj.c ### */

extern struct obj *FDECL(mkobj_at, (char,int,int,boolean));
extern struct obj *FDECL(mksobj_at, (int,int,int,boolean,boolean));
extern struct obj *FDECL(mkobj, (char,boolean));
extern int NDECL(rndmonnum);
extern struct obj *FDECL(splitobj, (struct obj *,long));
extern void FDECL(replace_object, (struct obj *,struct obj *));
extern void FDECL(bill_dummy_object, (struct obj *));
extern struct obj *FDECL(mksobj, (int,boolean,boolean));
extern int FDECL(bcsign, (struct obj *));
extern int FDECL(weight, (struct obj *));
extern struct obj *FDECL(mkgold, (long,int,int));
extern struct obj *FDECL(mkcorpstat,
		(int,struct monst *,struct permonst *,int,int,boolean));
extern struct obj *FDECL(obj_attach_mid, (struct obj *, unsigned));
extern struct monst *FDECL(get_mtraits, (struct obj *, boolean));
extern struct obj *FDECL(mk_tt_object, (int,int,int));
extern struct obj *FDECL(mk_named_object,
			(int,struct permonst *,int,int,const char *));
extern struct obj *FDECL(rnd_treefruit_at, (int, int));
extern void FDECL(start_corpse_timeout, (struct obj *));
extern void FDECL(bless, (struct obj *));
extern void FDECL(unbless, (struct obj *));
extern void FDECL(curse, (struct obj *));
extern void FDECL(uncurse, (struct obj *));
extern void FDECL(blessorcurse, (struct obj *,int));
extern boolean FDECL(is_flammable, (struct obj *));
extern boolean FDECL(is_rottable, (struct obj *));
extern void FDECL(place_object, (struct obj *,int,int));
extern void FDECL(remove_object, (struct obj *));
extern void FDECL(discard_minvent, (struct monst *));
extern void FDECL(obj_extract_self, (struct obj *));
extern void FDECL(extract_nobj, (struct obj *, struct obj **));
extern void FDECL(extract_nexthere, (struct obj *, struct obj **));
extern int FDECL(add_to_minv, (struct monst *, struct obj *));
extern struct obj *FDECL(add_to_container, (struct obj *, struct obj *));
extern void FDECL(add_to_migration, (struct obj *));
extern void FDECL(add_to_buried, (struct obj *));
extern void FDECL(dealloc_obj, (struct obj *));
extern void FDECL(obj_ice_effects, (int, int, boolean));
extern long FDECL(peek_at_iced_corpse_age, (struct obj *));
#ifdef WIZARD
extern void NDECL(obj_sanity_check);
#endif

/* ### mkroom.c ### */

extern void FDECL(mkroom, (int));
extern void FDECL(fill_zoo, (struct mkroom *));
extern boolean FDECL(nexttodoor, (int,int));
extern boolean FDECL(has_dnstairs, (struct mkroom *));
extern boolean FDECL(has_upstairs, (struct mkroom *));
extern int FDECL(somex, (struct mkroom *));
extern int FDECL(somey, (struct mkroom *));
extern boolean FDECL(inside_room, (struct mkroom *,signed char,signed char));
extern boolean FDECL(somexy, (struct mkroom *,coord *));
extern void FDECL(mkundead, (coord *,boolean,int));
extern struct permonst *NDECL(courtmon);
extern void FDECL(save_rooms, (int));
extern void FDECL(rest_rooms, (int));
extern struct mkroom *FDECL(search_special, (signed char));

/* ### mon.c ### */

extern int FDECL(undead_to_corpse, (int));
extern int FDECL(genus, (int,int));
extern int FDECL(pm_to_cham, (int));
extern int FDECL(minliquid, (struct monst *));
extern int NDECL(movemon);
extern int FDECL(meatmetal, (struct monst *));
extern int FDECL(meatobj, (struct monst *));
extern void FDECL(mpickgold, (struct monst *));
extern boolean FDECL(mpickstuff, (struct monst *,const char *));
extern int FDECL(curr_mon_load, (struct monst *));
extern int FDECL(max_mon_load, (struct monst *));
extern boolean FDECL(can_carry, (struct monst *,struct obj *));
extern int FDECL(mfndpos, (struct monst *,coord *,long *,long));
extern boolean FDECL(monnear, (struct monst *,int,int));
extern void NDECL(dmonsfree);
extern int FDECL(mcalcmove, (struct monst*));
extern void NDECL(mcalcdistress);
extern void FDECL(replmon, (struct monst *,struct monst *));
extern void FDECL(relmon, (struct monst *));
extern struct obj *FDECL(mlifesaver, (struct monst *));
extern boolean FDECL(corpse_chance,(struct monst *,struct monst *,boolean));
extern void FDECL(mondead, (struct monst *));
extern void FDECL(mondied, (struct monst *));
extern void FDECL(mongone, (struct monst *));
extern void FDECL(monstone, (struct monst *));
extern void FDECL(monkilled, (struct monst *,const char *,int));
extern void FDECL(unstuck, (struct monst *));
extern void FDECL(killed, (struct monst *));
extern void FDECL(xkilled, (struct monst *,int));
extern void FDECL(mon_to_stone, (struct monst*));
extern void FDECL(mnexto, (struct monst *));
extern boolean FDECL(mnearto, (struct monst *,signed char,signed char,boolean));
extern void FDECL(poisontell, (int));
extern void FDECL(poisoned, (const char *,int,const char *,int));
extern void FDECL(m_respond, (struct monst *));
extern void FDECL(setmangry, (struct monst *));
extern void FDECL(wakeup, (struct monst *));
extern void NDECL(wake_nearby);
extern void FDECL(wake_nearto, (int,int,int));
extern void FDECL(seemimic, (struct monst *));
extern void NDECL(rescham);
extern void NDECL(restartcham);
extern void FDECL(restore_cham, (struct monst *));
extern void FDECL(mon_animal_list, (boolean));
extern int FDECL(newcham, (struct monst *,struct permonst *,boolean,boolean));
extern int FDECL(can_be_hatched, (int));
extern int FDECL(egg_type_from_parent, (int,boolean));
extern boolean FDECL(dead_species, (int,boolean));
extern void NDECL(kill_genocided_monsters);
extern void FDECL(golemeffects, (struct monst *,int,int));
extern boolean FDECL(angry_guards, (boolean));
extern void NDECL(pacify_guards);

/* ### mondata.c ### */

extern void FDECL(set_mon_data, (struct monst *,struct permonst *,int));
extern struct attack *FDECL(attacktype_fordmg, (struct permonst *,int,int));
extern boolean FDECL(attacktype, (struct permonst *,int));
extern boolean FDECL(poly_when_stoned, (struct permonst *));
extern boolean FDECL(resists_drli, (struct monst *));
extern boolean FDECL(resists_magm, (struct monst *));
extern boolean FDECL(resists_blnd, (struct monst *));
extern boolean FDECL(can_blnd, (struct monst *,struct monst *,unsigned char,struct obj *));
extern boolean FDECL(ranged_attk, (struct permonst *));
extern boolean FDECL(hates_silver, (struct permonst *));
extern boolean FDECL(passes_bars, (struct permonst *));
extern boolean FDECL(can_track, (struct permonst *));
extern boolean FDECL(breakarm, (struct permonst *));
extern boolean FDECL(sliparm, (struct permonst *));
extern boolean FDECL(sticks, (struct permonst *));
extern int FDECL(num_horns, (struct permonst *));
/* E boolean FDECL(canseemon, (struct monst *)); */
extern struct attack *FDECL(dmgtype_fromattack, (struct permonst *,int,int));
extern boolean FDECL(dmgtype, (struct permonst *,int));
extern int FDECL(max_passive_dmg, (struct monst *,struct monst *));
extern int FDECL(monsndx, (struct permonst *));
extern int FDECL(name_to_mon, (const char *));
extern int FDECL(gender, (struct monst *));
extern int FDECL(pronoun_gender, (struct monst *));
extern boolean FDECL(levl_follower, (struct monst *));
extern int FDECL(little_to_big, (int));
extern int FDECL(big_to_little, (int));
extern const char *FDECL(locomotion, (const struct permonst *,const char *));
extern const char *FDECL(stagger, (const struct permonst *,const char *));
extern const char *FDECL(on_fire, (struct permonst *,struct attack *));
extern const struct permonst *FDECL(raceptr, (struct monst *));

/* ### monmove.c ### */

extern boolean FDECL(itsstuck, (struct monst *));
extern boolean FDECL(mb_trapped, (struct monst *));
extern void FDECL(mon_regen, (struct monst *,boolean));
extern int FDECL(dochugw, (struct monst *));
extern boolean FDECL(onscary, (int,int,struct monst *));
extern void FDECL(monflee, (struct monst *, int, boolean, boolean));
extern int FDECL(dochug, (struct monst *));
extern int FDECL(m_move, (struct monst *,int));
extern boolean FDECL(closed_door, (int,int));
extern boolean FDECL(accessible, (int,int));
extern void FDECL(set_apparxy, (struct monst *));
extern boolean FDECL(can_ooze, (struct monst *));

/* ### monst.c ### */

extern void NDECL(monst_init);

/* ### monstr.c ### */

extern void NDECL(monstr_init);

/* ### mplayer.c ### */

extern struct monst *FDECL(mk_mplayer, (struct permonst *,signed char, signed char,boolean));
extern void FDECL(create_mplayers, (int,boolean));
extern void FDECL(mplayer_talk, (struct monst *));

/* ### mthrowu.c ### */

extern int FDECL(thitu, (int,int,struct obj *,const char *));
extern int FDECL(ohitmon, (struct monst *,struct obj *,int,boolean));
extern void FDECL(thrwmu, (struct monst *));
extern int FDECL(spitmu, (struct monst *,struct attack *));
extern int FDECL(breamu, (struct monst *,struct attack *));
extern boolean FDECL(linedup, (signed char,signed char,signed char,signed char));
extern boolean FDECL(lined_up, (struct monst *));
extern struct obj *FDECL(m_carrying, (struct monst *,int));
extern void FDECL(m_useup, (struct monst *,struct obj *));
extern void FDECL(m_throw, (struct monst *,int,int,int,int,int,struct obj *));
extern boolean FDECL(hits_bars, (struct obj **,int,int,int,int));

/* ### muse.c ### */

extern boolean FDECL(find_defensive, (struct monst *));
extern int FDECL(use_defensive, (struct monst *));
extern int FDECL(rnd_defensive_item, (struct monst *));
extern boolean FDECL(find_offensive, (struct monst *));
#ifdef USE_TRAMPOLI
extern int FDECL(mbhitm, (struct monst *,struct obj *));
#endif
extern int FDECL(use_offensive, (struct monst *));
extern int FDECL(rnd_offensive_item, (struct monst *));
extern boolean FDECL(find_misc, (struct monst *));
extern int FDECL(use_misc, (struct monst *));
extern int FDECL(rnd_misc_item, (struct monst *));
extern boolean FDECL(searches_for_item, (struct monst *,struct obj *));
extern boolean FDECL(mon_reflects, (struct monst *,const char *));
extern boolean FDECL(ureflects, (const char *,const char *));
extern boolean FDECL(munstone, (struct monst *,boolean));

/* ### music.c ### */

extern void NDECL(awaken_soldiers);
extern int FDECL(do_play_instrument, (struct obj *));

/* ### nhlan.c ### */
#ifdef LAN_FEATURES
extern void NDECL(init_lan_features);
extern char *NDECL(lan_username);
# ifdef LAN_MAIL
extern boolean NDECL(lan_mail_check);
extern void FDECL(lan_mail_read, (struct obj *));
extern void NDECL(lan_mail_init);
extern void NDECL(lan_mail_finish);
extern void NDECL(lan_mail_terminate);
# endif
#endif

/* ### o_init.c ### */

extern void NDECL(init_objects);
extern int NDECL(find_skates);
extern void NDECL(oinit);
extern void FDECL(savenames, (int,int));
extern void FDECL(restnames, (int));
extern void FDECL(discover_object, (int,boolean,boolean));
extern void FDECL(undiscover_object, (int));
extern int NDECL(dodiscovered);

/* ### objects.c ### */

extern void NDECL(objects_init);

/* ### objnam.c ### */

extern char *FDECL(obj_typename, (int));
extern char *FDECL(simple_typename, (int));
extern boolean FDECL(obj_is_pname, (struct obj *));
extern char *FDECL(distant_name, (struct obj *,char *(*)(OBJ_P)));
extern char *FDECL(fruitname, (boolean));
extern char *FDECL(xname, (struct obj *));
extern char *FDECL(mshot_xname, (struct obj *));
extern boolean FDECL(the_unique_obj, (struct obj *obj));
extern char *FDECL(doname, (struct obj *));
extern boolean FDECL(not_fully_identified, (struct obj *));
extern char *FDECL(corpse_xname, (struct obj *,boolean));
extern char *FDECL(cxname, (struct obj *));
#ifdef SORTLOOT
extern char *FDECL(cxname2, (struct obj *));
#endif
extern char *FDECL(killer_xname, (struct obj *));
extern const char *FDECL(singular, (struct obj *,char *(*)(OBJ_P)));
extern char *FDECL(an, (const char *));
extern char *FDECL(An, (const char *));
extern char *FDECL(The, (const char *));
extern char *FDECL(the, (const char *));
extern char *FDECL(aobjnam, (struct obj *,const char *));
extern char *FDECL(Tobjnam, (struct obj *,const char *));
extern char *FDECL(otense, (struct obj *,const char *));
extern char *FDECL(vtense, (const char *,const char *));
extern char *FDECL(Doname2, (struct obj *));
extern char *FDECL(yname, (struct obj *));
extern char *FDECL(Yname2, (struct obj *));
extern char *FDECL(ysimple_name, (struct obj *));
extern char *FDECL(Ysimple_name2, (struct obj *));
extern char *FDECL(makeplural, (const char *));
extern char *FDECL(makesingular, (const char *));
extern struct obj *FDECL(readobjnam, (char *,struct obj *,boolean));
extern int FDECL(rnd_class, (int,int));
extern const char *FDECL(cloak_simple_name, (struct obj *));
extern const char *FDECL(mimic_obj_name, (struct monst *));

/* ### options.c ### */

extern boolean FDECL(match_optname, (const char *,const char *,int,boolean));
extern void NDECL(initoptions);
extern void FDECL(parseoptions, (char *,boolean,boolean));
extern int NDECL(doset);
extern int NDECL(dotogglepickup);
extern void NDECL(option_help);
extern void FDECL(next_opt, (winid,const char *));
extern int FDECL(fruitadd, (char *));
extern int FDECL(choose_classes_menu, (const char *,int,boolean,char *,char *));
extern void FDECL(add_menu_cmd_alias, (char, char));
extern char FDECL(map_menu_cmd, (char));
extern void FDECL(assign_warnings, (unsigned char *));
extern char *FDECL(nh_getenv, (const char *));
extern void FDECL(set_duplicate_opt_detection, (int));
extern void FDECL(set_wc_option_mod_status, (unsigned long, int));
extern void FDECL(set_wc2_option_mod_status, (unsigned long, int));
extern void FDECL(set_option_mod_status, (const char *,int));
#ifdef AUTOPICKUP_EXCEPTIONS
extern int FDECL(add_autopickup_exception, (const char *));
extern void NDECL(free_autopickup_exceptions);
#endif /* AUTOPICKUP_EXCEPTIONS */
#ifdef MENU_COLOR
extern boolean FDECL(add_menu_coloring, (char *));
#endif /* MENU_COLOR */

/* ### pager.c ### */

extern int NDECL(dowhatis);
extern int NDECL(doquickwhatis);
extern int NDECL(doidtrap);
extern int NDECL(dowhatdoes);
extern char *FDECL(dowhatdoes_core,(char, char *));
extern int NDECL(dohelp);
extern int NDECL(dohistory);

/* ### pcunix.c ### */

#if defined(PC_LOCKING)
extern void NDECL(getlock);
#endif

/* ### pickup.c ### */

#ifdef GOLDOBJ
extern int FDECL(collect_obj_classes,
	(char *,struct obj *,boolean,boolean FDECL((*),(OBJ_P)), int *));
#else
extern int FDECL(collect_obj_classes,
	(char *,struct obj *,boolean,boolean,boolean FDECL((*),(OBJ_P)), int *));
#endif
extern void FDECL(add_valid_menu_class, (int));
extern boolean FDECL(allow_all, (struct obj *));
extern boolean FDECL(allow_category, (struct obj *));
extern boolean FDECL(is_worn_by_type, (struct obj *));
#ifdef USE_TRAMPOLI
extern int FDECL(ck_bag, (struct obj *));
extern int FDECL(in_container, (struct obj *));
extern int FDECL(out_container, (struct obj *));
#endif
extern int FDECL(pickup, (int));
extern int FDECL(pickup_object, (struct obj *, long, boolean));
extern int FDECL(query_category, (const char *, struct obj *, int,
				menu_item **, int));
extern int FDECL(query_objlist, (const char *, struct obj *, int,
				menu_item **, int, boolean (*)(OBJ_P)));
extern struct obj *FDECL(pick_obj, (struct obj *));
extern int NDECL(encumber_msg);
extern int NDECL(doloot);
extern int FDECL(use_container, (struct obj *,int));
extern int FDECL(loot_mon, (struct monst *,int *,boolean *));
extern const char *FDECL(safe_qbuf, (const char *,unsigned,
				const char *,const char *,const char *));
extern boolean FDECL(is_autopickup_exception, (struct obj *, boolean));

/* ### pline.c ### */

extern void VDECL(pline, (const char *,...)) PRINTF_F(1,2);
extern void FDECL(plines, (const char *));
extern void VDECL(Norep, (const char *,...)) PRINTF_F(1,2);
extern void NDECL(free_youbuf);
extern void VDECL(You, (const char *,...)) PRINTF_F(1,2);
extern void VDECL(Your, (const char *,...)) PRINTF_F(1,2);
extern void VDECL(You_feel, (const char *,...)) PRINTF_F(1,2);
extern void VDECL(You_cant, (const char *,...)) PRINTF_F(1,2);
extern void VDECL(You_hear, (const char *,...)) PRINTF_F(1,2);
extern void VDECL(pline_The, (const char *,...)) PRINTF_F(1,2);
extern void VDECL(There, (const char *,...)) PRINTF_F(1,2);
extern void VDECL(verbalize, (const char *,...)) PRINTF_F(1,2);
extern void VDECL(raw_printf, (const char *,...)) PRINTF_F(1,2);
extern void VDECL(impossible, (const char *,...)) PRINTF_F(1,2);
extern const char *FDECL(align_str, (aligntyp));
extern void FDECL(mstatusline, (struct monst *));
extern void NDECL(ustatusline);
extern void NDECL(self_invis_message);

/* ### polyself.c ### */

extern void NDECL(set_uasmon);
extern void NDECL(change_sex);
extern void FDECL(polyself, (boolean));
extern int FDECL(polymon, (int));
extern void NDECL(rehumanize);
extern int NDECL(dobreathe);
extern int NDECL(dospit);
extern int NDECL(doremove);
extern int NDECL(dospinweb);
extern int NDECL(dosummon);
extern int NDECL(dogaze);
extern int NDECL(dohide);
extern int NDECL(domindblast);
extern void FDECL(skinback, (boolean));
extern const char *FDECL(mbodypart, (struct monst *,int));
extern const char *FDECL(body_part, (int));
extern int NDECL(poly_gender);
extern void FDECL(ugolemeffects, (int,int));

/* ### potion.c ### */

extern void FDECL(set_itimeout, (long *,long));
extern void FDECL(incr_itimeout, (long *,int));
extern void FDECL(make_confused, (long,boolean));
extern void FDECL(make_stunned, (long,boolean));
extern void FDECL(make_blinded, (long,boolean));
extern void FDECL(make_sick, (long, const char *, boolean,int));
extern void FDECL(make_vomiting, (long,boolean));
extern boolean FDECL(make_hallucinated, (long,boolean,long));
extern int NDECL(dodrink);
extern int FDECL(dopotion, (struct obj *));
extern int FDECL(peffects, (struct obj *));
extern void FDECL(healup, (int,int,boolean,boolean));
extern void FDECL(strange_feeling, (struct obj *,const char *));
extern void FDECL(potionhit, (struct monst *,struct obj *,boolean));
extern void FDECL(potionbreathe, (struct obj *));
extern boolean FDECL(get_wet, (struct obj *));
extern int NDECL(dodip);
extern void FDECL(djinni_from_bottle, (struct obj *));
extern struct monst *FDECL(split_mon, (struct monst *,struct monst *));
extern const char *NDECL(bottlename);

/* ### pray.c ### */

#ifdef USE_TRAMPOLI
extern int NDECL(prayer_done);
#endif
extern int NDECL(dosacrifice);
extern boolean FDECL(can_pray, (boolean));
extern int NDECL(dopray);
extern const char *NDECL(u_gname);
extern int NDECL(doturn);
extern const char *NDECL(a_gname);
extern const char *FDECL(a_gname_at, (signed char x,signed char y));
extern const char *FDECL(align_gname, (aligntyp));
extern const char *FDECL(halu_gname, (aligntyp));
extern const char *FDECL(align_gtitle, (aligntyp));
extern void FDECL(altar_wrath, (int,int));


/* ### priest.c ### */

extern int FDECL(move_special, (struct monst *,boolean,signed char,boolean,boolean,
			   signed char,signed char,signed char,signed char));
extern char FDECL(temple_occupied, (char *));
extern int FDECL(pri_move, (struct monst *));
extern void FDECL(priestini, (d_level *,struct mkroom *,int,int,boolean));
extern char *FDECL(priestname, (struct monst *,char *));
extern boolean FDECL(p_coaligned, (struct monst *));
extern struct monst *FDECL(findpriest, (char));
extern void FDECL(intemple, (int));
extern void FDECL(priest_talk, (struct monst *));
extern struct monst *FDECL(mk_roamer, (struct permonst *,aligntyp, signed char,signed char,boolean));
extern void FDECL(reset_hostility, (struct monst *));
extern boolean FDECL(in_your_sanctuary, (struct monst *,signed char,signed char));
extern void FDECL(ghod_hitsu, (struct monst *));
extern void NDECL(angry_priest);
extern void NDECL(clearpriests);
extern void FDECL(restpriest, (struct monst *,boolean));

/* ### quest.c ### */

extern void NDECL(onquest);
extern void NDECL(nemdead);
extern void NDECL(artitouch);
extern boolean NDECL(ok_to_quest);
extern void FDECL(leader_speaks, (struct monst *));
extern void NDECL(nemesis_speaks);
extern void FDECL(quest_chat, (struct monst *));
extern void FDECL(quest_talk, (struct monst *));
extern void FDECL(quest_stat_check, (struct monst *));
extern void FDECL(finish_quest, (struct obj *));

/* ### questpgr.c ### */

extern void NDECL(load_qtlist);
extern void NDECL(unload_qtlist);
extern short FDECL(quest_info, (int));
extern const char *NDECL(ldrname);
extern boolean FDECL(is_quest_artifact, (struct obj*));
extern void FDECL(com_pager, (int));
extern void FDECL(qt_pager, (int));
extern struct permonst *NDECL(qt_montype);

/* ### read.c ### */

extern int NDECL(doread);
extern boolean FDECL(is_chargeable, (struct obj *));
extern void FDECL(recharge, (struct obj *,int));
extern void FDECL(forget_objects, (int));
extern void FDECL(forget_levels, (int));
extern void NDECL(forget_traps);
extern void FDECL(forget_map, (int));
extern int FDECL(seffects, (struct obj *));
#ifdef USE_TRAMPOLI
extern void FDECL(set_lit, (int,int,void *));
#endif
extern void FDECL(litroom, (boolean,struct obj *));
extern void FDECL(do_genocide, (int));
extern void FDECL(punish, (struct obj *));
extern void NDECL(unpunish);
extern boolean FDECL(cant_create, (int *, boolean));
#ifdef WIZARD
extern boolean NDECL(create_particular);
#endif

/* ### rect.c ### */

extern void NDECL(init_rect);
extern NhRect *FDECL(get_rect, (NhRect *));
extern NhRect *NDECL(rnd_rect);
extern void FDECL(remove_rect, (NhRect *));
extern void FDECL(add_rect, (NhRect *));
extern void FDECL(split_rects, (NhRect *,NhRect *));

/* ## region.c ### */
extern void NDECL(clear_regions);
extern void NDECL(run_regions);
extern boolean FDECL(in_out_region, (signed char,signed char));
extern boolean FDECL(m_in_out_region, (struct monst *,signed char,signed char));
extern void NDECL(update_player_regions);
extern void FDECL(update_monster_region, (struct monst *));
extern NhRegion *FDECL(visible_region_at, (signed char,signed char));
extern void FDECL(show_region, (NhRegion*, signed char, signed char));
extern void FDECL(save_regions, (int,int));
extern void FDECL(rest_regions, (int,boolean));
extern NhRegion* FDECL(create_gas_cloud, (signed char, signed char, int, int));

/* ### restore.c ### */

extern void FDECL(inven_inuse, (boolean));
extern int FDECL(dorecover, (int));
extern void FDECL(trickery, (char *));
extern void FDECL(getlev, (int,int,signed char,boolean));
extern void NDECL(minit);
extern boolean FDECL(lookup_id_mapping, (unsigned, unsigned *));
#ifdef ZEROCOMP
extern int FDECL(mread, (int,void *,unsigned int));
#else
extern void FDECL(mread, (int,void *,unsigned int));
#endif

/* ### rip.c ### */

extern void FDECL(genl_outrip, (winid,int));

/* ### rnd.c ### */

extern int FDECL(rn2, (int));
extern int FDECL(rnl, (int));
extern int FDECL(rnd, (int));
extern int FDECL(d, (int,int));
extern int FDECL(rne, (int));
extern int FDECL(rnz, (int));

/* ### role.c ### */

extern boolean FDECL(validrole, (int));
extern boolean FDECL(validrace, (int, int));
extern boolean FDECL(validgend, (int, int, int));
extern boolean FDECL(validalign, (int, int, int));
extern int NDECL(randrole);
extern int FDECL(randrace, (int));
extern int FDECL(randgend, (int, int));
extern int FDECL(randalign, (int, int));
extern int FDECL(str2role, (char *));
extern int FDECL(str2race, (char *));
extern int FDECL(str2gend, (char *));
extern int FDECL(str2align, (char *));
extern boolean FDECL(ok_role, (int, int, int, int));
extern int FDECL(pick_role, (int, int, int, int));
extern boolean FDECL(ok_race, (int, int, int, int));
extern int FDECL(pick_race, (int, int, int, int));
extern boolean FDECL(ok_gend, (int, int, int, int));
extern int FDECL(pick_gend, (int, int, int, int));
extern boolean FDECL(ok_align, (int, int, int, int));
extern int FDECL(pick_align, (int, int, int, int));
extern void NDECL(role_init);
extern void NDECL(rigid_role_checks);
extern void NDECL(plnamesuffix);
extern const char *FDECL(Hello, (struct monst *));
extern const char *NDECL(Goodbye);
extern char *FDECL(build_plselection_prompt, (char *, int, int, int, int, int));
extern char *FDECL(root_plselection_prompt, (char *, int, int, int, int, int));

/* ### rumors.c ### */

extern char *FDECL(getrumor, (int,char *, boolean));
extern void FDECL(outrumor, (int,int));
extern void FDECL(outoracle, (boolean, boolean));
extern void FDECL(save_oracles, (int,int));
extern void FDECL(restore_oracles, (int));
extern int FDECL(doconsult, (struct monst *));

/* ### save.c ### */

extern int NDECL(dosave);
extern void FDECL(hangup, (int));
extern int NDECL(dosave0);
#ifdef INSURANCE
extern void NDECL(savestateinlock);
#endif
extern void FDECL(savelev, (int,signed char,int));
extern void FDECL(bufon, (int));
extern void FDECL(bufoff, (int));
extern void FDECL(bflush, (int));
extern void FDECL(bwrite, (int,void *,unsigned int));
extern void FDECL(bclose, (int));
extern void FDECL(savefruitchn, (int,int));
extern void NDECL(free_dungeons);
extern void NDECL(freedynamicdata);

/* ### shk.c ### */

#ifdef GOLDOBJ
extern long FDECL(money2mon, (struct monst *, long));
extern void FDECL(money2u, (struct monst *, long));
#endif
extern char *FDECL(shkname, (struct monst *));
extern void FDECL(shkgone, (struct monst *));
extern void FDECL(set_residency, (struct monst *,boolean));
extern void FDECL(replshk, (struct monst *,struct monst *));
extern void FDECL(restshk, (struct monst *,boolean));
extern char FDECL(inside_shop, (signed char,signed char));
extern void FDECL(u_left_shop, (char *,boolean));
extern void FDECL(remote_burglary, (signed char,signed char));
extern void FDECL(u_entered_shop, (char *));
extern boolean FDECL(same_price, (struct obj *,struct obj *));
extern void NDECL(shopper_financial_report);
extern int FDECL(inhishop, (struct monst *));
extern struct monst *FDECL(shop_keeper, (char));
extern boolean FDECL(tended_shop, (struct mkroom *));
extern void FDECL(delete_contents, (struct obj *));
extern void FDECL(obfree, (struct obj *,struct obj *));
extern void FDECL(home_shk, (struct monst *,boolean));
extern void FDECL(make_happy_shk, (struct monst *,boolean));
extern void FDECL(hot_pursuit, (struct monst *));
extern void FDECL(make_angry_shk, (struct monst *,signed char,signed char));
extern int NDECL(dopay);
extern boolean FDECL(paybill, (int));
extern void NDECL(finish_paybill);
extern struct obj *FDECL(find_oid, (unsigned));
extern long FDECL(contained_cost, (struct obj *,struct monst *,long,boolean, boolean));
extern long FDECL(contained_gold, (struct obj *));
extern void FDECL(picked_container, (struct obj *));
extern long FDECL(unpaid_cost, (struct obj *));
extern void FDECL(addtobill, (struct obj *,boolean,boolean,boolean));
extern void FDECL(splitbill, (struct obj *,struct obj *));
extern void FDECL(subfrombill, (struct obj *,struct monst *));
extern long FDECL(stolen_value, (struct obj *,signed char,signed char,boolean,boolean));
extern void FDECL(sellobj_state, (int));
extern void FDECL(sellobj, (struct obj *,signed char,signed char));
extern int FDECL(doinvbill, (int));
extern struct monst *FDECL(shkcatch, (struct obj *,signed char,signed char));
extern void FDECL(add_damage, (signed char,signed char,long));
extern int FDECL(repair_damage, (struct monst *,struct damage *,boolean));
extern int FDECL(shk_move, (struct monst *));
extern void FDECL(after_shk_move, (struct monst *));
extern boolean FDECL(is_fshk, (struct monst *));
extern void FDECL(shopdig, (int));
extern void FDECL(pay_for_damage, (const char *,boolean));
extern boolean FDECL(costly_spot, (signed char,signed char));
extern struct obj *FDECL(shop_object, (signed char,signed char));
extern void FDECL(price_quote, (struct obj *));
extern void FDECL(shk_chat, (struct monst *));
extern void FDECL(check_unpaid_usage, (struct obj *,boolean));
extern void FDECL(check_unpaid, (struct obj *));
extern void FDECL(costly_gold, (signed char,signed char,long));
extern boolean FDECL(block_door, (signed char,signed char));
extern boolean FDECL(block_entry, (signed char,signed char));
extern char *FDECL(shk_your, (char *,struct obj *));
extern char *FDECL(Shk_Your, (char *,struct obj *));

/* ### shknam.c ### */

extern void FDECL(stock_room, (int,struct mkroom *));
extern boolean FDECL(saleable, (struct monst *,struct obj *));
extern int FDECL(get_shop_item, (int));

/* ### sit.c ### */

extern void NDECL(take_gold);
extern int NDECL(dosit);
extern void NDECL(rndcurse);
extern void NDECL(attrcurse);

/* ### sounds.c ### */

extern void NDECL(dosounds);
extern const char *FDECL(growl_sound, (struct monst *));
extern void FDECL(growl, (struct monst *));
extern void FDECL(yelp, (struct monst *));
extern void FDECL(whimper, (struct monst *));
extern void FDECL(beg, (struct monst *));
extern int NDECL(dotalk);
#ifdef USER_SOUNDS
extern int FDECL(add_sound_mapping, (const char *));
extern void FDECL(play_sound_for_message, (const char *));
#endif

/* ### sp_lev.c ### */

extern boolean FDECL(check_room, (signed char *,signed char *,signed char *,signed char *,boolean));
extern boolean FDECL(create_room, (signed char,signed char,signed char,signed char,
			      signed char,signed char,signed char,signed char));
extern void FDECL(create_secret_door, (struct mkroom *,signed char));
extern boolean FDECL(dig_corridor, (coord *,coord *,boolean,signed char,signed char));
extern void FDECL(fill_room, (struct mkroom *,boolean));
extern boolean FDECL(load_special, (const char *));

/* ### spell.c ### */

#ifdef USE_TRAMPOLI
extern int NDECL(learn);
#endif
extern int FDECL(study_book, (struct obj *));
extern void FDECL(book_disappears, (struct obj *));
extern void FDECL(book_substitution, (struct obj *,struct obj *));
extern void NDECL(age_spells);
extern int NDECL(docast);
extern int FDECL(spell_skilltype, (int));
extern int FDECL(spelleffects, (int,boolean));
extern void NDECL(losespells);
extern int NDECL(dovspell);
extern void FDECL(initialspell, (struct obj *));

/* ### steal.c ### */

#ifdef USE_TRAMPOLI
extern int NDECL(stealarm);
#endif
#ifdef GOLDOBJ
extern long FDECL(somegold, (long));
#else
extern long NDECL(somegold);
#endif
extern void FDECL(stealgold, (struct monst *));
extern void FDECL(remove_worn_item, (struct obj *,boolean));
extern int FDECL(steal, (struct monst *, char *));
extern int FDECL(mpickobj, (struct monst *,struct obj *));
extern void FDECL(stealamulet, (struct monst *));
extern void FDECL(mdrop_special_objs, (struct monst *));
extern void FDECL(relobj, (struct monst *,int,boolean));
#ifdef GOLDOBJ
extern struct obj *FDECL(findgold, (struct obj *));
#endif

/* ### steed.c ### */

#ifdef STEED
extern void NDECL(rider_cant_reach);
extern boolean FDECL(can_saddle, (struct monst *));
extern int FDECL(use_saddle, (struct obj *));
extern boolean FDECL(can_ride, (struct monst *));
extern int NDECL(doride);
extern boolean FDECL(mount_steed, (struct monst *, boolean));
extern void NDECL(exercise_steed);
extern void NDECL(kick_steed);
extern void FDECL(dismount_steed, (int));
extern void FDECL(place_monster, (struct monst *,int,int));
#endif

/* ### teleport.c ### */

extern boolean FDECL(goodpos, (int,int,struct monst *,unsigned));
extern boolean FDECL(enexto, (coord *,signed char,signed char,struct permonst *));
extern boolean FDECL(enexto_core, (coord *,signed char,signed char,struct permonst *,unsigned));
extern void FDECL(teleds, (int,int,boolean));
extern boolean FDECL(safe_teleds, (boolean));
extern boolean FDECL(teleport_pet, (struct monst *,boolean));
extern void NDECL(tele);
extern int NDECL(dotele);
extern void NDECL(level_tele);
extern void FDECL(domagicportal, (struct trap *));
extern void FDECL(tele_trap, (struct trap *));
extern void FDECL(level_tele_trap, (struct trap *));
extern void FDECL(rloc_to, (struct monst *,int,int));
extern boolean FDECL(rloc, (struct monst *, boolean));
extern boolean FDECL(tele_restrict, (struct monst *));
extern void FDECL(mtele_trap, (struct monst *, struct trap *,int));
extern int FDECL(mlevel_tele_trap, (struct monst *, struct trap *,boolean,int));
extern void FDECL(rloco, (struct obj *));
extern int NDECL(random_teleport_level);
extern boolean FDECL(u_teleport_mon, (struct monst *,boolean));

/* ### tile.c ### */
#ifdef USE_TILES
extern void FDECL(substitute_tiles, (d_level *));
#endif

/* ### timeout.c ### */

extern void NDECL(burn_away_slime);
extern void NDECL(nh_timeout);
extern void FDECL(fall_asleep, (int, boolean));
extern void FDECL(attach_egg_hatch_timeout, (struct obj *));
extern void FDECL(attach_fig_transform_timeout, (struct obj *));
extern void FDECL(kill_egg, (struct obj *));
extern void FDECL(hatch_egg, (void *, long));
extern void FDECL(learn_egg_type, (int));
extern void FDECL(burn_object, (void *, long));
extern void FDECL(begin_burn, (struct obj *, boolean));
extern void FDECL(end_burn, (struct obj *, boolean));
extern void NDECL(do_storms);
extern boolean FDECL(start_timer, (long, short, short, void *));
extern long FDECL(stop_timer, (short, void *));
extern void NDECL(run_timers);
extern void FDECL(obj_move_timers, (struct obj *, struct obj *));
extern void FDECL(obj_split_timers, (struct obj *, struct obj *));
extern void FDECL(obj_stop_timers, (struct obj *));
extern boolean FDECL(obj_is_local, (struct obj *));
extern void FDECL(save_timers, (int,int,int));
extern void FDECL(restore_timers, (int,int,boolean,long));
extern void FDECL(relink_timers, (boolean));
#ifdef WIZARD
extern int NDECL(wiz_timeout_queue);
extern void NDECL(timer_sanity_check);
#endif

/* ### topten.c ### */

extern void FDECL(topten, (int));
extern void FDECL(prscore, (int,char **));
extern struct obj *FDECL(tt_oname, (struct obj *));

/* ### track.c ### */

extern void NDECL(initrack);
extern void NDECL(settrack);
extern coord *FDECL(gettrack, (int,int));

/* ### trap.c ### */

extern boolean FDECL(burnarmor,(struct monst *));
extern boolean FDECL(rust_dmg, (struct obj *,const char *,int,boolean,struct monst *));
extern void FDECL(grease_protect, (struct obj *,const char *,struct monst *));
extern struct trap *FDECL(maketrap, (int,int,int));
extern void FDECL(fall_through, (boolean));
extern struct monst *FDECL(animate_statue, (struct obj *,signed char,signed char,int,int *));
extern struct monst *FDECL(activate_statue_trap, (struct trap *,signed char,signed char,boolean));
extern void FDECL(dotrap, (struct trap *, unsigned));
extern void FDECL(seetrap, (struct trap *));
extern int FDECL(mintrap, (struct monst *));
extern void FDECL(instapetrify, (const char *));
extern void FDECL(minstapetrify, (struct monst *,boolean));
extern void FDECL(selftouch, (const char *));
extern void FDECL(mselftouch, (struct monst *,const char *,boolean));
extern void NDECL(float_up);
extern void FDECL(fill_pit, (int,int));
extern int FDECL(float_down, (long, long));
extern int FDECL(fire_damage, (struct obj *,boolean,boolean,signed char,signed char));
extern void FDECL(water_damage, (struct obj *,boolean,boolean));
extern boolean NDECL(drown);
extern void FDECL(drain_en, (int));
extern int NDECL(dountrap);
extern int FDECL(untrap, (boolean));
extern boolean FDECL(chest_trap, (struct obj *,int,boolean));
extern void FDECL(deltrap, (struct trap *));
extern boolean FDECL(delfloortrap, (struct trap *));
extern struct trap *FDECL(t_at, (int,int));
extern void FDECL(b_trapped, (const char *,int));
extern boolean NDECL(unconscious);
extern boolean NDECL(lava_effects);
extern void FDECL(blow_up_landmine, (struct trap *));
extern int FDECL(launch_obj,(short,int,int,int,int,int));

/* ### u_init.c ### */

extern void NDECL(u_init);

/* ### uhitm.c ### */

extern void FDECL(hurtmarmor,(struct monst *,int));
extern boolean FDECL(attack_checks, (struct monst *,struct obj *));
extern void FDECL(check_caitiff, (struct monst *));
extern signed char FDECL(find_roll_to_hit, (struct monst *));
extern boolean FDECL(attack, (struct monst *));
extern boolean FDECL(hmon, (struct monst *,struct obj *,int));
extern int FDECL(damageum, (struct monst *,struct attack *));
extern void FDECL(missum, (struct monst *,struct attack *));
extern int FDECL(passive, (struct monst *,boolean,int,unsigned char));
extern void FDECL(passive_obj, (struct monst *,struct obj *,struct attack *));
extern void FDECL(stumble_onto_mimic, (struct monst *));
extern int FDECL(flash_hits_mon, (struct monst *,struct obj *));

/* ### unixmain.c ### */

#ifdef UNIX
# ifdef PORT_HELP
extern void NDECL(port_help);
# endif
#endif /* UNIX */


/* ### unixtty.c ### */

#if defined(UNIX) || defined(__BEOS__)
extern void NDECL(gettty);
extern void FDECL(settty, (const char *));
extern void NDECL(setftty);
extern void NDECL(intron);
extern void NDECL(introff);
extern void VDECL(error, (const char *,...)) PRINTF_F(1,2);
#endif /* UNIX || __BEOS__ */

/* ### unixunix.c ### */

#ifdef UNIX
extern void NDECL(getlock);
extern void FDECL(regularize, (char *));
# if defined(TIMED_DELAY) && !defined(msleep)
extern void FDECL(msleep, (unsigned));
# endif
# ifdef SHELL
extern int NDECL(dosh);
# endif /* SHELL */
# if defined(SHELL) || defined(DEF_PAGER) || defined(DEF_MAILREADER)
extern int FDECL(child, (int));
# endif
#endif /* UNIX */

/* ### unixres.c ### */

#ifdef UNIX
# ifdef GNOME_GRAPHICS 
extern int FDECL(hide_privileges, (boolean));
# endif
#endif /* UNIX */

/* ### vault.c ### */

extern boolean FDECL(grddead, (struct monst *));
extern char FDECL(vault_occupied, (char *));
extern void NDECL(invault);
extern int FDECL(gd_move, (struct monst *));
extern void NDECL(paygd);
extern long NDECL(hidden_gold);
extern boolean NDECL(gd_sound);

/* ### version.c ### */

extern char *FDECL(version_string, (char *));
extern char *FDECL(getversionstring, (char *));
extern int NDECL(doversion);
extern int NDECL(doextversion);
extern boolean FDECL(check_version, (struct version_info *, const char *,boolean));
extern unsigned long FDECL(get_feature_notice_ver, (char *));
extern unsigned long NDECL(get_current_feature_ver);
#ifdef RUNTIME_PORT_ID
extern void FDECL(append_port_id, (char *));
#endif

/* ### video.c ### */

#ifdef VIDEOSHADES
extern int FDECL(assign_videoshades, (char *));
extern int FDECL(assign_videocolors, (char *));
#endif

/* ### vis_tab.c ### */

#ifdef VISION_TABLES
extern void NDECL(vis_tab_init);
#endif

/* ### vision.c ### */

extern void NDECL(vision_init);
extern int FDECL(does_block, (int,int,struct rm*));
extern void NDECL(vision_reset);
extern void FDECL(vision_recalc, (int));
extern void FDECL(block_point, (int,int));
extern void FDECL(unblock_point, (int,int));
extern boolean FDECL(clear_path, (int,int,int,int));
extern void FDECL(do_clear_area, (int,int,int,
			     void (*)(int,int,void *),void *));

/* ### weapon.c ### */

extern int FDECL(hitval, (struct obj *,struct monst *));
extern int FDECL(dmgval, (struct obj *,struct monst *));
extern struct obj *FDECL(select_rwep, (struct monst *));
extern struct obj *FDECL(select_hwep, (struct monst *));
extern void FDECL(possibly_unwield, (struct monst *,boolean));
extern int FDECL(mon_wield_item, (struct monst *));
extern int NDECL(abon);
extern int NDECL(dbon);
extern int NDECL(enhance_weapon_skill);
#ifdef DUMP_LOG
extern void NDECL(dump_weapon_skill);
#endif
extern void FDECL(unrestrict_weapon_skill, (int));
extern void FDECL(use_skill, (int,int));
extern void FDECL(add_weapon_skill, (int));
extern void FDECL(lose_weapon_skill, (int));
extern int FDECL(weapon_type, (struct obj *));
extern int NDECL(uwep_skill_type);
extern int FDECL(weapon_hit_bonus, (struct obj *));
extern int FDECL(weapon_dam_bonus, (struct obj *));
extern void FDECL(skill_init, (const struct def_skill *));

/* ### were.c ### */

extern void FDECL(were_change, (struct monst *));
extern void FDECL(new_were, (struct monst *));
extern int FDECL(were_summon, (struct permonst *,boolean,int *,char *));
extern void NDECL(you_were);
extern void FDECL(you_unwere, (boolean));

/* ### wield.c ### */

extern void FDECL(setuwep, (struct obj *));
extern void FDECL(setuqwep, (struct obj *));
extern void FDECL(setuswapwep, (struct obj *));
extern int NDECL(dowield);
extern int NDECL(doswapweapon);
extern int NDECL(dowieldquiver);
extern boolean FDECL(wield_tool, (struct obj *,const char *));
extern int NDECL(can_twoweapon);
extern void NDECL(drop_uswapwep);
extern int NDECL(dotwoweapon);
extern void NDECL(uwepgone);
extern void NDECL(uswapwepgone);
extern void NDECL(uqwepgone);
extern void NDECL(untwoweapon);
extern void FDECL(erode_obj, (struct obj *,boolean,boolean));
extern int FDECL(chwepon, (struct obj *,int));
extern int FDECL(welded, (struct obj *));
extern void FDECL(weldmsg, (struct obj *));
extern void FDECL(setmnotwielded, (struct monst *,struct obj *));

/* ### windows.c ### */

extern void FDECL(choose_windows, (const char *));
extern char FDECL(genl_message_menu, (char,int,const char *));
extern void FDECL(genl_preference_update, (const char *));

/* ### wizard.c ### */

extern void NDECL(amulet);
extern int FDECL(mon_has_amulet, (struct monst *));
extern int FDECL(mon_has_special, (struct monst *));
extern int FDECL(tactics, (struct monst *));
extern void NDECL(aggravate);
extern void NDECL(clonewiz);
extern int NDECL(pick_nasty);
extern int FDECL(nasty, (struct monst*));
extern void NDECL(resurrect);
extern void NDECL(intervene);
extern void NDECL(wizdead);
extern void FDECL(cuss, (struct monst *));

/* ### worm.c ### */

extern int NDECL(get_wormno);
extern void FDECL(initworm, (struct monst *,int));
extern void FDECL(worm_move, (struct monst *));
extern void FDECL(worm_nomove, (struct monst *));
extern void FDECL(wormgone, (struct monst *));
extern void FDECL(wormhitu, (struct monst *));
extern void FDECL(cutworm, (struct monst *,signed char,signed char,struct obj *));
extern void FDECL(see_wsegs, (struct monst *));
extern void FDECL(detect_wsegs, (struct monst *,boolean));
extern void FDECL(save_worm, (int,int));
extern void FDECL(rest_worm, (int));
extern void FDECL(place_wsegs, (struct monst *));
extern void FDECL(remove_worm, (struct monst *));
extern void FDECL(place_worm_tail_randomly, (struct monst *,signed char,signed char));
extern int FDECL(count_wsegs, (struct monst *));
extern boolean FDECL(worm_known, (struct monst *));

/* ### worn.c ### */

extern void FDECL(setworn, (struct obj *,long));
extern void FDECL(setnotworn, (struct obj *));
extern void FDECL(mon_set_minvis, (struct monst *));
extern void FDECL(mon_adjust_speed, (struct monst *,int,struct obj *));
extern void FDECL(update_mon_intrinsics,
		(struct monst *,struct obj *,boolean,boolean));
extern int FDECL(find_mac, (struct monst *));
extern void FDECL(m_dowear, (struct monst *,boolean));
extern struct obj *FDECL(which_armor, (struct monst *,long));
extern void FDECL(mon_break_armor, (struct monst *,boolean));
extern void FDECL(bypass_obj, (struct obj *));
extern void NDECL(clear_bypasses);
extern int FDECL(racial_exception, (struct monst *, struct obj *));

/* ### write.c ### */

extern int FDECL(dowrite, (struct obj *));

/* ### zap.c ### */

extern int FDECL(bhitm, (struct monst *,struct obj *));
extern void FDECL(probe_monster, (struct monst *));
extern boolean FDECL(get_obj_location, (struct obj *,signed char *,signed char *,int));
extern boolean FDECL(get_mon_location, (struct monst *,signed char *,signed char *,int));
extern struct monst *FDECL(get_container_location, (struct obj *obj, int *, int *));
extern struct monst *FDECL(montraits, (struct obj *,coord *));
extern struct monst *FDECL(revive, (struct obj *));
extern int FDECL(unturn_dead, (struct monst *));
extern void FDECL(cancel_item, (struct obj *));
extern boolean FDECL(drain_item, (struct obj *));
extern struct obj *FDECL(poly_obj, (struct obj *, int));
extern boolean FDECL(obj_resists, (struct obj *,int,int));
extern boolean FDECL(obj_shudders, (struct obj *));
extern void FDECL(do_osshock, (struct obj *));
extern int FDECL(bhito, (struct obj *,struct obj *));
extern int FDECL(bhitpile, (struct obj *,int (*)(OBJ_P,OBJ_P),int,int));
extern int FDECL(zappable, (struct obj *));
extern void FDECL(zapnodir, (struct obj *));
extern int NDECL(dozap);
extern int FDECL(zapyourself, (struct obj *,boolean));
extern boolean FDECL(cancel_monst, (struct monst *,struct obj *,
			       boolean,boolean,boolean));
extern void FDECL(weffects, (struct obj *));
extern int NDECL(spell_damage_bonus);
extern const char *FDECL(exclam, (int force));
extern void FDECL(hit, (const char *,struct monst *,const char *));
extern void FDECL(miss, (const char *,struct monst *));
extern struct monst *FDECL(bhit, (int,int,int,int,int (*)(MONST_P,OBJ_P),
			     int (*)(OBJ_P,OBJ_P),struct obj *));
extern struct monst *FDECL(boomhit, (int,int));
extern int FDECL(burn_floor_paper, (int,int,boolean,boolean));
extern void FDECL(buzz, (int,int,signed char,signed char,int,int));
extern void FDECL(melt_ice, (signed char,signed char));
extern int FDECL(zap_over_floor, (signed char,signed char,int,boolean *));
extern void FDECL(fracture_rock, (struct obj *));
extern boolean FDECL(break_statue, (struct obj *));
extern void FDECL(destroy_item, (int,int));
extern int FDECL(destroy_mitem, (struct monst *,int,int));
extern int FDECL(resist, (struct monst *,char,int,int));
extern void NDECL(makewish);

#endif /* !MAKEDEFS_C && !LEV_LEX_C */

#endif /* EXTERN_H */
