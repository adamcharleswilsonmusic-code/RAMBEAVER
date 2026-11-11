#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Original hand-authored indexed pixel sprites; zero is transparent."""
from pathlib import Path
R=Path(__file__).resolve().parents[1]
# 1 ink, 2 shadow fur, 3 grey, 4 light fur, 5 cream, 6 eye white,
# 7 coral, 8 copper, 9 blue steel, A teal, B gold, C steel highlight,
# D brown shadow, E warm brown fur, F golden fur highlight.
art={
'beaver':['               1111             ', '              1EFF11            ', '             1EFDFEF11          ', '             1EFDDFFFE111       ', '           1EFFFFFFFFFFFFE1     ', '          1EFFFFFFF161FFFE11    ', '      11EEEFFFFFFFFFFF555555E1  ', '     1EEEEFFFFFFFFFF55555555511 ', '    1EEEEFFFFFFFFFFF555555555111', '  1EEEEFFFFFFFFFFFFFF55555551   ', ' 1EEEEFFFEEEEEEEEEEEEE566561    ', '1DEEEEFFFEEEEEEEEEEEEE566561    ', '1DEEEEEFFFEEEEEEEEEEEE511511    ', '1DEEEEEFFFEEEEEEEEEEEEE11       ', '1DEEEEEFFFEEEEEEEEEEEEEE1       ', '1DEEEEEFFFEEEEEEEEDDEEEE1       ', ' 1DDEEEEFFFEEEE1DDD11111        ', ' 1DDDEEEEEEEEEE1DD1            ', '  1DDDEEEEEEEEEED11            ', '   11DDDEEEEEEED11            ', '     11DDDDDDD111             ', '       11111111               '],
'kit':['         111       ', '        1EF11      ', '        1EFFE111   ', '       1FFF1FFFE11 ', '      1FFFF1FFF5511', '    11FFFFF55555511', '   1EEFFFFF5555511 ', '  1EEEFFFFFF5611   ', ' 1EEEEFFFFFF561    ', '1DEEEEFFFFFFE11    ', '1DEEEEFFFFEEE1     ', '1DEEEEFFEEEE11     ', ' 1DDEEEEEEEE1      ', '  1DDDEEEEE11      ', '   11DDDDD11       ', '    1111111        '],
'bot':[
'            BB            ',
'            11            ',
'        1111111111        ',
'      119CCCCCC99111      ',
'     19CCCCCCCC999911     ',
'    19CCCCCCCCCC999911    ',
'    19C111111111111991    ',
'    19C1AAA111AAA11991    ',
'    19C1AAA111AAA11991    ',
'    19C111111111111991    ',
'    19CCCC1111CCC99991    ',
'    119CCC1111CC999911    ',
'     119CCCCCCC999911     ',
'      1119999991111       ',
'  111 19CCCCCC9911 111    ',
' 1999119C9BB9C991199911   ',
' 19C9119C9BB9C99119C991   ',
' 1999119CCCCCC991199911   ',
'  111 19C1111C9911 111    ',
'      19C1991C9911        ',
'      119999999111        ',
'       11999199111        ',
'       19911 19911        ',
'       19911 19911        ',
'      119911 199111       ',
'     19CCC11 19CCC11      ',
'     1111111 1111111      '],
'drone':[
'   111111            111111   ',
'  199CC991          199CC991  ',
' 1111111111        1111111111 ',
'      11              11     ',
'    1111111111111111111111   ',
'   199CCCCCCCCCCCCCCC999911  ',
'  19CCCCCCCCCCCCCCCCCC999911 ',
'  19C11111111111111111199991 ',
'  19C1AAA11111111AAA11999991 ',
'  19C1AAA11111111AAA11999991 ',
'  19C11111111111111111199991 ',
'   19CCCCCC7777CCCCCC999911  ',
'    199999111199999999911   ',
'     1111111111111111111    ',
'        117111111711       ',
'         1B1    1B1        ',
'          B      B         '],
'gun':['       11           ', '       1C1          ', '   111111111111     ', '  1CCCCCCCC9999111  ', '  1C999999991991CC1 ', '111999119991991111 ', '1DD199119999991    ', '11119911199111     ', '    191 1991       ', '    191 1991       ', '    111 1991       ', '         111       '],
}
lines=['/* Original authored indexed pixel artwork. */','#pragma once']
for name,rows in art.items():
 w=max(map(len,rows));h=len(rows);data=[int(ch,16) if ch!=' ' else 0 for row in rows for ch in row.ljust(w)]
 lines.append(f'#define SPR_{name.upper()}_W {w}\n#define SPR_{name.upper()}_H {h}')
 lines.append(f'static const uint8_t spr_{name}[{w*h}]={{'+','.join(map(str,data))+'};')
 (R/'assets/sprites'/f'{name}.txt').write_text('\n'.join(rows)+'\n')
(R/'firmware/src/app/sprites.h').write_text('\n'.join(lines)+'\n')
print('Authored original side-profile beaver, kits, ground robot, flying robot and compact gun sprites.')
