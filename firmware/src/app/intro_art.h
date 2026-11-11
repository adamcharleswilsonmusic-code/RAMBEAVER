/* SPDX-License-Identifier: GPL-3.0-only
 * Original forest walk and opening story, drawn at native resolution.
 * This renderer only reads intro state; it has no timers or frame buffers. */
#pragma once

static void intro_forest_walk(int x,int pose){
 rect(0,0,240,240,RGB(67,94,91));
 /* Keep the cloud below both story lines and the growing title. */
 mist(30,113,162,33,RGB(174,196,181));
 rect(0,190,240,50,RGB(58,86,54));
 for(int i=0;i<7;i++)ending_tree(0,8+i*35,58+(i%3)*12,132-(i%3)*12,45,0);
 rect(0,210,240,30,RGB(54,67,48));
 for(int i=0;i<15;i++){int xx=4+i*16,y=201+(i%3)*4;rect(xx,y,1,4,RGB(117,132,76));rect(xx+3,y+2,1,3,RGB(69,101,58));}
 ending_tree(0,10,23,184,61,1);ending_tree(0,230,27,173,55,1);
 ellipse(x-8,208,40,3,RGB(35,52,38));hero_beaver(x,178,1,pose);
}

static int intro_line_alpha(int tick){
 int t=tick%INTRO_LINE_TICKS;
 if(t<16)return t;
 if(t<116)return 16;
 if(t<140)return (140-t)*16/24;
 return 0;
}

/* Opening-only four-sided contour follows the same fade as the lettering. */
static void intro_story_line(int y,const char *s,int alpha){
 int x=(240-text_w(&FONT_M,s))/2;
 for(int dy=-1;dy<=1;dy++)for(int dx=-1;dx<=1;dx++)if(dx||dy)story_glyphs(x+dx,y+dy,&FONT_M,s,RGB(15,29,24),alpha);
 story_text(x,y,&FONT_M,s,CREAM,alpha);
}
static void intro_sentence(int tick){
 int line=imin(3,tick/INTRO_LINE_TICKS),alpha=intro_line_alpha(tick);
 static const char *const words[4][2]={{"AI has taken","over."},{"All humans","are gone."},{"The beavers","are next."},{"A hero has","been chosen."}};
 intro_story_line(48,words[line][0],alpha);intro_story_line(76,words[line][1],alpha);
}

static int intro_title_width(int tick){return 112+imin(INTRO_REVEAL_TICKS,imax(0,tick))*104/INTRO_REVEAL_TICKS;}
static void intro_title_glyphs(int x,int y,int width,uint16_t color,int opacity){
 if(opacity<=0)return;const char *s="RAMBEAVER";const felucca_font_t *f=&FONT_L;int sw=text_w(f,s),height=f->h*width/sw,cursor=0;
 for(;*s;s++){
  unsigned g=glyph(f,(uint8_t)*s),bw=f->bw[g],bpr=(bw+1)/2;const uint8_t *data=f->data+f->off[g];int left=(cursor-f->pad)*width/sw,right=(cursor-f->pad+(int)bw)*width/sw;
  for(int yy=0;yy<height;yy++)for(int xx=left;xx<right;xx++){
   int sx=(xx-left)*(int)bw/(right-left),sy=yy*f->h/height,px=x+xx,py=y+yy;int a=data[sy*bpr+sx/2];a=((sx&1)?a&15:a>>4)*opacity/16;if(!a||px<0||px>=240||py<0||py>=240)continue;
   uint16_t bg=swap16(cv_px[py*cv_w+px]);int r=((color>>11)*a+(bg>>11)*(15-a))/15,gc=(((color>>5)&63)*a+((bg>>5)&63)*(15-a))/15,b=((color&31)*a+(bg&31)*(15-a))/15;cv_pset(px,py,(uint16_t)(r<<11|gc<<5|b));
  }cursor+=f->adv[g];
 }
}
static void intro_overlay(void){
 int t=game.intro_tick;
 if(game.intro_phase==1){
  intro_forest_walk(40+imin(INTRO_STORY_TICKS,t)*92/INTRO_STORY_TICKS,BA_WALK_0+(t/4)%8);
  intro_sentence(t);
 }else if(game.intro_phase==3){
  intro_forest_walk(132+imin(INTRO_REVEAL_TICKS,t)*36/INTRO_REVEAL_TICKS,BA_WALK_0+((INTRO_STORY_TICKS+t)/4)%8);
  int alpha=t<16?t:t>INTRO_REVEAL_TICKS-24?(INTRO_REVEAL_TICKS-t)*16/24:16,width=intro_title_width(t),x=(240-width)/2,y=66-(FONT_L.h*width/text_w(&FONT_L,"RAMBEAVER"))/2;
  for(int dy=-1;dy<=1;dy++)for(int dx=-1;dx<=1;dx++)if(dx||dy)intro_title_glyphs(x+dx,y+dy,width,RGB(15,29,24),imin(16,imax(0,alpha)));
  intro_title_glyphs(x+2,y+2,width,RGB(15,29,24),imin(16,imax(0,alpha)));intro_title_glyphs(x,y,width,RGB(239,42,48),imin(16,imax(0,alpha)));
 }else if(game.intro_phase==2){
  intro_forest_walk(168+imin(INTRO_DEPART_TICKS,t)*94/INTRO_DEPART_TICKS,
    BA_WALK_0+((INTRO_STORY_TICKS+INTRO_REVEAL_TICKS+t)/4)%8);
 }
}
