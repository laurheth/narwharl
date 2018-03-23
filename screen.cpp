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
#include "nheader.h"
#include <cstring>
#include <sstream>
#include <cmath>

#ifndef WIN
#include <unistd.h>
#define SLEEP(a) usleep(a*1000)
#else
#include <windows.h>
#define SLEEP(a) Sleep(a)
#endif

using namespace std;

Screen::Screen()
{
  // Window dimensions.
  levelwiny=LINES-4;
  levelwinx=COLS-22;

  //if (levelwiny % 2 == 1) {levelwiny--;}
  //if (levelwinx % 2 == 1) {levelwinx--;}

  //messagewin = newwin(2,80,0,0);
  //levelwin = newwin(20,80,2,0);
  //statswin = newwin(2,80,22,0);
  
  messagewin = newwin(4,COLS,levelwiny,0);
  levelwin = newwin(levelwiny,levelwinx,0,22);
  statswin = newwin(levelwiny,22,0,0);

  // Used for drawing.
  uleftx = 0;
  ulefty = 0;
  message1=" ";
  message2=" ";
  message3=" ";
  nummsg=0;
}

Screen::~Screen()
{
  delwin(messagewin);
  delwin(levelwin);
  delwin(statswin);
  //  delete messagewin;
  //delete levelwin;
  //delete statswin;
}

void Screen::hidelevel(Map *map)
{
  int i,j;
  int maxy=map->ysize;
  int maxx=map->xsize;

  for (i=0;i<maxx;i++) {
    for (j=0;j<maxy;j++) {
      if (map->cansee(j,i)) { map->unsee(j,i);}
      if (i<levelwinx && j<levelwiny) {mvwaddch(levelwin,j,i,' ');}
    }
  }
}

int Screen::getachar(Map * map, int y, int x)
{
  nodelay(stdscr,1);
  //cbreak();
  //keypad(stdscr,TRUE);
  int button=ERR;
  while (button == ERR) {
    button=wgetch(stdscr);
    if (button == ERR) {
      SLEEP(40);
      this->showlevel(map,y,x);
    }
  }

  nodelay(stdscr,0);
  return button;
}

void Screen::showlevel(Map *map, int y, int x)
{
  int i,j;
  int maxy=map->ysize;
  int maxx=map->xsize;
  int lx,hx,ly,hy;
  hx = x + levelwinx/2;
  hy = y + levelwiny/2;
  if (hx > maxx) {hx=maxx;}
  if (hy > maxy) {hy=maxy;}
  lx = hx-levelwinx;
  ly = hy-levelwiny;
  if (lx < 0) {lx=0; hx=lx+levelwinx;}
  if (ly < 0) {ly=0; hy=ly+levelwiny;}
  
  for (i=0;i<levelwinx;i++) {
    for (j=0;j<levelwiny;j++) {
      //if (map->inside(j+ly,i+lx)==1) {
      if (map->seen(j+ly,i+lx)==1) {
	mvwaddch(levelwin,j,i,map->getsymb(j+ly,i+lx));
      }
    }
  }

  // Store lx and ly for use elsewhere for drawing temporary visual effects.
  uleftx = lx;
  ulefty = ly;

  wrefresh(levelwin);
}

void Screen::drawatpos(int y, int x, char symb, char color)
{
  if (color > 0) {
    drawatpos(y,x,symb | COLOR_PAIR(color));
  }
  else {
    drawatpos(y,x,symb | COLOR_PAIR(-color) | A_BOLD);
  }
}

void Screen::drawatpos(int y, int x, int symb)
{
  int i,j;
  j=y-ulefty;
  i=x-uleftx;
  if ((j >= 0 && j<levelwiny) && ((i >= 0) && i<levelwinx)) {
    mvwaddch(levelwin,j,i,symb);
  }
  wrefresh(levelwin);
}

