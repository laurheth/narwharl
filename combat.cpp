/*
NarwhaRL - Nice And Relaxing or Worst Hell Always? the RogueLike                                                                                          Copyright (C) 2010-2018 Lauren Hetherington

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
#include <sstream>
#include <cmath>
#include <string>

using namespace std;

extern string knowledge;

string parteffect(bpart bodypart, int * dmg, Creature * target, Map * map, int dmgtype, int critbonus);

Item * Creature::getmissile(Item * launcher)
{
  i_fuel ammotype;
  llist * temp;
  Item * maybemissile;
  temp = *(this->inventory());
  ammotype = launcher->consumes;
  while (temp != NULL) {
    maybemissile = (Item *)(list_item(&temp));
    if (maybemissile->consumes == ammotype) {
      if ((maybemissile->gettype()).compare("missile") == 0) {
	break;
      }
    }
    temp = *(&temp->next);
    maybemissile = NULL;
  }
  if (maybemissile == NULL) {return NULL;}
  return maybemissile;
}

// Ranged attack
void Creature::fire(int targy, int targx, Map *map, Screen *vscreen, int rangeincrement, Item * item)
{
  this->fire(targy, targx, map, vscreen, rangeincrement, item, NULL);
}

void Creature::fire(int targy, int targx, Map *map, Screen *vscreen, int rangeincrement, Item * item, Item * launcher)
{
  int ty,tx,dy,dx,lx,ly, px,py;
  int keepflying=1;
  int range = rangeincrement;
  int rangeleft = range;
  int travelled = 0;
  int thisjump=0;
  int tothitbonus=item->hitbonus;
  int totdmgbonus=item->dmgbonus;
  string message;
  string bparthit=" ";
  Creature * target;

  dmgtypes weapondamage;
  weapondamage = item->getdamage(0);

  if (launcher != NULL) {
    dmgtypes tempdmg = launcher->getdamage(-1);
    tothitbonus += launcher->hitbonus;
    totdmgbonus += launcher->hitbonus;
    int i;
    for (i=0;i<DMGTYPES;i++) {
      if (tempdmg.rolls[i] > weapondamage.rolls[i]) {
	weapondamage.rolls[i] = tempdmg.rolls[i];
      }
      if (tempdmg.var[i] > weapondamage.var[i]) {
	weapondamage.var[i] = tempdmg.var[i];
      }
    }
  }

  px=x;
  py=y;
  tx=x;
  ty=y;
  dy = targy - y;
  dx = targx - x;
  int tries = 10;
  while (keepflying == 1 && tries > 0) {
    rangeleft=range;
    tries = tries-1;

    lx=x;
    ly=y;
    tx=lx+dx;
    ty=ly+dy;
    los(map, ly, lx, &ty, &tx, &rangeleft, spellhit, travelled+1); // Use line of sight algorithm to trace flight path.

    // Confusing terminology. &rangeleft points to the range, but then rangeleft gets filled with the distance travelled. Could have been better thought out.
    travelled=rangeleft;
    if (px == tx && py == ty) {break;}
    if (travelled < 0) {break;}
    px = tx;
    py = ty;
    if (travelled > range) {travelled = range;}
    rangeleft = range - travelled;
    target = map->checkmonster(ty,tx);
    // To hit stuff. This should be a separate method.
    if (target != NULL) {
      int tohit, tododge,hitchance;
      tohit = ((target->size) * (dexterity) * (range-travelled))/range;
      tododge = (size) * (target->agility);
      hitchance = 50*(tohit - tododge);
      hitchance = 50 + hitchance / (tohit + tododge) + target->load + 5*(tothitbonus + this->getskill("ranged") - target->getskill("dodging"));
      //      hitchance=0;
      if ((int)(100 * mtrand() + 1) < hitchance) {
	this->increasestat("dexterity");
	if (mtrand() > 0.5) {
	  item->hplost += 1;
	  if ((item->gettype()).compare("missile") == 0) {
	    item->hplost += (char)(4*mtrand());
	  }
	}
	if ((item->gettype()).compare("potion") == 0) {
	  item->hplost = 20;
	}
        int i;
        dmgval wep;
        dmgval armor;
        dmgval dmgdone;

        int damage;
        string dmg;
        stringstream stream;
        int maxdmg;
        int iii,jjj;
	int effstr=(strength*size)/100;

	if (launcher != NULL) {
	  if (effstr > 10) {effstr=10;}
	}

        wep = calcdamage(weapondamage,effstr,100);
        maxdmg=0;
        iii=0;

        for (jjj=0;jjj<DMGTYPES;jjj++) {
          if (wep.dv[jjj] > maxdmg) {
            maxdmg = wep.dv[jjj];
            iii=jjj;
          }
        }

        damage = 0;

        Item * blocker;
        blocker = target->block();
        if (blocker != NULL) {

          message = target->getname() + " blocks the " + item->getname() + "!";

          // Deal damage to the object used to block.
          //dmgtobedone = calcdamage(damagetobedone,strength,size);
          for (i=0;i<DMGTYPES;i++) {
            damage = damage + wep.dv[i];
          }

          damage += totdmgbonus;

          vscreen->message(message);

          int hardness=1+(blocker->getmass())/20;

          damage = damage - hardness;

          if (damage > 0) {
            blocker->hplost += 1;
            if (blocker->hplost > 10) {
	      string tempstr;
	      target->sundered=1;
              tempstr=target->destroypos(blocker);
	      vscreen->message(tempstr);
            }
          }

          damage=0;


        }
        else {
          bpart bodypart = whereonbody(size, target->size, &bparthit);
          armor = armorcalc(target->armor(),bodypart);
          wep.dv[iii] += totdmgbonus;
          dmgdone = sumdmg(wep,armor,-1);
          message = "The " + item->getname() + " strikes " + target->getname();

          int maxtype=0;
          int lastmax=0;	  

          for (i=0;i<DMGTYPES;i++) {
            if (wep.dv[i] > lastmax) {maxtype=i; lastmax = wep.dv[i];}
	    damage = damage + dmgdone.dv[i];
	  }

	  string effectmessage = target->itemeffect(item);

	  string bonusmessage = parteffect(bodypart, &damage, target, map, maxtype, 0);

          target->hp = target->hp - damage;

          message = message + "'s" + bparthit + "doing ";
          stream << damage;
          stream >> dmg;
          message = message + dmg +  " damage!";
	  target->attacker = this->id;

          vscreen->message(message);
	  if (item->hplost > 10) {
	    vscreen->message("The " + item->getname() + " shatters on impact!");
	  }
          if (effectmessage[0] != '-') {vscreen->message(effectmessage);}
          if (bonusmessage[0] != '-') {vscreen->message(bonusmessage);}

          if ((target->isplayer != 1) && (target->hp < 1)) {
            message = target->getname() + " dies.";
            target->die(map);
            llist ** toremove = list_search((map->mlist), (void *)target);
            if (list_item(toremove) == (void *) target) {
              list_remove(toremove);
              delete target;
              target=NULL;
            }
	    vscreen->message(message);
          }

        }

        keepflying=0; // Successful hit. Stop flying.


      }
      else {
	target->increasestat("agility");
      }

    }
    if (rangeleft < 1) {keepflying=0;} // If out of range, stop flying.
  }

  vscreen->drawoutwards(y,x, range+2, item->getsymb(), item->color, map); // Draw!

  if (item->hplost > 10) {
    message=this->destroypos(item);
  }
  else {

    list_add(map->getitems(ty,tx),(void *)item); // Place item on the ground.
    list_remove(list_search(this->inventory(),(void *)item)); // Remove item from thrower's inventory.
    item->y = ty; // Item location
    item->x = tx;
  }
  this->calcload();
  time=(char)(40/agility); // Load doesn't slow down shooting.

}

// Individual strike
int Creature::strike(Creature *target, /*dmgtypes damagetobedone*/ Item * strikewith, string verb, Screen *vscreen, int * damage, int penalty, string *object, Map * map )
{
  time=(char)(40/agility); // Load doesn't slow down melee.
  dmgtypes damagetobedone;
  damagetobedone=strikewith->getdamage(0);
  if (*damage > target->hp) {return 1;}

  int hitbonus = (int)(strikewith->hitbonus);
  int dmgbonus = (int)(strikewith->dmgbonus);

  string message;
  string bparthit;
  int dmgdone=0;
  int tohit, tododge, hitchance, j, i, maxdmg;
  string dmg;
  stringstream stream;
  dmgval armor;
  dmgval dmgtobedone;
  dmgval wepdmg;

  string effectmessage = "-";

  string bonusmessage = "-";

  message = this->getname() + " " + verb + " " + target->getname();

  tohit = ((target->size) * (dexterity));
  tododge = (size) * (target->agility);

  hitchance = 50*(tohit - tododge);
  hitchance = 50 + hitchance / (tohit + tododge) + target-> load + 5*( hitbonus + this->getskill("melee") - target->getskill("dodging"));

  hitchance = (hitchance * (6-penalty))/6;
  if ((int)(100 * mtrand()+1) > hitchance) {
    target->increasestat("agility");
    return 0;
  }

  if ((strikewith->gettype()).compare("weapon") != 0 && (strikewith->gettype()).compare("shield") != 0) {
    if (mtrand() > 0.8) {
      strikewith->hplost += 1;
    }
  }
  if ((strikewith->gettype()).compare("potion") == 0) {
    strikewith->hplost = 20;
  }

  Item * blocker;
  blocker = target->block();

  if (blocker != NULL) {
    if (object != NULL) {message = message + " with a " + *object;}
    if ((blocker->gettype()).compare("shield")==0) {
      message = message + " but the attack is blocked!";
    }
    else {message = message + " but the attack is parried!";}
    // Deal damage to the object used to block.
    dmgtobedone = calcdamage(damagetobedone,strength,size);

    for (j=0;j<DMGTYPES;j++) {
      dmgdone = dmgdone + dmgtobedone.dv[j];
    }

    dmgdone += dmgbonus;

    vscreen->message(message);

    int hardness=1 + (blocker->getmass())/20;

    dmgdone = dmgdone - hardness;

    if (dmgdone > 0) {
      blocker->hplost += 1;
      if (blocker->hplost > 10) {
	string tempstr;
	target->sundered=1;
        tempstr=target->destroypos(blocker);
	vscreen->message(tempstr);
      }
    }

    dmgdone=0;

  }
  else {
    this->increasestat("dexterity");
    this->increasestat("strength");
    maxdmg=0;
    i=0;
    wepdmg = calcdamage(damagetobedone,strength,size);
    for (j=0;j<DMGTYPES;j++) {
      if (wepdmg.dv[j] > maxdmg) {
        maxdmg = wepdmg.dv[j];
        i=j;
      }
    }
    wepdmg.dv[i] += dmgbonus;
    bpart bodypart = whereonbody(size, target->size, &bparthit);
    armor = armorcalc(target->armor(),bodypart);
    dmgtobedone = sumdmg(wepdmg,armor,-1);
    for (j=0;j<DMGTYPES;j++) {
      dmgdone = dmgdone + dmgtobedone.dv[j];
      if (j==1 && dmgtobedone.dv[j] > 0) {
        int k=dmgtobedone.dv[j];
        while (k>0) {
          k=k-10;
          map->makebloody(target->y+(int)(3*mtrand())-1,target->x+(int)(3*mtrand())-1);
        }
      }
    }
    message = message + "'s " + bparthit;
    int critbonus=0;
    if (bparthit.find("hand") != string::npos) { critbonus = this->getskill("disarm"); }

    effectmessage = target->itemeffect(strikewith);

    bonusmessage = parteffect(bodypart, &dmgdone, target, map, i, critbonus);

    if (object != NULL) {message = message + "with a " + *object + " ";}
    if (dmgdone > 0) {
      message = message + "doing ";
      stream << dmgdone;
      stream >> dmg;
      message = message + dmg +  " damage!";
      target->attacker = this->id;
    }
    else {
      message = message + "but does no damage!";
    }
    vscreen->message(message);

  }
  if (strikewith->hplost > 10) {
    message=this->destroypos(strikewith);
    vscreen->message(message);
  }
  
  if (bonusmessage[0] != '-') {vscreen->message(bonusmessage);}
  if (effectmessage[0] != '-') {vscreen->message(effectmessage);}

  *damage = *damage + dmgdone;
  target->hp -= dmgdone;
  return 1;

}

