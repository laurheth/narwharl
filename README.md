# NarwhaRL - Nice And Relaxing or Worst Hell Always? the RogueLike

## Background

I made this in 2010 as a project to learn C++. It is a roguelike, rendered in ascii, in which you delve into a deep dungeon and fight monsters. I dug it up recently and intend to update it for the modern era of 2018 (with somewhat more experienced eyes).

## Installing

Currently only tested in OS X. Depends on ncurses. It _should_ be straightforward, but Makefiles probably need tinkering.

## Playing

Controls right now:

* arrow keys, hjklyubn : movement/melee attack
* i : look at inventory
* l : look at map
* t : throw
* q : quaff
* w : wield/unwield
* W : wear/unwear
* P : put on/take off (jewellry. Not properly implemented yet)
* g : Pick up
* d : drop
* m : look at skills
* S : save and quit
* Q : don't save and quit

Saves are stored in 'save'. It's all in plain text so feel free to do hilarious things like poison all of your enemies.
Definition files are stored in 'defs'. Also all in plain text and easy to alter. Go wild! Make as much absurd stuff as you want and send them to me if you think they're cool. Maybe they'll get added to the final game?

Copyright (c) 2010-2018 Lauren Hetherington

Distributed under the GPL 2.0. mtrand is distributed under some other compatible license, details of which can be found in the mtrand directory of this package.
