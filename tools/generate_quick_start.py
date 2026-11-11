#!/usr/bin/env python3
# Release portability/documentation changes: 2026-10-17; firmware behavior and assets unchanged.
"""RAMBEAVER's one-page FM-1 guide, matching the Trumpagotchi quick-start style."""
from pathlib import Path
from reportlab.pdfgen import canvas
from reportlab.lib.colors import HexColor
from reportlab.lib.pagesizes import A4
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'docs/rambeaver-quick-start.pdf'
from reportlab.pdfbase.pdfmetrics import Font
pdfmetrics.registerFont(Font('Guide', 'Helvetica', 'WinAnsiEncoding'))
pdfmetrics.registerFont(Font('GuideBold', 'Helvetica-Bold', 'WinAnsiEncoding'))
W, H = A4
INK, MUTED, LINE = '#243540', '#5E6B70', '#D7DDD9'
PAPER, WARM, TEAL = '#FFFFFF', '#F6F1E7', '#087A76'
BLUE, MINT, PALEBLUE, RED = '#275E9A', '#E9F4F0', '#EBF1F9', '#B64238'
OUT.parent.mkdir(parents=True, exist_ok=True)
c = canvas.Canvas(str(OUT), pagesize=A4)
c.setTitle('RAMBEAVER - Quick Start for M-VAVE FM-1')
c.setAuthor('RAMBEAVER project')

def rect(x, y, w, h, fill, stroke=None, radius=0):
    c.setFillColor(HexColor(fill)); c.setStrokeColor(HexColor(stroke or fill)); c.setLineWidth(.8)
    if radius: c.roundRect(x, H-y-h, w, h, radius, fill=1, stroke=bool(stroke))
    else: c.rect(x, H-y-h, w, h, fill=1, stroke=bool(stroke))

def text(x, y, value, size=10, color=INK, bold=False, align='left'):
    c.setFillColor(HexColor(color)); c.setFont('GuideBold' if bold else 'Guide', size)
    yy = H-y-size*.85
    if align == 'center': c.drawCentredString(x, yy, value)
    elif align == 'right': c.drawRightString(x, yy, value)
    else: c.drawString(x, yy, value)

def line(x, y, xx, yy, color=LINE, width=.8):
    c.setStrokeColor(HexColor(color)); c.setLineWidth(width); c.line(x, H-y, xx, H-yy)

def dot(x, y, radius, fill):
    c.setFillColor(HexColor(fill)); c.circle(x, H-y, radius, fill=1, stroke=0)

def wrapped(x, y, value, width, size=9.5, color=INK, bold=False, leading=12):
    rows, current = [], ''
    font = 'GuideBold' if bold else 'Guide'
    for word in value.split():
        trial = (current + ' ' + word).strip()
        if current and pdfmetrics.stringWidth(trial, font, size) > width:
            rows.append(current); current = word
        else: current = trial
    if current: rows.append(current)
    for i, row in enumerate(rows): text(x, y+i*leading, row, size, color, bold)
    return len(rows)*leading

text(36, 29, 'RAMBEAVER', 29, RED, True)
text(36, 65, 'QUICK START  /  M-VAVE FM-1  /  BUILD 745', 12, MUTED, True)
text(36, 89, 'PLAY / STOP opens the menu. Choose New Run with SELECT, then press SEL.', 10)
line(36, 113, W-36, 113, TEAL, 2)

# Top-down device panel; key geometry follows the physical FM-1 layout.
x0, y0, scale = 36, 130, (W-72)/1000
def R(x, y, w, h, fill, stroke=None, radius=0): rect(x0+x*scale, y0+y*scale, w*scale, h*scale, fill, stroke, radius*scale)
def T(x, y, value, size=17, color=INK, bold=True): text(x0+x*scale, y0+y*scale, value, size*scale, color, bold, 'center')
def D(x, y, radius, fill): dot(x0+x*scale, y0+y*scale, radius*scale, fill)
def L(x, y, xx, yy, color=LINE, width=1.5): line(x0+x*scale, y0+y*scale, x0+xx*scale, y0+yy*scale, color, width*scale)
R(4, 4, 992, 632, WARM, '#B7B2A8', 38)
R(300, 44, 236, 236, INK, radius=23)
screen = ROOT / 'docs/assets/quickstart-screen.png'
if screen.exists():
    c.drawImage(str(screen), x0+310*scale, H-(y0+270*scale), width=216*scale, height=216*scale)
for x, y, label, active in [(105,100,'MASTER',True),(215,100,'SELECT',True),(105,205,'PRESET',False),(215,205,'ALGO',False)]:
    D(x,y,30,'#D6D2CA'); D(x,y,24,INK if active else '#B4B6B3')
    L(x,y-18,x,y-7,PAPER,3); T(x,y-51,label,16,INK if active else MUTED)
for i in range(4):
    x=610+i*104; D(x,100,27,'#D6D2CA'); D(x,100,22,'#B4B6B3'); L(x,84,x,94,PAPER,3); T(x,51,'K'+str(i+1),17,MUTED)