// Melee attack
void Creature::attack(Creature *target, Map *map, Screen *vscreen)
{
  int tohit, tododge, hitchance;
  int penalty=0;
  int weps;
  int i,j;
  string bparthit=" ";
  string verb;
  int hitsuccess=0;
  int numberofhits=0;
  dmgval armor;
  dmgval dmgdone;
  int damage=0;
  Item naturalweapon(map, 0, 0, 's', "natural weapon");
  naturalweapon.settype("weapon");
  dmgtypes natwepdmg;
  for (i=0;i<DMGTYPES;i++) {
    natwepdmg.rolls[i]=0;
    natwepdmg.var[i]=0;
  }
  string message;
  message = this->getname() + " attacks " + target->getname() + " but misses!";

  if (natwepnum > 0) {
    for (i=0;i<natwepnum;i++) {
      natwep wepnat;
      wepnat = naturalweapons[i];
      switch (wepnat) {
      case bite:

        verb="bites";
        natwepdmg.rolls[0] = 1;
        natwepdmg.rolls[1] = 1;
        natwepdmg.var[0] = 3;
        natwepdmg.var[1] = 3;
        break;
      case claw:

        verb="claws";
        natwepdmg.rolls[0] = 0;
        natwepdmg.rolls[1] = 1;
        natwepdmg.var[0] = 0;
        natwepdmg.var[1] = 3;
        break;
      case slam:

        verb="slams";
        natwepdmg.rolls[0] = 1;
        natwepdmg.rolls[1] = 0;
        natwepdmg.var[0] = 3;
        natwepdmg.var[1] = 0;
        break;
      case horns:

        verb="gores";
        natwepdmg.rolls[0] = 0;
        natwepdmg.rolls[1] = 2;
        natwepdmg.var[0] = 0;
        natwepdmg.var[1] = 3;
        break;
      }
      naturalweapon.setdamage(natwepdmg);
      hitsuccess += this->strike(target, &naturalweapon, verb, vscreen, &damage, 0 , NULL, map);

    }
  }
  //  string message;
  string object;
  stringstream stream;

  if (lhand == NULL && rhand == NULL ) {weps=0;}
  else if (lhand == NULL && rhand != NULL) {weps=1;}
  else if (lhand != NULL && rhand == NULL) {weps=2;}
  else {
    weps=3;penalty=3;
    if (this->getskill("2wepfighting") > 0) {penalty=1;}
    if ((lhand->gettype()).compare("shield")==0 || (rhand->gettype()).compare("shield")==0) {
      penalty=0;
    }
  }

  if (weps==0 && natwepnum == 0) {
    natwepdmg.rolls[0] = 1;
    natwepdmg.rolls[1] = 0;
    natwepdmg.var[0] = 2;
    natwepdmg.var[1] = 0;
    verb="punches";
    naturalweapon.setdamage(natwepdmg);
    hitsuccess += this->strike(target, &naturalweapon, verb, vscreen, &damage, 0, NULL, map );
  }

  if (weps==1 || weps==3) {
    i=penalty;
    verb="hits";
    object = rhand->getname();
    if ((rhand->gettype()).compare("shield") == 0) {verb="bashes"; object="shield"; penalty=3;}
    hitsuccess += this->strike(target,rhand, verb, vscreen, &damage, penalty, &object, map);
    penalty=i;
  }

  if (weps==2 || weps==3) {
    i=penalty;
    verb="hits";
    object = lhand->getname();
    if ((lhand->gettype()).compare("shield") == 0) {verb="bashes"; object="shield"; penalty=3;}
    hitsuccess += this->strike(target,lhand, verb, vscreen, &damage, penalty, &object, map);
    penalty=i;
  }

  //  target->hp = target->hp - damage;

  if (hitsuccess==0) {vscreen->message(message);}

  if ((target->isplayer != 1) && (target->hp < 1)) {
    message = target->getname() + " dies.";
    target->die(map);
    llist ** toremove = list_search((map->mlist), (void *)target);
    if (list_item(toremove) == (void *) target) {
      list_remove(toremove);
      delete target;
      target=NULL;
    }
    vscreen->message(message);
  }


}