void Screen::drawoutwards(int y, int x, int range, char symb, char color, Map * map)
{
  int distout=0;
  int angle=0;
  int effrange=range+2;
  int i,j;
  int havedrawn=0;
  for (distout=0; distout<effrange; distout++) {
    havedrawn=0;
    for (angle=0; angle<6*effrange; angle++) {
      i=x+(int)(((float)distout)*cos(2*PI*((float)angle)/((float)effrange*6))+0.5);
      j=y+(int)(((float)distout)*sin(2*PI*((float)angle)/((float)effrange*6))+0.5);
      if ((i<0) || (j<0) || (i > map->xsize-1 || j > map->ysize-1)) {i=x;j=y;}
      if (map->iseffected(j,i) != 0) {
	map->seteffected(j,i,0);
	if (map->cansee(j,i) != 0) {
	  this->drawatpos(j,i,symb,color);
	  havedrawn=1;
	}
      }
    }
    if (havedrawn>0) {
      //  usleep(20000);
      SLEEP(20);
    }
    for (angle=0; angle<6*effrange; angle++) {
      i=x+(int)(((float)distout)*cos(2*PI*((float)angle)/((float)effrange*6))+0.5);
      j=y+(int)(((float)distout)*sin(2*PI*((float)angle)/((float)effrange*6))+0.5);
      if ((i<0) || (j<0) || (i > map->xsize-1 || j > map->ysize-1)) {i=x;j=y;}
      if (map->cansee(j,i) != 0) {
	this->drawatpos(j,i,map->getsymb(j,i));
	havedrawn=1;
      }
    }

  }
}

void Screen::movecursor(Map *map, int *y, int *x)
{
  int stop=0;
  int ch;

  int maxy=map->ysize;
  int maxx=map->xsize;
  int lx,hx,ly,hy;

  hx = *x + levelwinx/2;
  hy = *y + levelwiny/2;
  if (hx > maxx) {hx=maxx;}
  if (hy > maxy) {hy=maxy;}
  lx = hx-levelwinx;
  ly = hy-levelwiny;
  if (lx < 0) {lx=0; hx=lx+levelwinx;}
  if (ly < 0) {ly=0; hy=ly+levelwiny;}

  //  curs_set(1);
  //  wmove(levelwin,*y-ly,*x-lx);
  this->drawatpos(*y,*x,'X', -2);
  wrefresh(levelwin);
  while (stop==0){
    ch = getch();
    this->drawatpos(*y,*x,map->getsymb(*y,*x));
    switch(ch)
      {
      case 'y':
	if (*x-lx > 0){*x = *x-1;}
	if (*y-ly > 0){*y = *y-1;}
	break;
      case 'u':
	if (*x-lx < levelwinx-1){*x = *x+1;}
	if (*y-ly > 0){*y = *y-1;}
	break;
      case 'b':
	if (*x-lx > 0){*x = *x-1;}
	if (*y-ly < levelwiny-1){*y = *y+1;}
	break;
      case 'n':
	if (*x-lx < levelwinx-1){*x = *x+1;}
	if (*y-ly < levelwiny-1){*y = *y+1;}
	break;
      case 'h':
      case KEY_LEFT:
	if (*x-lx > 0){*x = *x-1;}
	break;
      case 'l':
      case KEY_RIGHT:
	if (*x-lx < levelwinx-1){*x = *x+1;}
	break;
      case 'k':
      case KEY_UP:
	if (*y-ly > 0){*y = *y-1;}
	break;
      case 'j':
      case KEY_DOWN:
	if (*y-ly < levelwiny-1){*y = *y+1;}
	break;
      default:
	stop=1;
      }
    //wmove(levelwin,*y-ly,*x-lx);
    if (map->inside(*y,*x) == 1 & map->cansee(*y,*x) == 1) {
      this->drawatpos(*y,*x,'X', -2);
    }
    else {
      this->drawatpos(*y,*x,'X', -3);
    }
    this->message(map->look(*y,*x));
    wrefresh(levelwin);
  }
  this->drawatpos(*y,*x,map->getsymb(*y,*x));
  //  curs_set(0);
}

