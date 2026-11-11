/* SPDX-License-Identifier: GPL-3.0-only
 * Original support-surface shadow. Drawing never feeds back into physics. */
#pragma once
typedef struct {int surface,x,y,width,height,strength;} player_shadow_t;
static player_shadow_t player_shadow(void){
 int cx=game.x/Q8+PLAYER_W/2,feet=(game.y+PLAYER_H*Q8+Q8-1)/Q8;
 player_shadow_t s={-1,cx,10000,0,0,0};
 for(int i=0;i<terrain_count[game.scene];i++){
  const platform_t *p=terrains[game.scene]+i;
  if(game.drop_timer&&i==game.drop_platform)continue;
  if(cx>=p->x&&cx<p->x+p->w&&p->y>=feet&&p->y<s.y){s.surface=i;s.y=p->y;}
 }
 if(s.surface<0)return s;
 int rise=s.y-feet;
 s.width=imax(5,28-rise/5);s.height=rise>48?1:2;s.strength=imax(2,9-rise/18);
 return s;
}
static void player_shadow_draw(int cam){
 player_shadow_t s=player_shadow();if(s.surface<0)return;
 const platform_t *p=terrains[game.scene]+s.surface;
 int first=imax(p->x,s.x-s.width/2),last=imin(p->x+p->w,s.x+(s.width+1)/2);
 for(int yy=0;yy<s.height;yy++)for(int wx=first;wx<last;wx++){
  int x=wx-cam,y=s.y+yy;
  if(x<0||x>=(int)cv_w||y+cv_oy<cv_y0||y+cv_oy>=cv_y1)continue;
  // Darken the actual platform color instead of painting a hovering oval.
  uint16_t c=swap16(cv_px[(y+cv_oy)*cv_w+x]);int keep=16-s.strength;
  int r=(c>>11)*keep/16,g=((c>>5)&63)*keep/16,b=(c&31)*keep/16;
  cv_pset(x,y,(uint16_t)(r<<11|g<<5|b));
 }
}
