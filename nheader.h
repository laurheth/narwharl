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

#ifdef MINGW
#include <curses.h>
#else
#include <ncurses/cursesapp.h>
#endif

#include <string>
//#include <cstring>
#include <cstdlib>
#include <time.h>
#include "mtrand/mtrand.h"

#define DMGTYPES (4) // Number of defined damage types.
#define NUMEFFECTS (7) // Number of defined item effects.
#define PI (3.14159265) // Value for PI.
#define NUMSKILLS (12)
#define XPCURVE (10) // experience curve. XPCURVE * current level is the amount needed to go up.

// Define color pair shortcuts.
#define WHITE (1)
#define BROWN (2)
#define RED (3)
#define GREEN (4)
#define BLUE (5)
#define MAGENTA (6)
#define CYAN (7)

using namespace std;

// Object type enum
enum objtype {
  creature,
  item,
  spell
};

// Spell effect enum

enum effecttype {
  noeffect,
  firedmg,
  icedmg,
  identifyitem
};

// Spell travel type enum
enum traveltype {
  untilopaque,
  untilhit
};

// Natural weapon enum
enum natwep {
  bite, // half bludgeoning and cutting, and more painful
  claw, // all cutting
  slam, // all bludgeoning
  horns // all cutting, more painful
};

// Body part!
enum bpart {
  all,
  head,
  upper,
  lower,
  feet,
  lhand,
  rhand
};

#define AMMOTYPES (1) // Number of defined ammo types.

// Consumables!
enum i_fuel {
  none,
  arrow
};

class Creature;

class Item;

class Map;

class Screen;

// Spell class
class Spell
{
 public:
  Spell(); // Constructor
  ~Spell(); // Destructor
  string name; // Name of spell.
  effecttype effect; // Effect type
  traveltype travel; // Travel type (i.e. go until hit? go until opaque?)
  int trail; // Leave trail until stopping.
  int level; // Level of spell. Also, cost!
  int range; // Maximum range of spell.
  int skiprange; // Skip this distance before beginning to apply effect.
  int spread; // Spread, for cones.
  int blastradius; // At end of travel, blast radius. (i.e. for a fireball)
  int stopattarget; // Stop at target, or keep going to full range?
  int args[6]; // Numerical arguments interpretted by effect.
  char color; // Effect color!
  int cast(Creature * caster, Map * map, Screen * vscreen, int targy, int targx);
};

typedef struct mons {
  streampos ware; // Where in the def file to look.
  int level; // Level of monster/object
} mondef;

typedef struct dmgtypes{
  unsigned char rolls[DMGTYPES]; // Number of rolls
  unsigned char var[DMGTYPES]; // Size of dieroll. So in the 1d6 notation, (rolls)d(var)
} damage;

typedef struct dmgdone{
  int dv[DMGTYPES];
} dmgval;

typedef struct node{
  void *x; // Pointer to object in list.
  node *next; // Pointer to next node.
} llist;

typedef struct xycords{
  int x;
  int y;
} coords;

llist *list_add(llist **p, void *add);
llist *list_append(llist **p, void *add);
void list_remove(llist **p);
void list_delete(llist **p);
llist **list_search(llist **n, void *find);
void *list_item(llist **p);
void *list_merge(llist **p, llist * q);

// Tile definition
typedef struct{
  unsigned int visible : 1;
  unsigned int passable : 1;
  unsigned int seen : 1;
  unsigned int bloody : 1;
  unsigned int seethrough : 1;
  unsigned int opacity : 3;
  unsigned int effected : 1; // Mark squares influenced by a spell.
  unsigned int shootthrough : 1;
  char color;
  char symbol;
  char seensymbol;
  Creature *creature;
  llist *ilist;
} tile;

// Object placer
class Stuffgetter
{
 public:
  Stuffgetter();
  ~Stuffgetter();
  Creature * addcreature(int level);
  Item * addweapon(int level);
  Item * addlauncher(int level);
  Item * addmissile(int level);
  Item * addmissile(i_fuel ammotype);
  Item * addarmor(int level);
  Item * addpotion(int level);
  Item * addscroll(int level);
  Item * addwand(int level);
  Spell * spellbyid(int idnum);
  Spell * spellbyname(string spellname);
  llist ** allspells();
  string * scrollmystery; // Mystery scroll names in an array.
  string * wandmystery;
  string * potmystery; // Mystery potion names in an array.
  string potionname(); // Generate a random mystery potion name.
  string wandname(); // Generate a random mystery wand name.
  int nummons;
  int numweps;
  int numlaunch;
  int numarmors;
  int numpots;
  int numspells;
 private:
  llist * mondefs;
  llist * weapondefs;
  llist * armordefs;
  llist * potiondefs;
  llist * spells;
  llist * launcherdefs;
};

