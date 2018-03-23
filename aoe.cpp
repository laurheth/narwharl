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
#include <cmath>
#include "nheader.h"

using namespace std;

int lineofsight(Map *map, int y, int x, int ty, int tx, int viewdist)
{
  float i;
  int visibility;
  float dx,dy;
  float hypot;
  float ddx, ddy;
  int checkx,checky;
  visibility = 2*viewdist;

  dx = (float)tx-(float)x;
  dy = (float)ty-(float)y;

  hypot=sqrt((dx*dx + dy*dy));

  ddx=dx/hypot;
  ddy=dy/hypot;

  while(i<viewdist && visibility>0){
    checkx=(int)((float)x+(float)i * ddx +0.5);
    checky=(int)((float)y+(float)i * ddy +0.5);
    if (checkx==tx && checky==ty) {break;}
    if ((checkx<0 || checky<0) || (checkx>map->xsize-1 || checky>map->ysize-1)) {break;}
    visibility=visibility-map->getopacity(checky,checkx);
    i=i+1;
    if (map->seethrough(checky,checkx) == 0) {visibility=0;}
    else {
      if ((abs(ty-checky)<2 && (abs(tx-checkx)<2))) {
	return 1;
      }
    }
  }

  return 0;
}

void los(Map *map, int y, int x, int * ty, int * tx, int * dd, int (*aoefunc)(Map *, int, int, int),int jinit)
{
  float yy, xx, tyy, txx;
  tyy = (float)*ty;
  txx = (float)*tx;
  yy = (float)y;
  xx = (float)x;
  los(map, yy, xx, &tyy, &txx, dd, aoefunc, jinit);
  *ty = (int)(tyy+0.5);
  *tx = (int)(txx+0.5);
}

void los(Map *map, float y, float x, float * ty, float * tx, int * dd, int (*aoefunc)(Map *, int, int, int),int jinit)
{
  int i,j,cansee,d;
  float dx, dy;
  float ddx, ddy; // Unit vector in direction of ray
  float hypot; // hypotenuse of (y,x) -> (*ty,*tx)
  float checkx, checky; // Coordinate to check.

  dx = (*tx) - x; // Change in x to target
  dy = (*ty) - y; // Change in y to target
  
  hypot = sqrt((dx*dx + dy*dy)); // Distance to target
  ddx = (dx) / (hypot); // x component of unit vector towards target
  ddy = (dy) / (hypot); // y component

  j=0;
  
  d = *dd; // Double d for the possibility of lower than normal opacity.
  d = 2*d;
  cansee = d;
  (*tx)=x; // tx and ty contain the value of x and y, so they can be returned to the caller.
  (*ty)=y;
  //*dd=0;

  // j increases to max range, and cansee reduces according to visibility
  while (j<d && cansee > 0) {
    // Calculate location to check. the 0.5 puts the source in the middle of (x,y)
    checkx = x + (float)j * ddx;
    checky = y + (float)j * ddy;

    // Round it off.
    checkx = checkx + 0.5;
    checky = checky + 0.5;
    
    // Perform action on square action=0.
    // Also returns an opacity value. Default should be 2.
    // Opacity can be for visibility, or whatever else is relevant.
    // Maybe temperature for heat and cold attacks?
    int opacity;
    opacity = aoefunc(map,(int)checky,(int)checkx,0);
    cansee = cansee - opacity;

    // Check if the ray should keep going. action=1... but don't check until j is above jinit
    if (j >= jinit) {
      int keepgoing=aoefunc(map,(int)checky,(int)checkx,1);
      if ( keepgoing == 0 ) {
	*tx = (int)checkx;
	*ty = (int)checky;
	break;
      }
      else if ( keepgoing == 3) {j=-1;break;}
      else {
	*tx = (int)checkx;
	*ty = (int)checky;
      }
    }

    j++;

  }

  *dd=j;

  return;
    
}

void aoe(Map *map, int y, int x, int d, int (*aoefunc)(Map *,int, int, int))
{
  float yy, xx;
  yy = (float)y;// - 0.5;
  xx = (float)x;// - 0.5;
  aoe(map,yy,xx,d,aoefunc);
}

void aoe(Map *map, float y, float x, int d, int (*aoefunc)(Map *,int, int, int))
{
  int i,j,cansee,dd; //indices
  int dx,dy; //coordinates of outer box
  int px,py; //previous coordinates, to avoid double checking *unused*
  float tx,ty; // target for los()
  float ddx,ddy; //unit vector in distance of ray
  float hypot; //hypotenus of dx,dy
  float checkx, checky; //coordinate to check
  //d = 2*d;
  dx=d;
  dy=0;
  dd=d;
  for (i=0;i<8*d+1;i++) {
    // draw a box around the region
    if ((dx == d) && (dy < d)){dy++;}
    else if ((dy == d) && (dx > -d)){dx--;}
    else if ((dx == -d) && (dy > -d)){dy--;}
    else if ((dy == -d) && (dx < d)){dx++;}

    tx=x+(float)dx;
    ty=y+(float)dy;

    // Line of sight algorithm called towards each point around the box.
    los(map, (float)y, (float)x, &ty, &tx, &d, aoefunc,0);
    d=dd;
  }

}
