# nethack

A fork of [nethack](http://www.nethack.org/v343/download-src.html).

## Status

Not cool yet.

## How do I shot game?

### Dependencies (Ubuntu)

```
sudo apt-get install libncurses5-dev bison flex
```

### Build

```
make
```

### Run

```
./nethack
```

## Why are you doing this?

Why make another fork of nethack?
Playing nethack can be a lot of fun once you know what you're doing.
But nethack is one of the most frustrating games to try to learn.

The goal of this fork is to make the game more approachable for new players.

 * Browser-based graphical interface.
   * Server-side game logic to allow spectating and prevent cheating.
   * Try to preserve all the keyboard shortcuts, but have a more obvious way to do most things as well.
   * (I am aware of the tile-based interface built into the standard game. It sucks.)
 * You shouldn't need the wiki.
   The game should feature an in-game encyclopedia that's unlocked as you play, and it should persist across games.
   For instance, once your brain is sucked by a master mindflayer, that ability will appear in the master mindflayer article in the encyclopedia.
   Right-clicking on a master mindflayer in-game should provide a link to the article, etc.
 * Don't touch the balance too much.
   Keep all the same items, monsters, roles, etc.
   Try to preserve the idiosyncrasies in the game engine,
   like the [dual slow-digestion exploit](http://nethackwiki.com/wiki/Foodless#Dual_slow_digestion).
 * Improve the information conveyance.
   * Non-cursed fortune cookies should be more helpful, and should fill in the encyclopedia.
     Cursed fortune cookies should just be unhelpful instead of outright lies.
   * Priest donations should persist and indicate progress toward the next divine protection.
   * The Oracle should be more helpful.
     * At all times, the oracle should direct you toward your next goal for free,
       such as telling you that you should do the quest next.
     * The Major Consolations should tell you something important and relevant,
       such as the weaknesses of your quest nemesis.
       Major Consolations should be added to your encyclopedia, and should never repeat.
   * Make the "You hear" and "You feel" messages more direct, possibly after getting them enough times.
   * The retro "Rogue" level has got to go.
 * A game mode that allows save states, called "Practice Mode".
   The normal game should be called "Hardcore Mode".
   (Get rid of the current Explore and Wizard modes.)
   * You can still unlock encyclopedia entries in Practice Mode.

Additional fun stuff maybe someday:

 * Challenge modes, inspired by [The Binding of Isaac: Rebirth](http://bindingofisaacrebirth.gamepedia.com/Challenges).
   * You are hallucinating the whole game.
   * You can never wear anything.
   * You only have one hand.
   * Etc.
 * Achievements
 * [TAS](http://en.wikipedia.org/wiki/Tool-assisted_speedrun) support
 * (The web api for the client would already be appropriate for AI players.)

And to make this project more approachable for developers:

 * Drop support for all the crazy build configurations that no one wants, like BeOS and Win32.
 * Remove all compile-time options, like tourist and kops.
   That should all be enabled all the time.
 * Use modern tools where appropriate.
   I'm not afraid to port the level compiler to python, for example.
 * Use Git and Github.
 * Clean up the code: no Pascal-style argument declarations,
   no hard tabs, use clear identifier names, etc.
 * Abandon all hope of cleanly merging changes to the standard game into this fork.
