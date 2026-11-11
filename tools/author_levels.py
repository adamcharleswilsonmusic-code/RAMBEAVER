#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Authored region geometry and encounter placement, compiled to bounded C tables."""
from pathlib import Path
import json
ROOT=Path(__file__).resolve().parents[1]
# (x, floor y, length): short gaps and changing street/roof heights.
floors=[[(0,212,740),(800,194,660),(1520,212,760),(2340,186,690),(3090,212,510)],[(0,212,530),(590,180,590),(1240,212,700),(2000,192,720),(2780,212,520),(3360,176,480)],[(0,212,580),(640,184,630),(1330,212,640),(2030,170,690),(2780,212,580)],[(0,212,640)],[(0,212,320)],[(0,212,320)]]
ledges=[[(260,176,90),(380,144,94),(498,112,105),(610,156,92),(880,157,115),(1040,125,132),(1195,94,110),(1370,155,85),(1680,177,100),(1820,145,115),(1970,113,115),(2150,153,110),(2480,150,104),(2620,116,100),(2800,143,116),(3170,176,90),(3300,145,110)],[(180,174,108),(325,141,110),(680,143,114),(824,109,115),(1000,140,115),(1360,174,112),(1510,140,98),(1650,106,100),(1810,145,100),(2100,155,112),(2250,121,115),(2410,91,112),(2590,147,102),(2870,178,125),(3030,146,106),(3200,115,100),(3450,140,110),(3590,108,110)],[(165,177,110),(315,144,115),(750,146,115),(905,112,115),(1090,148,115),(1420,177,120),(1590,142,115),(1760,108,110),(1850,150,110),(2120,134,100),(2270,100,120),(2450,135,110),(2580,106,112),(2880,174,110),(3030,139,110),(3180,110,100)],[(120,178,76),(240,158,78)],[(110,170,75),(215,135,90)],[(100,170,72),(220,136,88)]]
scene_names=['OUTER CITY','INNER CITY','DATA CENTER','MOTHER MACHINE','SALVAGE DEN','MAINTENANCE CACHE']
# The humanoid guard is 32px tall. Low overhangs now sit 56px above their
# street, leaving 6px clear below the deepest 18px wall mount and 15px below
# the platform cap. Keep the route's floors and horizontal spans unchanged.
for scene,indices in [(0,[0,4,8,12]),(1,[0,2,5,9,13,16]),(2,[2,5,9,13])]:
 for i in indices:
  x,y,w=ledges[scene][i]
  floor=next(fy for fx,fy,fw in floors[scene] if fx<=x and x+w<=fx+fw)
  ledges[scene][i]=(x,floor-56,w)
# A nearby firing balcony frames the mother's core. Its approach retains a
# modest 20px rise and 42px horizontal gap; both remain clear of the machine.
ledges[3]=[(210,178,78),(314,158,80)]
# A last climb reaches the roof-mounted jetpack without changing the street.
ledges[1].append((3740,74,82))
# Flight owns its movement; this flat placeholder documents the same y208
# roof-contact boundary (the ordinary y212 ground renderer is bypassed).
floors.append([(0,212,3840)]);ledges.append([])
scene_names.append('ROOFTOP FLIGHT')
floors.append([(0,212,600),(640,200,510),(1190,212,510),(1740,212,230),(2070,212,330)])
ledges.append([(245,165,105),(390,137,100),(800,150,112),(960,122,100),(1360,150,115),(1510,140,105),(1970,212,100)])
scene_names.append('FOREST / RIVER WORKS')
widths=[3600,3840,3360,640,320,320]
widths.extend([3840,2400,2020])
floors.append([(0,212,2020)]);ledges.append([]);scene_names.append('CITY MOTORBIKE RUN')
# 0 = ground robot, 1 = flying automation, 2 = mother, 3 = spawned worker.
robots=[(0,410,180,0,3),(0,570,80,0,4),(0,960,162,0,4),(0,1215,62,0,4),(0,1770,180,0,4),(0,2030,81,0,4),(0,2530,154,0,5),(0,2860,111,0,4),(0,3350,180,0,5),
 (1,245,180,0,4),(1,455,124,1,3),(1,745,148,0,5),(1,1100,107,1,3),(1,1410,180,0,5),(1,1720,120,1,3),(1,2160,160,0,5),(1,2450,125,1,4),(1,2885,180,0,5),(1,3180,118,1,4),(1,3500,144,0,5),(1,3720,110,1,4),
 (2,330,112,0,5),(2,490,139,1,4),(2,835,152,0,5),(2,1120,116,0,5),(2,1480,180,0,5),(2,1750,124,1,4),(2,2140,138,0,5),(2,2420,132,1,4),(2,2590,74,0,5),(2,2930,180,0,5),(2,3150,118,1,4),(2,3250,78,0,5),
 (3,510,180,2,60),(3,470,180,3,0),(3,470,180,3,0),(3,470,180,3,0),(4,255,180,0,4),(5,265,180,0,5)]