// Map class definition
class Map
{
 public:
  Map(int sseed);
  ~Map();
  void allocatemap(int y, int x);
  void deallocatemap();
  int getopacity(int y, int x);
  int getsymb(int y, int x);
  int cango(int y, int x);
  int canshoot(int y, int x);
  int seethrough(int y, int x);
  int opendoor(int y, int x);
  int closedoor(int y, int x);
  int inside(int y, int x);
  void see(int y, int x);
  void unsee(int y, int x);
  int seen(int y, int x);
  int cansee(int y, int x);
  void go(Creature *creature, int y, int x);
  void put(Item *item, int y, int x);
  void leave(int y, int x);
  Creature * checkmonster(int y, int x);
  void additem(Item * item, int y, int x);
  string look(int y, int x);
  void unsave();
  int savemap();
  int savemap(int lvl);
  int loadmap(llist ** mmlist);
  int loadmap(llist ** mmlist, int lvl);
  llist ** getitems(int y, int x);
  llist ** mlist;
  llist * currentmon;
  int genmap(llist ** mmlist, int *sy, int *sx);
  int newgenmap(llist ** mmlist, int *sy, int *sx, Map * prevmap);
  void makebloody(int y, int x);
  int ysize;
  int xsize;
  int iseffected(int y, int x);
  void seteffected(int y, int x, int value);
  tile gettile(int y, int x);
  void settile(int y, int x, tile newtile);
  char dlvl;
  char plvl;
  char maxdlvl;
  int pop;
  int maxpop;
  char getfloorsymb(int y, int x);
  void setmaxdlvl(char newmax);
  int loop(Screen * vscreen);
  void changelevel(char change, Creature * changer);
  void addnewmonster();
  int **plan;
  void placemonster(int y, int x);
  void placeweapon(int y, int x);
  void placearmor(int y, int x);
  void placepotion(int y, int x);
  void placescroll(int y, int x);
  void placewand(int y, int x);
  void placelauncher(int y, int x);
  void placeammo(int y, int x);
 private:
  void rpos(int *y, int *x);
  int mkroom(int y, int x, int exits, int roomdepth);
  int hallway(int y, int x, int dir, int roomdepth);
  int seed;
  //  tile map[20][80];
  tile **map;
};

// View screen class definition

class Screen
{
 public:
  Screen();
  ~Screen();
  int getachar(Map * map, int y, int x);
  void showlevel(Map *map, int y, int x);
  void message(string message);
  void hidelevel(Map *map);
  void movecursor(Map *map, int * y, int * x);
  Item *inventory(Creature * creature, string imsg, string filter);
  void stats(Creature * creature);
  void help();
  void drawatpos(int y, int x, char symb, char color);
  void drawatpos(int y, int x, int symb);
  void drawoutwards(int y, int x, int range, char symb, char color, Map *map);
  void *choosefromlist(llist **ilist, string imsg, objtype kind, Creature * creature);
  int nummsg;
  void level(Creature * creature, int skillstoup);
 private:
  string message1;
  string message2;
  string message3;
  int uleftx;
  int ulefty;
  int levelwinx;
  int levelwiny;
  WINDOW *levelwin;
  WINDOW *messagewin;
  WINDOW *statswin;
};

// General Item class definition

class Item
{
 public:
  Item(Map *map, int sy, int sx, char ssymb, string sname);
  ~Item();
  int x;
  int y;
  char color;
  int uses;
  char getsymb();
  string getname(); // Returns name if identified. mysteryname if not.
  string gettype();
  void settype(string newtype);
  int getmass();
  string getsubcat();
  void setsubcat(string subcat);
  void setmass(int newmass);
  damage getdamage(int take);
  void setdamage(damage newdamage);
  char * geteffects();
  void setmysteryname(string mystery);
  string name;
  string mysteryname;
  void identify();
  void setblockchance(char chance);
  char getblockchance();
  void setspell(Spell * newspell);
  string getspellname();
  int cast(Creature * reader, Map *map, Screen *vscreen, int targy, int targx);
  int hplost;
  char hitbonus;
  char dmgbonus;
  char stackable;
  i_fuel consumes;
  int size;
 protected:
  Spell * spell;
  char blockchance;
  char * effects;
  string subtype;
  damage dmg;
  int mass;
  unsigned char type;
  char symb;
};

// General Creature class definition

