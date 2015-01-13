# nethack

A fork of [nethack 3.4.3](http://www.nethack.org/v343/download-src.html).

## Status

Not cool yet.

## How do I shot game?

### Dependencies (Ubuntu)

```
sudo apt-get install bison flex clang
```

And [install node.js](https://github.com/joyent/node/wiki/installing-node.js-via-package-manager).

### Build

```
make
(cd server && npm run build)
```

### Run

```
(cd server && node lib/server.js)
```

Then open the printed link in a web browser.

## Why are you doing this?

Why make another fork of nethack?
Playing nethack can be a lot of fun once you know what you're doing.
But nethack is one of the most frustrating games to try to learn.

The goal of this fork is to make the game more approachable for new players.

 * Browser-based graphical interface.
   * Server-side game logic to allow spectating and prevent cheating.
   * Have keyboard shortcuts for everything, preferably preserving the commands from the original game.
   * (I am aware of the tile-based interface built into the standard game. It sucks.)
 * You shouldn't need the wiki.
   The game should feature an in-game encyclopedia that's unlocked as you play, and it should persist across games.
   For instance, once your brain is sucked by a master mindflayer, that ability will appear in the master mindflayer article in the encyclopedia.
   Right-clicking on a master mindflayer in-game should provide a link to the article, etc.
 * Don't touch the balance too much.
   This project is not to create a game, but to improve the user experience for an existing game.
   * Keep all the same items, monsters, roles, etc.
   * Preserve many of the idiosyncrasies in the game engine,
     like the [dual slow-digestion exploit](http://nethackwiki.com/wiki/Foodless#Dual_slow_digestion).
   * Fix crashes, such as [when you apply a wielded cream pie](http://tasvideos.org/GameResources/DOS/Nethack.html#CreamPieGlitch).
   * Fix quirks that don't make sense, such as [C343-10](http://nethackwiki.com/wiki/Bugs_in_NetHack_3.4.3#C343-10):
     If there is a corpse in a pit, you have to enter the pit to pick it up but not to eat it.
   * Real-world time shouldn't affect the game.
     Move the all the nighttime/moon-phase mechanics to correspond to in-game time.
     In-game time should be randomly initialized for new games, and flow with the turn counter.
     * You can ask the Oracle for the in-game time for free, and then you know it forever afterward (intrinsic chronoscience).
   * Blacklist all special names.
     This includes the names of unique monsters, artifact names, and anything containing "elbereth" (ignoring case).
     These names would be forbidden for usernames, pet names, custom fruits, and the Call command.
     * Fixes the [artifact naming exploit](http://nethackwiki.com/wiki/Naming_artifacts).
     * Claiming that you're Creosus is more fair to users on a server.
     * No need for special logic for Elbereth on tombstones.
 * Improve the information conveyance.
   * Uncursed fortune cookies should unlock a random tidbit from the encyclopedia.
     Blessed fortune cookies should do the same, but select from a set of relevant tidbits,
     such as a possible use for a stethoscope if you're carrying a stethoscope.
     Cursed fortune cookies should just be vague and unhelpful instead of outright lies.
     * Monks should start with only cursed fortune cookies,
       because repeatedly starting a Monk character to slowly unlock the encyclopedia would be dumb.
       (The BUC status of fortune cookies only matters for the message, not the nutrition.)
       You should have to work a little to uncurse or bless the fortune cookies if you want good messages.
   * If you are carrying a stethoscope that you can use,
     display the information for using it on yourself in the status bar at all times.
     This eliminates the tedium of having to apply it to yourself for free at every step.
     * Blessed stethoscopes should provide more information, including exercise scores.
       While carrying a blessed stethoscope, you will learn the actions that exercise and abuse attributes
       and add that information to the encyclopedia as you go.
   * Priest donations should persist and indicate progress toward the next divine protection.
   * The Oracle should be more helpful.
     * At all times, the oracle should direct you toward your next goal for free,
       such as telling you that you should do the quest next.
     * The Major Consultations should tell you something important and relevant,
       such as the weaknesses of your quest nemesis.
       Major Consultations should be added to your encyclopedia, and should never repeat.
   * Once you identify a unique-appearance item, such as an apron always being an alchemy smock,
     add that to the encyclopedia, and have that translation resolved for all future games.
   * After getting a "You hear" or "You feel" message and experiencing the consequences,
     asscociate the two in the encyclopedia, and add an explanation when future games get that message.
     For example:
     * "You hear the chime of a cash register", then "You hear someone counting money", then you find a shop.
       You learn that the former message meant there was a store, and the latter message is still a mystery.
     * "You feel full of hot air", then you resist a cold attack;
       you learn that the message means you got cold resistance.
       Display the fact that you have cold resistance at all times;
       if you lose cold resistance for any reason, explain it immediately, and update the display.
   * The retro "Rogue" level has got to go.
   * Provide a high-level map of the dungeon branches you've discovered so far,
     and a detailed map of each floor you've been to.
 * A game mode that allows save states, called "Practice Mode".
   The normal game should be called "Hardcore Mode".
   * You can still unlock encyclopedia entries in Practice Mode.
   * Rename Wizard Mode to Debug Mode, have it only accessible by running the game locally.
   * Get rid of the standard Explore Mode.

Additional fun stuff maybe someday:

 * Challenge modes, inspired by [The Binding of Isaac: Rebirth](http://bindingofisaacrebirth.gamepedia.com/Challenges).
   * You are hallucinating the whole game.
   * You can never wear anything.
   * You only have one hand.
   * Etc.
 * Achievements
 * Seed input, and other [TAS](http://en.wikipedia.org/wiki/Tool-assisted_speedrun) support features
 * (The web api for the client would already be appropriate for AI projects.)

And to make this project more approachable for developers:

 * No support for all the crazy build configurations that no one wants, like BeOS and Win32.
 * No compile-time options. Tourists, Kops, Steeds, etc. are all enabled all the time now.
 * Git and [GitHub](https://github.com/thejoshwolfe/nethack).
 * Major code clean up.
   * Aesthetic changes: no Pascal-style argument declarations, no hard tabs, clearer identifier names.
   * Don't use the preprocessor for programming: no `#undef`, no `#include`ing yourself, no macro functions.
   * Refactoring: minimize/eliminate global variables. Turn the game into a library with a clean interface.
   * String manipulation: NUL-terminated `char*`s are awful. Write our own string library if we have to.

Needless to say, we're abandoning all hope of cleanly merging changes for the standard game into this fork.