def ground_y(scene,x):
 for a,y,w in floors[scene]:
  if a<=x<a+w:return y
 return 212
# Ready-to-throw bottles: deliberate street supplies plus optional rooftop caches.
# Each tuple is scene, x, y, reserved (zero), optional defeated-robot gate (-1).
pickups=[]
for scene,xs in [(0,[145,705,1040,1720,2240,2720,3380]),
                 (1,[125,500,960,1450,1900,2310,2950,3610]),
                 (2,[105,550,965,1435,1930,2360,2910,3305]),
                 (3,[100,255]),(4,[95,245]),(5,[100,255])]:
 for x in xs:pickups.append((scene,x,ground_y(scene,x)-9,0,-1))
for scene,j in [(0,4),(0,10),(1,3),(1,13),(2,6),(2,12)]:
 x,y,w=ledges[scene][j];pickups.append((scene,x+w//2,y-9,0,-1))
for x in [240,1080,1640]:pickups.append((7,x,ground_y(7,x)-9,0,-1))
checkpoints=[]
for scene,xs in [(0,[32,940,1700,2460,3200]),(1,[32,710,1440,2140,2890,3540]),(2,[32,790,1460,2130,2860]),(3,[32])]:
 for x in xs:checkpoints.append((scene,x,ground_y(scene,x)))
checkpoints.extend([(7,32,212),(7,1810,212),(7,2110,212),(8,58,212)])
lines=['/* Original authored Trash Panda region/encounter tables. */','#pragma once',f'#define NROB {len(robots)}',f'#define NSNACK {len(pickups)}','#define PICK_WORDS 7 /* Stable save footprint; v4 used 217 scrap bits. */',f'#define NCP {len(checkpoints)}','#define FOREST_SCENE 7','#define FOREST_START_CP 17','#define FOREST_DAM_CP 18','#define FOREST_EXIT_CP 19','#define BIKE_SCENE 8','#define BIKE_CP 20','#define INNER_START_CP 5','#define BOSS_CP 16','#define MOTHER 33','#define WORKER_FIRST 34','#define WORKER_LAST 37','#define JETPACK_X 3780','#define JETPACK_Y 74','typedef struct {int x,y,w;} platform_t;','typedef struct {int scene,x,y,type,hp;} robot_def_t;','typedef struct {int scene,x,y,reserved,drop;} snack_t;','typedef struct {int scene,x,y;} checkpoint_t;']
scenes=len(floors)
for i in range(scenes):lines.append(f'static const platform_t terrain_{i}[]={{'+','.join('{%d,%d,%d}'%t for t in floors[i]+ledges[i])+'};')
lines.append(f'static const platform_t *const terrains[{scenes}]={{'+','.join('terrain_'+str(i) for i in range(scenes))+'};')
lines.append(f'static const uint8_t terrain_count[{scenes}]={{'+','.join(str(len(floors[i]+ledges[i])) for i in range(scenes))+'};')
lines.append(f'static const uint8_t floor_count[{scenes}]={{'+','.join(str(len(f)) for f in floors)+'};')
lines.append(f'static const int scene_width[{scenes}]={{'+','.join(map(str,widths))+'};')
lines.append(f'static const char *const scene_names[{scenes}]={{'+','.join('"'+n+'"' for n in scene_names)+'};')
lines.append('static const robot_def_t robot_defs[NROB]={'+','.join('{%d,%d,%d,%d,%d}'%r for r in robots)+'};')
lines.append('static const snack_t snacks[NSNACK]={'+','.join('{%d,%d,%d,%d,%d}'%p for p in pickups)+'};')
lines.append('static const checkpoint_t checkpoints[NCP]={'+','.join('{%d,%d,%d}'%c for c in checkpoints)+'};')
(ROOT/'firmware/src/app/levels.h').write_text('\n'.join(lines)+'\n')
(ROOT/'assets/level-plan.json').write_text(json.dumps({'scene_names':scene_names,'widths':widths,'floors':floors,'ledges':ledges,'robots':robots,'pickups':pickups,'checkpoints':checkpoints},indent=2)+'\n')
print(f'Authored {sum(widths[:4])+sum(widths[6:])} px main route, {sum(map(len,ledges))} platforms, {len(robots)-3} fixed enemies, {len(pickups)} individual firebomb pickups.')