class Creature
{
 public:
  Creature(Map *map, int sy, int sx, char ssymb, string sname);
  ~Creature();
  void step(Map *map, int dx, int dy, Screen *vscreen);
  char getsymb();
  char getcolor();
  string getname();
  string name;
  int x;
  int y;
  char color;
  virtual int ai(Map *map, Screen *vscreen);
  int strike(Creature * target, Item * strikewith, string verb, Screen *vscreen, int *damage, int penalty, string * object, Map *map);
  void attack(Creature * target, Map * map, Screen *vscreen);
  void fire(int targy, int targx, Map *map, Screen *vscreen, int rangeincrement, Item * item);
  void fire(int targy, int targx, Map *map, Screen *vscreen, int rangeincrement, Item * item, Item *launcher);
  Item * getmissile(Item * launcher);
  //  int launch(int targy, int targx, Map *map, Screen *vscreen, Item *launcher);
  void die(Map *map);
  void quaff(Item * potion, Screen *vscreen);
  string itemeffect(Item * potion);
  char time;
  int viewdist;
  int strength;
  int dexterity;
  int agility;
  int toughness;
  int size;
  int level;
  int leveladjust;
  int experience;
  int maxhp;
  int hp;
  int maxmp;
  int mp;
  int isplayer;
  int poisoned;
  int bleeding;
  int healrate;
  int healcount;
  int rechargerate;
  int rechargecount;
  int disarmed;
  int sundered;
  int load; // Load. Encumberance. Whatever.
  void calcload(); // Recalculate encumberance
  llist **inventory();
  llist **armor();
  llist **spells();
  //  void spellsfromlist(llist ** newspelllist);
  int wear(Item * item);
  int unwear(Item * item);
  int wield(Item * item);
  int unwield(Item * item);
  string isworn(Item * item);
  string savepos(Item * item);
  string destroypos(Item * item);
  Item * lhand;
  Item * rhand;
  natwep * naturalweapons;
  int natwepnum;
  string wearingarmor(string armortype);
  void bleed(Map * map);
  void poison();
  void heal();
  void burn();
  void freeze();
  int cast(Spell * spelltocast, Map *map, Screen *vscreen, int targy, int targx);
  void read(Item * item, Map *map, Screen *vscreen, int targy, int targx);
  void zap(Item * item, Map *map, Screen *vscreen, int targy, int targx);
  Item * block();
  int getskill(string skillname);
  void setskill(string skillname, int newvalue);
  char skills[NUMSKILLS];
  int id;
  int attacker;
  void increasestat(string stat);
  int animation : 6;
  bool onfire;
  bool frozen;
  bool special;
 protected:
  llist * larmor;
  Item * neck;
  Item * lring;
  Item * rring;
  llist * possessions;
  llist * spellist;
  char symb;
};

// Player class definition

class Player: public Creature
{
 public:
  int turns;
  Player(Map *map, int sy, int sx, char ssymb, string sname);
  int ai(Map *map, Screen *vscreen);
};

class Monster: public Creature
{
 public:
  Monster(Map *map, int sy, int sx, char ssymb, string sname);
  int ai(Map *map, Screen *vscreen);
  int targetx; // Target location
  int targety; // ditto
  int numberofspells; // Number of known spells, used for decided which one to cast.
  int active; // If >0, monster is awake and actively searching for the player.
  void details(); // Sorts out some values used for ai, like number of spells.
  llist * plannedpath; // Linked list of the steps needed to follow pathfinding.
  void redopath(Map *map, Screen *vscreen); // Delete plannedpath and build a new one.
};

// Various function prototypes.

string findnumberofitems(llist **ilist, Item * citem);

void advancemon(Creature * creature, int levels);

llist * findapath(int sy, int sx, int ty, int tx, Map * map, llist ** moves, Screen *vscreen);

int lineofsight(Map *map, int y, int x, int ty, int tx, int viewdist);

int spellget(llist ** stuffs, string deffile);

int see(Map *map, int y, int x, int action);

int hit(Map *map, int y, int x, int action);

int spellsee(Map *map, int y, int x, int action);

int spellhit(Map *map, int y, int x, int action);

int bloodtrail(Map *map, int y, int x, int action);

int fuck(Map *map, int y, int x, int action); // Why is this even still here?

void aoe(Map *map, int y, int x, int d, int (*aoefunc)(Map *,int,int,int));

void los(Map *map, int y, int x, int * ty, int * tx, int * d, int (*aoefunc)(Map *,int,int,int),int jinit);

void los(Map *map, float y, float x, float * ty, float * tx, int * d, int (*aoefunc)(Map *,int,int,int),int jinit);

void aoe(Map *map, float y, float x, int d, int (*aoefunc)(Map *,int,int,int));

double mtrand();

void mtinit(int * seed);

dmgval calcdamage(damage wpndmg, int strength, int dudesize);

dmgval sumdmg(dmgval val1, dmgval val2, int scale);

dmgval armorcalc(llist **armor, bpart where);

int getlist (llist ** stuffs, string deffile);

Creature * getcreature(streampos ware, Map *map, int y, int x);

Item * getitem(streampos ware, Map *map, int y, int x, string deffile);

bpart whereonbody(int attackersize, int defendersize, string * bpartname);