R(568,160,400,140,'#E7E1D7',radius=17)
for row, names in enumerate([['FX','SEL','ENV','LFO','EDIT','GLO'],['HOME','SAVE','ARP','SEQ','PLAY','REC']]):
    for i, label in enumerate(names):
        x,y=582+i*63,174+row*62; active=label in {'SEL','SAVE','PLAY'}
        R(x,y,54,48,TEAL if active else '#D5D1CA',radius=8)
        T(x+27,y+17,label,14,PAPER if active else MUTED)
        if label=='PLAY': T(x+27,y+33,'STOP',9,PAPER)
R(58,258,210,58,'#E7E1D7',radius=12)
for x,label in [(72,'OCT-'),(170,'OCT+')]: R(x,268,82,38,'#D5D1CA',radius=8); T(x+41,279,label,14,MUTED)
R(36,340,928,270,'#E7E1D7',radius=16)
pitch=(928-24)/16; kw=pitch*.78
white_labels={0:'1',1:'2',13:'3',14:'4',15:'5'}
for i in range(16):
    x=48+i*pitch+(pitch-kw)/2; active=i in white_labels
    R(x,456,kw,140,PALEBLUE if active else '#DBD9D2',BLUE if active else '#C7C5BF',kw/2)
    if active: T(x+kw/2,512,white_labels[i],29,BLUE)
for k, a in enumerate([0,1,2,4,5,7,8,9,11,12,14]):
    x=48+(a+1)*pitch-kw/2; label={0:'6',9:'7',10:'8'}.get(k)
    R(x,356,kw,89,TEAL if label else '#C7C5BF',radius=kw/2)
    if label: T(x+kw/2,383,label,29,PAPER)
T(500,323,'NUMBERED KEYS = GAME CONTROLS',18,TEAL)
T(500,615,'LEFT HAND: MOVE + DOORS     /     RIGHT HAND: ACTIONS',16,MUTED)

text(36, 475, 'LOWER / WHITE KEYS', 10, BLUE, True)
gap=7; cw=(W-72-4*gap)/5
for i,(n,label,desc) in enumerate([('1','LEFT','Hold to move.'),('2','RIGHT','Hold to move.'),('3','DUCK / DROP','Hold on ground.'),('4','JUMP','Press to jump.'),('5','FIRE','Hold to shoot.')]):
    x=36+i*(cw+gap); rect(x,492,cw,51,PALEBLUE,radius=7)
    dot(x+14,506,8,BLUE); text(x+14,501,n,9,PAPER,True,'center')
    text(x+27,500,label,8.7,BLUE,True)
    text(x+9,526,desc,8.6)
text(36, 550, 'UPPER / BLACK KEYS', 10, TEAL, True)
cw=(W-72-2*8)/3
for i,(n,label,desc) in enumerate([('6','DOOR','Enter / exit when the key lights.'),('7','POSSE','Clear visible robots; excludes Mother.'),('8','MOLOTOV','Throw a bottle. Carry up to three.')]):
    x=36+i*(cw+8); rect(x,567,cw,51,MINT,radius=7)
    dot(x+15,581,8,TEAL); text(x+15,576,n,9,PAPER,True,'center')
    text(x+29,575,label,10,TEAL,True)
    wrapped(x+10,595,desc,cw-20,8.7,leading=10)

col=(W-72-24)/2; right=36+col+24
text(36,630,'PAUSE, SOUND & SAVING',11.5,bold=True)
text(right,630,'STAY ALIVE & KEEP MOVING',11.5,bold=True)
y=650
for label,desc in [
    ('PLAY / STOP + SELECT / SEL','Pause, turn SELECT to choose, then press SEL. Play resumes.'),
    ('SAVE - THEN PAUSE BEFORE POWER-OFF','Live saves queue until Pause or death. Wait for PROGRESS SAVED before switching off.'),
    ('MASTER / MUSIC','MASTER sets overall volume. Music Off in the menu keeps voices and effects.')]:
    text(36,y,label,9,TEAL,True); y+=13
    y+=wrapped(36,y,desc,col,9.2,leading=11.5)+7
y=650
for label,desc in [
    ('DUCK, HEAL & RECHARGE','Hold 3 to duck: no moving or shooting. Tap 3 on a thin platform to drop. Logs heal; posse refills in 12 seconds.'),
    ('BIKE / JETPACK','Both move forward for you. Bike: smash robots; ramps launch you. Jetpack: hold Jump to rise; release to descend.'),
    ('BEAVER DOWN?','Jump or Fire retries. Continue loads your save; New Run starts over.')]:
    text(right,y,label,9,TEAL,True); y+=13
    y+=wrapped(right,y,desc,col,9.2,leading=11.5)+7
rect(36,786,W-72,20,MINT,radius=4)
wrapped(42,789,'GOLDEN BEAVER: Clear a fresh run with no damage or retries and every enemy defeated, including side rooms. Check Pause for flawless status. Jump / Fire advances the ending; press again to start a gold run. Save the unlock before power-off.',W-84,8,TEAL,leading=9)
line(36,810,W-36,810)
text(36,821,'Grey controls are unused. Capture: Follow FM-1 in Chrome; Record > Stop & Export saves video + USB audio to Desktop.',7.7,MUTED)
c.showPage(); c.save()
print(OUT)