void Screen::stats(Creature * creature)
{
  stringstream s1;
  string line1;
  string line2;
  string temp;
  int i;
  /*  for (i=0;i<levelwinx;i++) {
    mvwaddch(statswin,0,i,' ');
    mvwaddch(statswin,1,i,' ');
    }*/

  temp="                      ";
  for (i=0;i<levelwiny;i++) {
    mvwprintw(statswin,i,0,temp.c_str());
  }

  temp = creature->getname();
  mvwprintw(statswin,1,0,temp.c_str());
  
  s1 << creature->hp << " " << creature->maxhp << " " <<creature->mp << " " << creature->maxmp << " " << creature->strength << " " << creature->dexterity << " " << creature->agility << " " << creature->toughness << " " << creature->level << " " << creature->experience << " " << (XPCURVE * (creature->level));
  //  s1 << creature->hp << " " << creature->maxhp;
  line1="HP: ";
  s1 >> temp;
  line1=line1 + temp + "/";
  s1 >> temp;
  line1=line1 + temp;
  mvwprintw(statswin,4,0,line1.c_str());

  //s1 << creature->mp << " " << creature->maxmp;
  line1="MP: ";
  s1 >> temp;
  line1=line1 + temp + "/";
  s1 >> temp;
  line1=line1 + temp;
  mvwprintw(statswin,5,0,line1.c_str());

  //  temp = "Stats: ";
  //mvwprintw(statswin,7,0,temp.c_str());
  line2 = "ST: ";
  s1 >> temp;
  mvwprintw(statswin,7,0,(line2+temp).c_str());
  line2 = "DX: ";
  s1 >> temp;
  mvwprintw(statswin,7,7,(line2+temp).c_str());
  line2 = "AG: ";
  s1 >> temp;
  mvwprintw(statswin,7,14,(line2+temp).c_str());
  line2 = "CN: ";
  s1 >> temp;
  mvwprintw(statswin,8,0,(line2+temp).c_str());
  line2 = "LVL: ";
  s1 >> temp;
  mvwprintw(statswin,2,0,(line2+temp).c_str());
  line2 = "XP: ";
  s1 >> temp;
  line2 += temp + "/";
  s1 >> temp;
  mvwprintw(statswin,2,8,(line2+temp).c_str());

  int attr1, attr2;
  attr1=A_NORMAL;
  attr2=A_NORMAL;
  temp = "Wielding: ";
  mvwprintw(statswin,10,0,temp.c_str());

  line2 = " ";
  Item * lhand=creature->lhand;
  Item * rhand=creature->rhand;
  if (creature->lhand == NULL && creature->rhand == NULL) {
    line1 = "Unarmed.";
  }
  else if (creature->lhand != NULL && creature->rhand == NULL) {
    line1 = (creature->lhand)->getname();
    if (lhand->hplost > 5) {attr1=A_BOLD | COLOR_PAIR(2);}
    if (lhand->hplost > 9) {attr1=A_BOLD | COLOR_PAIR(3);}
  }
  else if (creature->rhand != NULL && creature->lhand == NULL) {
    line1 = (creature->rhand)->getname();
    if (rhand->hplost > 5) {attr2=A_BOLD | COLOR_PAIR(2);}
    if (rhand->hplost > 9) {attr2=A_BOLD | COLOR_PAIR(3);}
  }
  else if (creature->lhand != NULL && creature->rhand != NULL) {
    line1 = (creature->lhand)->getname();
    line2 = (creature->rhand)->getname();
    if (lhand->hplost > 5) {attr1=A_BOLD | COLOR_PAIR(2);}
    if (rhand->hplost > 5) {attr2=A_BOLD | COLOR_PAIR(2);}
    if (lhand->hplost > 9) {attr1=A_BOLD | COLOR_PAIR(3);}
    if (rhand->hplost > 9) {attr2=A_BOLD | COLOR_PAIR(3);}
  }
  //  attr1=COLOR_PAIR(3);
  for (i=0;i<line1.length();i++) {
    mvwaddch(statswin,11,i,line1[i] | attr1);
  }
  for (i=0;i<line2.length();i++) {
    mvwaddch(statswin,12,i,line2[i] | attr2);
  }

  if (creature->load > 0) {
    line1="Encumbered";
    if (creature->load > 20) {attr1=A_BOLD | COLOR_PAIR(2);}
    if (creature->load > 40) {attr1=A_BOLD | COLOR_PAIR(3);}
    for (i=0;i<line1.length();i++) {
      mvwaddch(statswin,14,i,line1[i] | attr1);
    }

  }

  if (creature->poisoned > 0) {
    line1="Poisoned";
    if (creature->poisoned > 0) {attr1=A_BOLD | COLOR_PAIR(2);}
    if (creature->poisoned > 50) {attr1=A_BOLD | COLOR_PAIR(3);}
    for (i=0;i<line1.length();i++) {
      mvwaddch(statswin,15,i+9,line1[i] | attr1);
    }
  }
  if (creature->bleeding > 0) {
    line1="Bleeding";
    attr1= COLOR_PAIR(3);
    for (i=0;i<line1.length();i++) {
      mvwaddch(statswin,15,i,line1[i] | attr1);
    }
  }
  if (creature->disarmed > 0) {
    line1="Disarmed";
    attr1=A_BOLD | COLOR_PAIR(2);
    for (i=0;i<line1.length();i++) {
      mvwaddch(statswin,16,i,line1[i] | attr1);
    }
  }
  if (creature->sundered > 0) {
    line1="Sundered";
    attr1=A_BOLD | COLOR_PAIR(2);
    for (i=0;i<line1.length();i++) {
      mvwaddch(statswin,16,i+9,line1[i] | attr1);
    }
  }

  //  mvwprintw(statswin,14,0,line1.c_str());
  //mvwprintw(statswin,15,0,line2.c_str());

  wrefresh(statswin);
}

