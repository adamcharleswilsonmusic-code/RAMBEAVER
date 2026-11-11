# SPDX-License-Identifier: GPL-3.0-only
"""Source-hook house zones, forest intro, flight and a celebratory victory."""
from pathlib import Path
import json
R = Path(__file__).resolve().parents[1]
a = json.loads((R/'music/reanalysis.json').read_text())
events = []
slots = {}
def add(zone, tick, channel, note, instrument, length, level, replace=False):
    key = (zone, tick % 128, channel)
    event = [tick % 128, channel, note, instrument, length, level, 1 << zone]
    if key in slots and not replace:
        assert slots[key] == event, ('Conflicting event', key)
    slots[key] = event
# Keep every original bass attack and the alternating rest. Additional notes
# are syncopated reattacks of that same source pitch, never a generic scale run.
for zone in range(4):
    for bar in range(8):
        base = bar * 16
        build = bar in [2, 3, 6, 7]
        for q, note in enumerate([27, 27 if bar % 2 == 0 else None, 34, 30]):
            if note is None:
                continue
            t = (base + q * 4 - 2) % 128
            short = zone in [1, 3] or (build and q in [0, 2])
            add(zone, t, 1, note, 1, 1 if short else 2, [74, 67, 72, 68][q])
            if (zone == 0 and build and q == 2) or (zone == 1 and q in [0, 2]) or (zone == 2 and build and q == 0) or (zone == 3 and q in [0, 2]):
                add(zone, t + 2, 1, note, 1, 1, 44 if zone < 3 else 49)
            if bar in [1, 5] and q >= 1:
                continue
            if zone == 0:
                add(zone, t, 0, note + 24, 0, 1 if build else 2, 42)
                if build and q in [0, 2]:add(zone, t + 2, 0, note + 24, 7, 1, 27)
            elif zone == 1:
                add(zone, t + 1, 0, note + 24, 0, 1, 47)
                if build and q in [0, 2]:add(zone, t + 3, 0, note + 24, 7, 1, 31)
            elif zone == 2:
                add(zone, t, 0, note + (36 if q in [1, 3] else 24), 0, 1, 42)
                if build and q in [0, 2]:add(zone, t + 2, 0, note + 36, 7, 1, 30)
            else:
                add(zone, t, 0, note + 24, 0, 1, 47)
                if q in [0, 2]:add(zone, t + 1, 0, note + 24, 7, 1, 33)
                if build and q == 3:add(zone, t + 2, 0, note + 24, 7, 1, 30)
        # The recognisable response retains the previously analysed notes.
        if bar in [1, 5]:
            for dt, note, length in [(4, 54, 2), (7, 56, 4), (13, 53, 1), (14, 56, 2)]:
                add(zone, base + dt - 2, 0, note + (12 if zone == 2 else 0), 0,
                    min(length, 2) if zone in [1, 3] else length, 48)
        # Four-floor pulse stays steady; phrase-end pickups, hats and clap
        # rolls create an eight-bar rise/answer/turnaround rather than a flat bar.
        for q in range(4):
            add(zone, base + q * 4, 2, 32, 3, 1, 76 if q == 0 else 72)
        if bar in [3, 7]:add(zone, base + (14 if zone in [1, 3] else 15), 2, 32, 3, 1, 42)
        for dt in [4, 12]:add(zone, base + dt, 3, 67, 4, 1, 53 if dt == 4 else 58)
        for dt in [2, 6, 10, 14]:add(zone, base + dt, 3, 78, 6, 1, [33, 39, 34, 42][dt // 4])
        ghosts = [7, 15] if zone == 0 else [1, 7, 9, 15] if zone == 1 else [3, 7, 11, 15] if zone == 2 else [1, 3, 7, 9, 11, 15]
        if build:ghosts = sorted(set(ghosts + [1, 3, 9, 11]))
        for dt in ghosts:add(zone, base + dt, 3, 80, 5, 1, 18 if dt % 4 == 1 else 24)
        if bar in [3, 7]:
            # One percussion voice: short rolls intentionally replace selected
            # hat ticks; no extra mixer voices or source-recording samples.
            for dt, level in ([(13, 24), (15, 37)] if zone in [0, 2] else [(13, 22), (14, 29), (15, 39)]):
                add(zone, base + dt, 3, 67, 8, 1, level, replace=True)
# Mother-only finishing pass: a tighter sixteenth bass pocket, extra kick
# pickups, syncopated octave replies and expanding percussion turnarounds.
# No other variant or mixer voice allocation is changed.
for bar in range(8):
    base=bar*16
    build=bar in [2,3,6,7]
    for q,note in [(0,27),(2,34)]:
        tick=(base+q*4-2)%128
        add(3,tick+3,1,note,1,1,52)
        add(3,tick+2,0,note+(36 if build else 24),7,1,34,replace=True)
    for dt in ([6,10] if build else [10]):add(3,base+dt,2,32,3,1,48)
    for dt in [5,13]:add(3,base+dt,3,80,5,1,24,replace=True)
    if bar in [3,7]:
        for dt,level in [(11,26),(13,31),(14,36),(15,44)]:
            add(3,base+dt,3,67,8,1,level,replace=True)
# Flight keeps the source's28 bass attacks and alternating rest, but gives
# the high lead an anticipated/answered shape over a tighter bass pocket.
# The existing variants above are deliberately untouched.
for bar in range(8):
    base = bar * 16
    build = bar in [2, 3, 6, 7]
    for q, note in enumerate([27, 27 if bar % 2 == 0 else None, 34, 30]):
        if note is None:continue
        tick = (base + q * 4 - 2) % 128
        add(5, tick, 1, note, 1, 1, [74, 67, 72, 68][q])
        if q in [0, 2]:add(5, tick + 3, 1, note, 1, 1, 46)
        if bar in [1, 5] and q >= 1:continue
        lead_tick = (tick + 1) % 128
        # Close the final anticipation inside the loop, so the first pass
        # and every repeated pass begin with the same voice state.
        add(5, lead_tick, 0, note + (36 if q in [0, 2] else 24), 0,
            2 if q == 0 and lead_tick != 127 else 1, 44)
        if build and q in [0, 2]:add(5, tick + 3, 0, note + 24, 7, 1, 31)
    if bar in [1, 5]:
        for dt, note, length in [(4, 54, 2), (7, 56, 4), (13, 53, 1), (14, 56, 2)]:
            add(5, base + dt - 2, 0, note + 12, 0, min(2, length), 46)
    for q in range(4):add(5, base + q * 4, 2, 32, 3, 1, 76 if q == 0 else 72)
    if bar in [3, 7]:add(5, base + 14, 2, 32, 3, 1, 42)
    for dt in [4, 12]:add(5, base + dt, 3, 67, 4, 1, 53 if dt == 4 else 58)
    for dt in [2, 6, 10, 14]:add(5, base + dt, 3, 78, 6, 1, [38, 44, 40, 46][dt // 4])
    for dt in ([1, 3, 5, 7, 9, 11, 13, 15] if build else [1, 3, 7, 9, 11, 15]):
        add(5, base + dt, 3, 80, 5, 1, 18 if dt % 4 == 1 else 24)
    if bar in [3, 7]:
        for dt, level in [(13, 24), (14, 32), (15, 42)]:add(5, base + dt, 3, 67, 8, 1, level, replace=True)
# Intro-only variant: long low tones and separated source-note questions.
# Its own envelopes and absence of drums change the feel without merely
# reducing the game groove's volume. The four playable-zone events above
# remain unchanged; only this fifth mask uses the new soft instruments.
for tick, note in [(0, 39), (32, 39), (64, 46), (96, 42)]:
    add(4, tick, 1, note, 10, 30, 42)
for tick, note in [(0, 51), (32, 54), (64, 58), (96, 54)]:
    add(4, tick, 2, note, 11, 28, 26)
for tick, note, length, level in [(2, 63, 8, 40), (14, 58, 10, 30),
        (35, 66, 8, 37), (51, 68, 6, 28), (66, 65, 8, 34),
        (80, 68, 10, 30), (94, 66, 10, 35), (112, 63, 12, 39)]:
    add(4, tick, 0, note, 9, length, level)
# Celebration mode6 is the separate official-melody/original-accompaniment
# O Canada score generated by music/arrange_anthem.py.
# Forest pulse: sustained woodland harmony over syncopated source bass.
# Defense raises the rhythmic density and bass articulation, keeping the
# same theme/pad/call rather than importing a new composition or recording.
for bar in range(8):
    base=bar*16;bass=[39,39,46,42,39,39,46,42][bar]
    for dt,length,level in [(0,5,53),(6,2,47),(10,3,51),(14,2,42)]:
        add(7,base+dt,1,bass,10,length,level)
    add(7,base+1,2,bass+12,11,14,25)
    for dt,note in [(2,bass+24),(9,bass+31),(13,bass+28)]:
        add(7,base+dt,0,note,9,4 if dt==2 else 3,45 if dt==2 else 36)
    for dt in [0,8]:add(7,base+dt,3,32,3,1,53)
    for dt in [4,12]:add(7,base+dt,3,67,8,1,37)
    for dt in [2,6,10,14]:add(7,base+dt,3,78,6,1,25)
    for dt,note in [(0,bass),(3,bass+12),(6,bass),(8,bass),(11,bass+12),(14,bass)]:
        add(8,base+dt,1,note,1,2,57 if dt in [0,8] else 48)
    add(8,base+1,2,bass+12,11,14,26)
    for dt,note in [(2,bass+24),(5,bass+27),(9,bass+31),(13,bass+24)]:
        add(8,base+dt,0,note,9,2,50 if dt in [2,9] else 39)
    for dt in [0,4,8,12]:add(8,base+dt,3,32,3,1,68 if dt in [0,8] else 59)
    for dt in [2,6,10,14]:add(8,base+dt,3,78,6,1,34)
events = list(slots.values())
# Each channel is monophonic; reject accidental competing notes at one tick.
keys = [(e[6], e[0], e[1]) for e in events]
assert len(keys) == len(set(keys)), 'Conflicting events within a zone/channel'
# Merge common material so the arrangements still use a small event table.
merged = {}
for event in events:
    key = tuple(event[:6]); merged[key] = merged.get(key, 0) | event[6]
events = [list(key) + [mask] for key, mask in merged.items()]
events.sort(key=lambda e: (e[0], e[1], e[6]))
rows = ['/* Source-hook house arrangements; procedural synth, no source PCM. */',
        '#pragma once', '#define MUSIC_BPM 120u', '#define MUSIC_TICKS 128u',
        '#define MUSIC_LOOP_FRAMES 705600u',
        'typedef struct {uint8_t tick,channel,note,instrument,length,level;uint16_t mask;} music_event_t;',
        f'#define MUSIC_EVENTS {len(events)}', 'static const music_event_t music_events[MUSIC_EVENTS]={']
rows += ['{' + ','.join(map(str, event)) + '},' for event in events]
rows += ['};']
(R/'firmware/src/dsp/music_score.h').write_text('\n'.join(rows) + '\n')
record = {
    'revision': 'FM-1_733', 'source_start': a['start'] + a['beat']/2,
    'source_end': a['start'] + a['beat']/2 + 32*a['beat'], 'bpm': 120,
    'events': events, 'event_bytes': len(events)*8,
    'basis': 'Source Eb1/Eb1-or-rest/Bb1/Gb1 hook and harmonic rhythm retained. Four-on-the-floor kick with phrase-end pickups, clearer offbeat hats, backbeat procedural clap and turnaround rolls, same-note bass syncopation and source-note response. Bars 3/4 and 7/8 build rhythmic density; the loop resolves back to the opening pocket. No generic scale arpeggio or imported samples.',
    'zones': {'8': 'Dam defense: four-on-the-floor drive, syncopated sub bass and denser woodland-theme calls.', '7': 'Forest: grooving source-tone triangle calls, syncopated sine bass/pad, kick/clap/offbeat hats; eight voices total including effects.', '0': 'Outskirts: audible saw/sub bass, full warm hook, laid-back echoes and phrase-ending clap pickups.',
              '1': 'Inner city: short bass reattacks, delayed filtered PWM stabs, shuffled echoes and denser turnaround rolls.',
              '2': 'Data center: resonant sync-oscillator calls and octave answers from source notes, growing sixteenth hats and late kick pickups.',
              '3': 'Mother: sixteenth source-bass pickups, syncopated octave replies, extra kick attacks and building percussion turnarounds.',
              '4': 'Forest intro: sparse source-note questions, long sine bass and softly rising triangle pad; no kick, hats or clap.',
              '6': 'Victory: Eb-major triangle melody and answering keys, warm bass and light dance percussion through the entire ending.',
              '5': 'Jetpack flight: bright sync-oscillator hook anticipations and lower octave answers, tight same-note bass pickups, propulsive hats and rising turnaround rolls.'},
    'limits': 'The note reduction came from mixed-recording inference; source recognizability still needs listener judgement. Arrangement is now house-oriented rather than constrained to chiptune.',
    'voices': '4 music + 4 SFX; same four monophonic music channels in every zone.',
    'mix_headroom': 'Music-only interpolated ducking during multiple simultaneous combat effects; quick recovery, no SFX gain changes.',
    'victory': 'Mother defeat fades background to silence; native explosions accompany shutdown and a bounded descending alarm accompanies indoor escape. On exit, mode6 starts and persists through city/forest/party; phase-aware Continue, mute and pause remain coherent.',
    'transitions': 'Normal variant requests 0..6 commit on bar boundaries without resetting the sample clock or score cursor; variant4 is New Run intro and5 is flight. Playable intro arrival waits for the queued0 commit when music is enabled; roof/drop requests2.',
    'intro': '16 extra events use original soft triangle/sine instruments and long attack envelopes; intro material unchanged from725; Mother-only intensity changes are recorded separately.',
    'flight': 'Retains all28 source-bass attacks; original rhythmic/articulation variation using the same native synth and4 music plus4SFX budget. Flight events are unchanged from725.',
    'boss_finish': 'Mother-only extra kick pickups and sixteenth bass/lead responses; all other five variants unchanged from725.',
    'energy_revision': 'Faster kick pitch drop and short attack noise, lower/clearer hat band, brighter bass upper harmonics and moving synth filter envelopes; not merely master gain.',
    'provenance': 'Original project arrangement/generator and existing GPL Felucca synth engines; user-supplied composition rights remain separate.'}
(R/'music/revised-arrangement.json').write_text(json.dumps(record, indent=2) + '\n')
print(f'Four-zone house, forest intro, flight and victory score: {len(events)} events, {len(events)*8} bytes.')