// Block an attack
Item * Creature::block()
{
  int chancetoblock;
  int skillscale=2;
  if (this->getskill("parrying") > 0) {skillscale=5;}
  if (lhand != NULL) {
    if ((lhand->gettype()).compare("shield") == 0) {skillscale=5;}
    else {skillscale=2;}
    chancetoblock = (int)(lhand->getblockchance()) + skillscale * this->getskill("blocking");
    if (chancetoblock > (int)(100*mtrand())) {return lhand;}
  }
  if (rhand != NULL) {
    if ((rhand->gettype()).compare("shield") == 0) {skillscale=5;}
    else {skillscale=2;}
    chancetoblock = (int)(rhand->getblockchance()) + skillscale * this->getskill("blocking");
    if (chancetoblock > (int)(100*mtrand())) {return rhand;}
  }
  return NULL;
}

// Effect of damage to various body parts.
string parteffect(bpart bodypart, int * dmg, Creature * target, Map * map, int dmgtype, int critbonus)
{

  string message;
  message = "-";
  Item * item;
  int x,y;
  int i;
  x = target->x;
  y = target->y;
  int nocritchance;
  nocritchance=((100 * (target->hp - *dmg))/(target->maxhp));
  if (nocritchance < 0) {nocritchance = 0;}
  nocritchance += 2*(target->toughness) - 5 * critbonus;

  if (nocritchance < (100*mtrand()+1)) {
    if (bodypart == head) {
      *dmg += *dmg;
    }
    else if (bodypart == lhand) {

      item = target->lhand;
      if (item == NULL) {return message;}
      target->disarmed=1;
      target->unwield(item);
      list_add(map->getitems(y,x),(void *)item);
      list_remove(list_search(target->inventory(),(void *)item));
      item->y = y;
      item->x = x;
      message = target->getname() + " dropped their " + item->getname() + "!";


    }

    else if (bodypart == rhand) {

      item = target->rhand;
      if (item == NULL) {return message;}
      target->disarmed=1;
      target->unwield(item);
      list_add(map->getitems(y,x),(void *)item);
      list_remove(list_search(target->inventory(),(void *)item));
      item->y = y;
      item->x = x;
      message = target->getname() + " dropped their " + item->getname() + "!";

    }
    else {

      if (dmgtype==1) {
	if (*dmg > 0) {
	  target->bleeding += 32 * (*dmg)-1;
	  if (*dmg < target->hp) {
	    message = target->getname() + " is bleeding!";
	  }
	}
      }

    }

    if (*dmg >= target->hp) {

      switch (bodypart) {
      case head: {
        if (dmgtype==1) {
	  float tty,ttx;
	  int dx=0;
	  int dy=0;
	  int range=(int)(6*mtrand() + 4);
	  while (abs(dx) == 0 && abs(dy) == 0) {
	    dx=(int)(7*mtrand() - 3);
	    dy=(int)(7*mtrand() - 3);
	  }
	  tty = (float)(y+dy);
	  ttx = (float)(x+dx);
	  message=target->getname() + "'s head flies off!";
	  los(map, y, x, &tty, &ttx, &range, bloodtrail, 0);
//	  vscreen->drawoutwards(y,x, range, ',', 3, map);
	  item = new Item(map, (int)tty, (int)ttx, ',', target->name + " head"); // Make item: Head!
	  item->color = 3; // Set its background to red.
	  item->setmass(5);
	}
        else {
	  message=target->getname() + "'s head explodes!";
	  for (i=0;i<8;i++) {
	    map->makebloody(y+(int)(3*mtrand() - 1), x+(int)(3*mtrand() - 1));
	  }
	}
        break;
      }
      default:
	break;
      }

    }
  }
  else {target->increasestat("toughness");}

  return message;

}