void Screen::message(string message)
{
  int j;
  int attributes[4]; //=A_NORMAL;
  nummsg++;
  for (j=0;j<COLS;j++) {
    mvwaddch(messagewin,0,j,' ');
    mvwaddch(messagewin,1,j,' ');
    mvwaddch(messagewin,2,j,' ');
    mvwaddch(messagewin,3,j,' ');
  }
  for (j=0;j<4;j++) {
    if (j<nummsg) {
      attributes[j]=A_BOLD;
    }
    else {
      attributes[j]=A_NORMAL;
    }
  }
  if (message[0] > 96) {message[0] = message[0] - 32;}

  for (j=0;j<message.length();j++) {
    mvwaddch(messagewin,0,j,message[j] | attributes[0]);
  }
  for (j=0;j<message1.length();j++) {
    mvwaddch(messagewin,1,j,message1[j] | attributes[1]);
  }
  for (j=0;j<message2.length();j++) {
    mvwaddch(messagewin,2,j,message2[j] | attributes[2]);
  }
  for (j=0;j<message3.length();j++) {
    mvwaddch(messagewin,3,j,message3[j] | attributes[3]);
  }

  if (message[0] != '>') {
    message3=message2;
    message2=message1;
    message1=message;
  }
  else {nummsg--;}
  wrefresh(messagewin);
}

