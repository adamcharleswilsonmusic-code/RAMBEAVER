#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Deterministically reduce the original ImageGen cutout, retaining alpha.
The unchanged source and generation prompt are in assets/generated/.
No external wildlife photograph or sprite is used.
"""
from pathlib import Path
from PIL import Image
import hashlib,json
R=Path(__file__).resolve().parents[1]
src=R/'assets/generated/beaver-natural-source.png'
im=Image.open(src).convert('RGBA'); alpha=im.getchannel('A')
assert alpha.getextrema()==(0,255), 'A genuinely transparent source is required'
bounds=alpha.point(lambda a:255 if a>64 else 0).getbbox()
im=im.crop(bounds)
lines=['/* Original ImageGen-derived animal; see assets/generated/provenance.json. */','#pragma once']
previews=[]
for name,width in [('natural_beaver',50),('natural_kit',29)]:
 h=round(im.height*width/im.width);small=im.resize((width,h),Image.Resampling.LANCZOS)
 q=small.convert('RGB').quantize(colors=15,method=Image.Quantize.MEDIANCUT,dither=Image.Dither.NONE)
 pal=q.getpalette()[:45];a=small.getchannel('A');data=[v+1 if av>=96 else 0 for v,av in zip(q.getdata(),a.getdata())]
 colors=[(0,0,0)]+[tuple(pal[i:i+3]) for i in range(0,45,3)]
 lines += [f'#define {name.upper()}_W {width}',f'#define {name.upper()}_H {h}',f'static const uint16_t {name}_pal[16]={{'+','.join('RGB(%d,%d,%d)'%rgb for rgb in colors)+'};',f'static const uint8_t {name}[{width*h}]={{'+','.join(map(str,data))+'};']
 out=Image.new('RGBA',(width,h));out.putdata([(*colors[v],255 if v else 0) for v in data]);out.save(R/f'assets/generated/{name}.png');previews.append(out)
(R/'firmware/src/app/beaver_bitmap.h').write_text('\n'.join(lines)+'\n')
meta={'generated_with':'OpenAI built-in image_gen','date':'2026-10-13','source':src.name,'source_sha256':hashlib.sha256(src.read_bytes()).hexdigest(),'prompt':'beaver-prompt.txt','source_mode':'RGBA','source_dimensions':Image.open(src).size,'alpha_extrema':alpha.getextrema(),'crop':bounds,'derivation':'tools/reduce_beaver.py: alpha bounding crop, aspect-preserving Lanczos reduction, 15 RGB colors, transparent index 0. Original full alpha remains in source PNG.','external_image_inputs':[],'sprites':[(p.width,p.height) for p in previews]}
(R/'assets/generated/provenance.json').write_text(json.dumps(meta,indent=2)+'\n')
canvas=Image.new('RGB',(640,240),(30,38,48))
for i,p in enumerate(previews):canvas.paste(p.resize((p.width*8,p.height*8),Image.Resampling.NEAREST),(15+i*370,35),p.resize((p.width*8,p.height*8),Image.Resampling.NEAREST))
canvas.save(R/'build/test/natural-beaver-sprites.png')
print(json.dumps(meta))
