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
Playing nethack can be a lot of fun, if you know what you're doing.
But this is one of the most frustrating games to try to learn.

The goal of this fork is to make the game more approachable for new players.

 * The UI needs an overhaul.
   I am aware of the tile-based interface available for the standard game, but it sucks.
   I intend to make a more modern GUI interface to this game, starting with something like Castle of the Winds.
   All the old keyboard shortcuts should still work though (possibly with some reassignment).
   * The retro "Rogue" level has got to go.
     Sorry, nostalgia.
 * You shouldn't need the wiki.
   The game should feature an in-game encyclopedia that's unlocked as you play, and it should persist across games.
   For instance, once your brain is sucked by a master mindflayer, that ability will appear in the master mindflayer article in the encyclopedia.
   Right-clicking on a master mindflayer in-game should provide a link to the article, etc.
 * Don't touch the balance too much.
   No new items, monsters, roles, etc.
   The only changes would be related to conveying information,
   such as making the non-cursed fortune cookies more helpful.

Additional fun stuff maybe:

 * Challenge modes, inspired by [The Binding of Isaac: Rebirth](http://bindingofisaacrebirth.gamepedia.com/Challenges).
   For example, you are blind the whole game, or you are hallucinating the whole game.
 * A game mode that allows save states, called "Practice Mode".
   The normal game should be called "Hardcore Mode".

And to make this project more approachable for developers:

 * Drop support for all the crazy build configurations that no one wants, like BeOS and Win32.
 * Remove all compile-time options, like tourist and kops.
   That should all be enabled all the time.
 * Use modern tools where appropriate.
   I'm not afraid to port the level compiler to python, for example.
 * Use Git and Github
 * Clean up C code (and Makefiles).
   Don't use Pascal-style argument declarations.
   Use clear identifier names (e.g. replace `DGNCOMPSRC` with `DUNGEON_COMPILER_SOURCE`).
   Etc.
 * Abandon all hope of cleanly merging changes to the standard game into this fork.
