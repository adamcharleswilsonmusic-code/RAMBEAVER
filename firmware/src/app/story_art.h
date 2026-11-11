/* SPDX-License-Identifier: GPL-3.0-only
 * Original direct RGB565 story lettering; bounded glyph reads, no bitmap buffers. */
#pragma once
static void story_glyphs(int x,int y,const felucca_font_t *f,const char *s,uint16_t c,int opacity){
 if(opacity<=0)return;
 for(;*s;s++){
  unsigned gi=glyph(f,(uint8_t)*s),w=f->bw[gi],bpr=(w+1)/2;const uint8_t *data=f->data+f->off[gi];
  for(unsigned yy=0;yy<f->h;yy++)for(unsigned xx=0;xx<w;xx++){
   int px=x-f->pad+(int)xx,py=y+(int)yy+cv_oy;
   if(px<0||px>=(int)cv_w||py<cv_y0||py>=cv_y1)continue;
   int a=data[yy*bpr+xx/2];a=((xx&1)?a&15:a>>4)*opacity/16;if(!a)continue;
   uint16_t bg=swap16(cv_px[py*cv_w+px]);
   int r=((c>>11)*a+(bg>>11)*(15-a))/15,g=(((c>>5)&63)*a+((bg>>5)&63)*(15-a))/15,b=((c&31)*a+(bg&31)*(15-a))/15;
   cv_px[py*cv_w+px]=swap16((r<<11)|(g<<5)|b);
  }x+=f->adv[gi];
 }
}
static void story_text(int x,int y,const felucca_font_t *f,const char *s,uint16_t c,int alpha){
 story_glyphs(x+2,y+2,f,s,RGB(15,29,24),alpha);story_glyphs(x,y,f,s,c,alpha);
}
static void story_line(int y,const char *s,int alpha){story_text((240-text_w(&FONT_M,s))/2,y,&FONT_M,s,CREAM,alpha);}