// Help screen!!!!
void Screen::help()
{
  WINDOW * helpwin;
  helpwin = newwin(LINES,COLS,0,0);
  int i,j;
  for (j=0;j<LINES;j++) {
    for (i=0;i<COLS;i++) {
      mvwaddch(helpwin,j,i,' ');
    }
  }

  string line;
  line="List of commands : ";

  mvwprintw(helpwin,1,COLS/2-(line.length()/2)-2,line.c_str());

  line="Arrow keys or hjklyubn - Step/attack in direction.";
  mvwprintw(helpwin, 3, 3, line.c_str());

  line="q - Quaff a potion.";
  mvwprintw(helpwin, 4, 3, line.c_str());

  line="w - Wield or unwield an object.";
  mvwprintw(helpwin, 5, 3, line.c_str());

  line="W - Wear or unwear a piece of armor.";
  mvwprintw(helpwin, 6, 3, line.c_str());

  line="z - Zap a wand.";
  mvwprintw(helpwin, 7, 3, line.c_str());

  line="r - Read a scroll.";
  mvwprintw(helpwin, 8, 3, line.c_str());

  line="t - Throw an object.";
  mvwprintw(helpwin, 9, 3, line.c_str());

  line="f - If you are wielding a launcher (like a bow) and have ammo, fire it!";
  mvwprintw(helpwin, 10, 3, line.c_str());

  line="g - Get an object off the floor.";
  mvwprintw(helpwin, 11, 3, line.c_str());

  line="d - Drop an object onto the floor.";
  mvwprintw(helpwin, 12, 3, line.c_str());

  line="< or > - Ascend or descend a staircase.";
  mvwprintw(helpwin, 13, 3, line.c_str());

  line="S or Q - Save and quit, or just quit.";
  mvwprintw(helpwin, 14, 3, line.c_str());

  line="i - View inventory.";
  mvwprintw(helpwin, 15, 3, line.c_str());

  line="x - eXamine surrounding.";
  mvwprintw(helpwin, 16, 3, line.c_str());

  line="o or c - Open or close a door.";
  mvwprintw(helpwin, 16, 3, line.c_str());

  line="Press any key to continue.";
  mvwprintw(helpwin, 18, 3, line.c_str());

  wrefresh(helpwin);
  char ch;
  ch = getch();

  delwin(helpwin);
  touchwin(messagewin);
  touchwin(statswin);
  touchwin(levelwin);
  wrefresh(messagewin);
  wrefresh(statswin);
  wrefresh(levelwin);  


}

// Level up prompt!
void Screen::level(Creature * creature, int skillstoup)
{
  //  char * monskills=creature->skills;
  WINDOW * levelupwin;
  levelupwin = newwin(LINES,COLS,0,0);

  // clear screen
  int i,j,k;
  int levelup=0;
  if (skillstoup > 0) {levelup=1;}
  for (j=0;j<LINES;j++) {
    for (i=0;i<COLS;i++) {
      mvwaddch(levelupwin,j,i,' ');
    }
  }

  int arrangement[10] = {0,6,9,2,5,7,1,8,3,4};
  int tree[10] = {-1,-1,-1,-1,-1,2,0,2,-1,0};
  int req[10] = {0,0,0,0,0,3,3,3,0,3};
  int max[10] = {-1, -1, 20, -1, -1, 1, 1, -1, -1, 20 };
  int accept[10];

/*

melee
|-2wepfighting
\-disarm
blocking
|-parrying
\-hardness
dodging
ranged
magic
stealth

*/

  // show blank itemwin
  wrefresh(levelupwin);
  string skills[10];
  string skillnames[10];
  skills[0]="melee";
  skills[1]="dodging";
  skills[2]="blocking";
  skills[3]="magic";
  skills[4]="stealth";
  skills[5]="parrying";
  skills[6]="2wepfighting";
  skills[7]="hardness";
  skills[8]="ranged";
  skills[9]="disarm";

  skillnames[0]="Melee Combat";
  skillnames[1]="Dodging";
  skillnames[2]="Blocking";
  skillnames[3]="Spellcasting";
  skillnames[4]="Stealth";
  skillnames[5]="|- Weapon Blocking";
  skillnames[6]="|- Two Weapon Fighting";
  skillnames[7]="+- Item Preservation";
  skillnames[8]="Ranged Combat";
  skillnames[9]="+- Disarming";

  while (1 > 0) {
    string line="Advance skills :";
    string line2;
    string noline;
    char letter='a';
    mvwprintw(levelupwin,0,COLS/2-(line.length()/2)-2,line.c_str());
    for (i=0;i<9;i++) {
      noline=" ";
      accept[i]=1;
      k=arrangement[i];
      line=letter;
      line+=" - " + skillnames[k];
      letter++;
      for (j=0;j<COLS;j++) {
	mvwprintw(levelupwin,3+i,j," ");
      }
      
      int skillnumnow = creature->getskill(skills[k]);
      stringstream current;
      current << skillnumnow;
      current >> line2;
      line = line;
      int attr = A_NORMAL;

      if (tree[k] > -1) {
	if (req[k] > creature->getskill(skills[tree[k]])) { 
	  stringstream anotherone;
	  anotherone << req[k];
	  anotherone >> noline;
	  noline = " (Requires " + skillnames[tree[k]] + " " + noline + ")";
	  line += noline;
	  attr = COLOR_PAIR(3);
	  accept[i]=0;
//	  skillnumnow = 100;
	}
      }

      if ((skillnumnow > creature->level + 3) || ( max[k] > 0 && skillnumnow >= max[k] ) ) {
	noline = " (Maximum)";
	line += noline;
	attr = COLOR_PAIR(3);
	accept[i]=0;
      }

      for (j=0;j<line.length();j++) {
	mvwaddch(levelupwin,3+i,3+j,line[j] | attr);
      }  
    
//      mvwprintw(levelupwin,3+i,3,line.c_str());
      mvwprintw(levelupwin,3+i,COLS-10,line2.c_str());
    }

    int ch='1';

    if (skillstoup == 0) {
      line = "Press any key to continue.";
      mvwprintw(levelupwin, 15, 3, line.c_str());
      wrefresh(levelupwin);
      ch = getch();
      break;
    }
    else {

      stringstream pointsleft;
      line="Skill points left : ";
      pointsleft << skillstoup;
      pointsleft >> line2;
      mvwprintw(levelupwin, 15, 3, (line + line2).c_str());
      wrefresh(levelupwin);

    }

    int currentval;
    
    while (1>0) {
      while (ch < 'a' || ch > letter) {
        ch = getch();
      }
      k = arrangement[ch-'a'];
//      if (creature->getskill(skills[k]) > creature->level + 3) {ch='0';}
      if (accept[ch-'a'] == 0) {ch='0';}
      else {break;}
    }
    //ch-='a';
    currentval = creature->getskill(skills[k]);
    creature->setskill(skills[k], currentval+1);
    skillstoup--;
  }

  if (levelup > 0) {
    creature->maxhp += creature->toughness;
    creature->hp += creature->toughness;
    creature->level++;
  }

  delwin(levelupwin);
  touchwin(messagewin);
  touchwin(statswin);
  touchwin(levelwin);
  wrefresh(messagewin);
  wrefresh(statswin);
  wrefresh(levelwin);  

}

