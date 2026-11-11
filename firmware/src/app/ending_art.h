/* SPDX-License-Identifier: GPL-3.0-only
 * Original native-resolution ending illustrations. No image/video buffers,
 * heap allocation, gameplay mutation, or external assets. */
#pragma once

static void ending_scene_props(int cam){
 if(game.scene!=3)return;
 int x=24-cam,open=game.ending_phase==2;
 /* One wall-mounted emergency exit, attached to the real floor at212. */
 rect(x-22,142,44,70,RGB(12,24,30));
 rect(x-20,144,40,68,open?RGB(75,113,98):RGB(59,76,81));
 rect(x-17,160,34,50,RGB(17,30,34));
 rect(x-17,144,34,14,RGB(21,36,39));
 label(x-12,145,"EXIT",open?MINT:RGB(148,157,151));
 int raised=open?imin(49,game.ending_tick*4):0;
 rect(x-15,162,30,49-raised,RGB(44,65,68));
 if(!open){
  for(int y=170;y<207;y+=10)rect(x-14,y,28,1,RGB(70,88,88));
  rect(x+11,150,3,3,CORAL);
 }else{
  rect(x-14,211-raised,28,raised,RGB(31,58,48));
  rect(x-11,211-raised,5,raised,RGB(25,45,38));
  rect(x+7,211-raised,3,raised,RGB(43,72,53));
  if(raised>30){rect(x-5,184,12,3,MINT);cv_line(x-5,185,x,180,MINT);cv_line(x-5,185,x,190,MINT);}
 }
 rect(x-22,210,44,2,open?RGB(119,160,124):RGB(73,91,89));
}

static int ending_slide(int tick){
 int remaining=imax(0,30-tick);
 return 240*remaining*remaining/900;
}

static void ending_city_tower(int px,int bx,int top,int width,int tick,int id){
 int collapse=imin(22,imax(0,tick-45-id*32)/5),roof=top+collapse;
 int x=px+bx;
 rect(x,roof,width,198-roof,RGB(23,34,44));
 rect(x+3,roof+3,width-6,195-roof,RGB(51,61,70));
 rect(x+3,roof+3,width-6,2,RGB(98,108,111));
 rect(x+width-7,roof+6,4,192-roof,RGB(35,45,55));
 /* Narrow optical strips make these buildings part of the robot city. */
 rect(x+7,roof+14,width-14,7,RGB(15,27,33));
 int live=tick<45+id*32;
 rect(x+10,roof+16,(width-25)/2,2,live?RGB(207,87,70):RGB(55,67,72));
 rect(x+width/2+4,roof+16,(width-25)/2,2,live?RGB(207,87,70):RGB(55,67,72));
 for(int y=roof+32,row=0;y<187;y+=13,row++)for(int col=0;col<(width-12)/11;col++){
  int wx=x+7+col*11;
  rect(wx,y,5,5,live&&(row+col+id)%3?RGB(107,122,113):RGB(34,48,58));
 }
 if(collapse){cv_line(x+width/2,roof+27,x+width/2-6,roof+48,INK);cv_line(x+width/2-6,roof+48,x+width/2+4,roof+72,INK);}
 rect(x+width/2-1,roof-13,2,13,RGB(80,91,96));
}

static void ending_city(int px,int tick){
 rect(px,0,240,240,RGB(29,28,43));
 rect(px,37,240,94,RGB(49,36,50));
 mist(px,39,159,33,RGB(73,43,62));mist(px+71,51,169,31,RGB(65,46,64));
 mist(px+9,57,149,28,RGB(53,45,61));mist(px+88,34,146,27,RGB(49,39,56));
 for(int i=0;i<7;i++){
  int x=px+i*36,top=93+(i*19)%31;
  rect(x,top,30,107-top,RGB(32,40,50));rect(x+3,top-6,24,7,RGB(32,40,50));
 }
 ending_city_tower(px,17,85,55,tick,0);
 ending_city_tower(px,91,58,69,tick,1);
 ending_city_tower(px,180,100,45,tick,2);
 /* Three one-shot ruptures spread across the skyline, never a screen flash. */
 static const int centers[3][3]={{46,115,34},{124,101,72},{204,137,110}};
 for(int i=0;i<3;i++){
  int t=tick-centers[i][2],cx=px+centers[i][0],cy=centers[i][1];
  if(t>=0&&t<29){
   int size=12+t*2;
   mist(cx-size/2,cy-size/2,size,size,RGB(198,73,52));
   mist(cx-size/3,cy-size/3,size*2/3,size*2/3,RGB(239,151,76));
   if(t<12)mist(cx-7,cy-7,14,14,RGB(255,214,134));
  }
  if(t>13){
   int rise=imin(45,(t-13)/2);
   mist(cx-14,cy-18-rise,28,20,RGB(72,70,75));
   mist(cx-8,cy-30-rise,23,19,RGB(58,60,68));
  }
  if(t>=0&&t<52)for(int j=0;j<5;j++){
   int dx=centers[i][0]+(j-2)*(8+t/4),yy=imin(197,cy-18+j*5+t*t/90);
   if(dx>2&&dx<234)rect(px+dx,yy,4,3,j&1?RGB(141,133,114):RGB(74,89,95));
  }
 }
 rect(px,198,240,42,RGB(26,32,39));rect(px,198,240,3,RGB(83,86,82));
 rect(px,209,240,2,RGB(48,57,58));
 for(int i=0;i<9;i++)rect(px+9+i*27,204,9,1,RGB(66,69,65));
 int runner=196-imin(150,tick)*148/150;
 ellipse(px+runner-3,207,35,3,RGB(13,22,28));
 hero_beaver(px+runner,177,-1,BA_WALK_0+(tick/3)%8);
}

