/* SPDX-License-Identifier: GPL-3.0-only
 * Original simple, soft woodland silhouettes. Immutable world seeds vary
 * broad foliage masses, proportions and roots. Fixed six-lobe maximum;
 * no outlines, per-leaf noise, heap, particles or mutable draw state. */
#pragma once
static void woodland_tree(int x,int top,int height,int width,int near,unsigned seed){
 if(x+width<0||x-width>(int)cv_w||height<24||width<12)return;
 unsigned h=hash(seed+613u);int base=top+height,lean=(int)(h>>8&7)-3,tw=imax(3,width/10),fork=top+height/3,crown=height*(h%3==0?73:61)/100;
 uint16_t bark=near?RGB(91,75,48):RGB(119,133,101),lit=near?RGB(129,108,70):RGB(142,151,115),shade=near?RGB(49,73,43):RGB(92,123,88);
 for(int y=fork;y<base;y++){int w=1+(y-fork)*tw/(base-fork),cx=x+lean*(base-y)/height;rect(cx-w/2,y,w,1,bark);rect(cx-w/2,y,1,1,lit);}
 cv_line(x,base-5,x-tw-4,base-1,bark);cv_line(x,base-5,x+tw+5,base-1,bark);
 cv_line(x,base-height/3,x-width/4,top+crown/2,lit);cv_line(x,base-height/3,x+width/4,top+crown/3,bark);
 uint16_t deep=near?RGB(44,79,45):RGB(91,125,89),mid=near?RGB(61,99,52):RGB(111,143,99),light=near?RGB(82,116,61):RGB(129,156,108);
 /* Broad overlapping lobes fuse into one asymmetric soft crown. A few
  * generous light planes replace the former twig/leaf speckle texture. */
 for(int n=0;n<6;n++){
  unsigned a=hash(h+(unsigned)n*173u);int bw=width*(65+(int)(a%26))/100,bh=imax(12,crown*(50+(int)(a>>8&15))/100),dx=(int)(a>>16)%(width/2+1)-width/4,dy=(int)(a>>24)%imax(1,crown-bh);
  if(h%3==0){dy=n*crown/8;bw=width*(48+n*9)/100;bh=imax(12,crown/3);dx=lean+(n&1?width/10:-width/10);}
  mist(x+dx-bw/2,top+dy,bw,bh,n<3?deep:mid);
  if(n==1||n==4)mist(x+dx-bw/2+2,top+dy+1,bw*3/4,bh/2,light);
 }
 ellipse(x-tw-2,base-1,tw*2+5,2,shade);
}