string findnumberofitems(llist **ilist, Item * citem)
{

  llist * temp;
  Item * nextitem;
  int number=0;
  temp = *ilist;
  while (temp != NULL ) {
    nextitem = (Item *)(list_item(&temp));
    if ((citem->name).compare(nextitem->name) == 0) {
      number++;
    }
    temp = *(&temp->next);
  }
  string toreturn;
  if (number > 1) {
    stringstream river;
    river << number;
    river >> toreturn;
    toreturn = " (" + toreturn + ")";
  }
  else {toreturn="";}
  
  return toreturn;

}

// Fuck filtering. Filtering can be the job of whatever calls choosefromlist.
void *Screen::choosefromlist(llist **ilist, string imsg, objtype kind, Creature * owner)
{

  WINDOW * itemwin;
  int keepgoing = 1;
  llist * itemses;
  llist * possessions;
  char letter = 'a';
  string message;
  string submessage="";
  Item * citem;

  int cpos=-1;
  int linecolor[LINES];
  Item * items[LINES];
  int itempos[LINES];
  char currentitem='a';

  itemses = *ilist;
  possessions = *ilist;
  possessions = itemses;  
  
  itemwin = newwin(LINES,COLS,0,0);
  
  // clear screen
  int i,j;
  for (j=0;j<LINES;j++) {
    items[j]=NULL;
    itempos[j]=-1;
    linecolor[j]=0;
    for (i=0;i<COLS;i++) {
      mvwaddch(itemwin,j,i,' ');
    }
  }

  // show blank itemwin
  wrefresh(itemwin);

  mvwprintw(itemwin,0,40-(imsg.length()/2),(imsg + "\n\n").c_str());

  int iii=0;
  int jjj=-1;

  if (possessions == NULL) {
    switch (kind) {
    case creature: message="There is no one here."; break;
    case spell: message="No spells."; break;
    default: message="You have nothing.";
    }
  }
  else {
    while(possessions != NULL) {
      if (kind == item) {
	int cont=0;
	jjj++;
	citem = (Item *)(list_item(&possessions));
	if (citem->stackable == 1) {
	  for (i=0;i<(int)(currentitem-'a');i++) {
	    if ((citem->name).compare((items[i])->name) == 0) {
	      cont=1;
	      break;
	    }
	  }
	  if (cont==1) {possessions=*(&possessions->next); continue;}
	}
	itempos[currentitem-'a'] = jjj;
	items[currentitem-'a'] = citem;
	currentitem++;

      }
      if (iii==0) {
	message = letter;
	message = message + " - ";
      }
      else {
	letter++;
	message = message + "\n";
	message = message + letter + " - ";
      }
      switch (kind) {
      case item: {

	if (citem->hplost > 5) {
	  cpos++;
	  linecolor[cpos]=iii+2;
	  if (citem->hplost > 8) {
	    linecolor[cpos]=-iii-2;
	  }
	}
	
	submessage = citem->getname();
	if (citem->stackable == 1) {submessage += findnumberofitems(&possessions, citem);}
	break;

      }
      case creature: submessage = ((Creature *)(list_item(&possessions)))->getname(); break;
      case spell: submessage = ((Spell *)(list_item(&possessions)))->name; break;      
      }
      iii++;
      int templen = 30-submessage.length();
      for (i=0;i<(templen);i++) {
	submessage = submessage + " "; 
      }
      if (kind == item && owner != NULL) {submessage = submessage + (owner->isworn((Item *)(list_item(&possessions))));}
      message = message + submessage;
      possessions = *(&possessions->next);
    }
  }
  possessions = itemses;
  wprintw(itemwin,message.c_str());

  if (cpos > -1) {
    for (iii=0;iii<cpos+1;iii++) {
      int color=2;
      if (linecolor[iii] < 0) {color=3; linecolor[iii]=-(linecolor[iii]);}
      mvwchgat(itemwin,linecolor[iii],0,-1, A_BOLD, color, NULL );
    }
  }

  wrefresh(itemwin);

  int ch;
  ch = getch();
  
  delwin(itemwin);
  touchwin(messagewin);
  touchwin(statswin);
  touchwin(levelwin);
  wrefresh(messagewin);
  wrefresh(statswin);
  wrefresh(levelwin);

  if (ch >= 'a' && ch <= letter) {
    if (ch > 'a') {
      int ubound = (int)(ch-'a');
      if (kind == item) {ubound = itempos[(ch-'a')];}
      for (i=0;i<ubound;i++) {
	possessions = *(&possessions->next);
      }
    }
  }
  else {
    //    list_delete(&itemses);
    return NULL;
  }
  
  void * tobereturned = (list_item(&possessions));
  //list_delete(&itemses);
  
  return tobereturned;

  
}

Item *Screen::inventory(Creature * creature, string imsg,string filter)
{
  llist **ilist;
  ilist = creature->inventory();
  //  WINDOW * itemwin;
  int keepgoing = 1;
  llist * itemses=NULL;
  llist * possessions;
  char letter = 'a';
  string message;
  string submessage="";

  string objecttype;
  possessions = *ilist;
  while (possessions != NULL) {
    objecttype = ((Item *)list_item(&possessions))->gettype();
    if (filter.find(objecttype) != string::npos || filter.compare("all") == 0) {
      list_append(&itemses,(void *)list_item(&possessions));
    }
    possessions = *(&possessions->next);
  }
  possessions = itemses;

  objtype kind=item;
  
  Item * tobereturned = (Item *)(this->choosefromlist(&possessions, imsg, kind, creature));
  
  //  Item * tobereturned = (Item *)(list_item(&possessions));
  list_delete(&itemses);
  
  return tobereturned;

}