static void ending_tree(int px,int x,int top,int height,int width,int layer){
 unsigned seed=hash((unsigned)x*43u+(unsigned)top*17u+(unsigned)height);int jitter=layer?0:(int)(seed%13)-6;
 woodland_tree(px+x+jitter,top,height,width,layer,seed);
}

static int ending_groove_pose(int offset){
 if(!game.music||game.pause)return BA_IDLE;
 unsigned beat=art_beat();
 if(!offset&&(art_music_step()&15u)<4u&&beat<16384u)return BA_GROOVE_ACCENT;
 return BA_GROOVE_0+(((beat>>14)+(unsigned)offset)&3u);
}

static void party_beaver(int x,int cy,int dir,int rot,int waterline,int scale,int pose){
 const uint8_t *data=beaver_anim[pose];
 for(int yy=0;yy<14;yy++)for(int xx=0;xx<32;xx++){
  int sy=8+yy*24/14,sx=xx*56/32;if(dir<0)sx=55-sx;int at=sy*56+sx,p=data[at/2];p=(at&1)?p&15:p>>4;if(!p)continue;
  int dx=(xx-16)*scale/32,dy=(yy-7)*scale/32,rx=rot==1?-dy:rot==2?-dx:rot==3?dy:dx,ry=rot==1?dx:rot==2?-dy:rot==3?-dx:dy;
  if(cy+ry<waterline)cv_pset(x+rx,cy+ry,natural_beaver_pal[p]);
 }
}
static void ending_pond(int px,int clock){
 /* Broad midground pond, behind the bank and full-size foreground hero. */
 mist(px+33,155,176,70,RGB(77,111,65));mist(px+42,162,155,55,RGB(57,119,131));mist(px+52,171,73,40,RGB(62,131,137));mist(px+133,177,65,32,RGB(62,131,137));mist(px+62,179,122,29,RGB(76,143,146));
 for(int n=0;n<6;n++){int x=px+66+(n*23+clock/5)%100;rect(x,172+(n%3)*9,11,1,RGB(148,192,174));}
 static const int dancers[8]={24,49,74,99,142,167,192,217};
 for(int i=0;i<8;i++)party_beaver(px+dancers[i],153+(i&1)*2-tri(clock/8+i*5,16)/6,i&1?-1:1,0,240,18+(i%3)*2,BA_GROOVE_0+((clock/16+i)&3));
 for(int i=0;i<6;i++){
  int full=clock+i*42,t=full%256,start=i&1?214:28,target=i&1?180-i/2*20:60+i/2*20,dir=i&1?-1:1,scale=22+(i&1)*2;
  if(t<32)party_beaver(px+start,178,dir,0,240,scale,BA_GROOVE_0+((clock/16+i)&3));
  else if(t<80){int u=t-32,x=start+(target-start)*u/48,cy=178-12*u*(48-u)/576+7*u/48,flip=(i==1&&(full/256%2==0))||(i==3&&(full/256%2==1));party_beaver(px+x,cy,dir,flip?u/12:0,240,scale,BA_APEX);}
  else{int waterline=186+(i&1)*8;
   if(t<96){int age=t-80;for(int n=0;n<6;n++){int x=px+target+(n-2)*(2+age/5),y=waterline-imax(0,12-iabs(age-6))-n%3;rect(x,y,2,2,CREAM);}party_beaver(px+target,imin(waterline-1,181+age*2),dir,0,waterline,scale,BA_APEX);}
   else{int x=42+tri((t-96)/2+i*37,320);ellipse(px+x-12,waterline-1,24,3,RGB(161,204,183));party_beaver(px+x,waterline-1,dir,0,waterline,scale,BA_APEX);rect(px+x-15,waterline+1,8,1,RGB(171,208,184));}
  }
 }
}
static void ending_front_friends(int px,int clock){
 static const int x[4]={28,63,181,217};for(int i=0;i<4;i++)party_beaver(px+x[i],211+(i&1)*8-tri(clock/6+i*7,20)/6,i&1?-1:1,0,240,28+(i&1)*4,BA_GROOVE_0+((clock/16+i)&3));
}
static void ending_party_line(int y,const char *text,int offset){
 int t=game.party_tick+offset;uint16_t c=RGB(180+tri(t,384)*75/192,180+tri(t+128,384)*75/192,170+tri(t+256,384)*85/192);
 story_text((240-text_w(&FONT_M,text))/2,y,&FONT_M,text,c,16);
}
static void ending_confetti(int px,int clock){
 static const uint16_t colors[5]={MINT,CORAL,GOLD,BLUE,CREAM};
 for(int i=0;i<18;i++){int x=(i*59+clock*(1+i%2)/4)%240,y=5+(clock/3+i*29)%230;
  if((x>7&&x<233&&y>21&&y<154)||(x>83&&x<153&&y>166&&y<218)||(x>18&&x<78&&y>167&&y<198)||(x>159&&x<219&&y>167&&y<198))continue;
  rect(px+x+tri(clock/7+i*9,20)/4,y,(clock/8+i)&1?3:1,(clock/8+i)&1?1:4,colors[i%5]);
 }
}
static void ending_forest(int px,int tick,int done){
 rect(px,0,240,240,RGB(144,184,193));
 rect(px,120,240,45,RGB(104,134,99));
 rect(px,165,240,75,RGB(54,88,57));
 for(int i=0;i<7;i++)ending_tree(px,10+i*35,61+(i%3)*10,87-(i%3)*10,43,0);
 rect(px+48,177,144,2,RGB(98,139,114));
 ending_tree(px,9,29,176,67,1);ending_tree(px,226,26,180,65,1);
 rect(px,213,240,27,RGB(41,69,45));
 for(int i=0;i<19;i++){
  int x=5+i*12,y=200+(i*7)%16;
  rect(px+x,y,1,4,RGB(119,134,75));rect(px+x+2,y+2,1,3,RGB(69,102,59));
 }
 ending_pond(px,game.party_tick);
 for(int x=0;x<240;x++){int mound=imax(0,5-iabs(x-115)/9),shore=215+tri(x+17,126)/16-mound;rect(px+x,shore,1,240-shore,RGB(47,77,47));}
 for(int i=0;i<15;i++){int x=px+4+i*17;rect(x,217+(i%3)*6,1,4,RGB(110,133,72));}
 ending_front_friends(px,game.party_tick);
 int hero=29+imin(60,tick)*83/60;ellipse(px+hero-7,212,40,3,RGB(34,57,38));hero_beaver(px+hero,181,1,tick<60?BA_WALK_0+(tick/3)%8:ending_groove_pose(0));
 ending_confetti(px,game.party_tick);
 /* Sparse fireflies drift gently; no rapidly blinking celebration effects. */
 for(int i=0;i<5;i++){
  int x=45+i*34+tri(tick/8+i*5,12)/3,y=112+(i*19)%52-tri(tick/12+i*3,10)/3;
  rect(px+x,y,2,2,RGB(198,196,112));
 }
}

static void ending_overlay(void){
 int phase=game.ending_phase,tick=game.ending_tick;
 if(phase==3){
  ending_city(ending_slide(tick),tick);
 }else if(phase==4){
  int slide=ending_slide(tick);
  if(slide)ending_city(0,170);
  ending_forest(slide,tick,0);
 }else if(phase==5){
  if(game.gold_reveal==1){
   /* Exclusive earned-unlock page: cover the entire world/HUD and use only
    * these two lines. Confirm advances; no countdown or restart prompt. */
   rect(0,0,240,240,INK);
   const char *title="FLAWLESS RUN",*reward="GOLDEN BEAVER UNLOCKED";
   cv_text((240-text_w(&FONT_M,title))/2,48,&FONT_M,title,GOLD);
   biglabel((240-text_w(&FONT_S,reward))/2,87,reward,CREAM);
   ellipse(91,199,58,3,RGB(36,35,31));hero_beaver(115,167,1,BA_IDLE);
   return;
  }
  ending_forest(0,170,1);
  ending_party_line(24,"AI HAS BEEN",0);ending_party_line(52,"DESTROYED",64);
  ending_party_line(91,"THE BEAVERS NOW",128);ending_party_line(119,"RULE THE PLANET",192);
 }
}
