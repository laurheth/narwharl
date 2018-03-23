/*
NarwhaRL - Nice And Relaxing or Worst Hell Always? the RogueLike
Copyright (C) 2010-2018 Lauren Hetherington

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.

 */


#include <iostream>
//#include <ncurses/cursesapp.h>
#include "nheader.h"

using namespace std;

int seed;

Stuffgetter * objget = new Stuffgetter();
string knowledge="";

int main(int argc, char * argv[])
{

  // Initial coordinates
  int startx, starty;
  // desired coordinates
  int wantx, wanty;

  // key press
  int ch;

  // get seed and initialize
  if (argc != 2) {
    seed = 0;
  }
  else {
    seed = atoi(argv[1]);
  }
  mtinit(&seed);
  cout << seed << endl;

  // ncurses stuff
  initscr(); //initialize screen
  cbreak(); //line buffering disabled
  noecho(); //disable echoing characters to the window
  keypad(stdscr, TRUE); //get keypad stuff

  // check screen size
  if (LINES < 24 || COLS < 80) {

    endwin();
    cout << "Screen size " << COLS << " x " << LINES << " is too small.\n";
    cout << "Must be minimum of 80 x 24.";
    return 0;

  }


  curs_set(0);
  // Define colours;
  start_color();

  init_pair(1, COLOR_WHITE, COLOR_BLACK);
  init_pair(2, COLOR_YELLOW, COLOR_BLACK);
  init_pair(3, COLOR_RED, COLOR_BLACK);
  init_pair(4, COLOR_GREEN, COLOR_BLACK);
  init_pair(5, COLOR_BLUE, COLOR_BLACK);
  init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
  init_pair(7, COLOR_CYAN, COLOR_BLACK);
  init_pair(8, COLOR_WHITE, COLOR_RED);
  init_pair(9, COLOR_YELLOW, COLOR_RED);
  init_pair(10, COLOR_RED, COLOR_RED);
  init_pair(11, COLOR_GREEN, COLOR_RED);
  init_pair(12, COLOR_BLUE, COLOR_RED);
  init_pair(13, COLOR_MAGENTA, COLOR_RED);
  init_pair(14, COLOR_CYAN, COLOR_RED);


  // Make the map
  Map map(seed);

  //map.allocatemap(20,80);

  // linked list for creatures.
  llist *mmlist = NULL;

  // linked list of monster defs.
  llist *mondefs;
  int nummons;
  nummons = getlist(&mondefs,"monsters.txt");

  int mapsuccess;
  starty=-10;
  startx=-10;
  mapsuccess = map.loadmap(&mmlist);
  if (mapsuccess == 0) {
    map.newgenmap(&mmlist, &starty, &startx, NULL);
    Player * player;
    player = new Player(&map, starty, startx, '@', "Player");
    list_add(&mmlist, (void *) player);
  }

  // pointer to the start of the list.
  llist *mlist = mmlist;

  // Get FOV
  int viewdist;
  //void (*fovptr)(Map *, int, int);
  //fovptr = &see;
  viewdist = 6;

  // build screen layout

  Screen vscreen;

  // draw map for first time
  refresh();

  vscreen.showlevel(&map, starty, startx);

  refresh();
  int stillgoing;
  stillgoing=1;
  map.mlist = &mmlist;

  while (map.loop(&vscreen) == 1) {
    //NOTHING!!!!
  }

  curs_set(1);
  endwin();
  return 0;

}
