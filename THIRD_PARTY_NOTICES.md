# Component notices

| Component | Attribution | License / retained evidence |
| --- | --- | --- |
| FM-1 HAL, startup, loader, USB/audio, display, storage, MIDI, packaging, updater | Felucca by Leo Kuroshita (@kurogedelic), Hügelton Instruments; X0X/FoMni adaptations by Charles Vestal | GPL-3.0-only; source headers, LICENSE, provenance/upstream |
| ANALOG/NOISE synthesis, shared DSP helpers and lookup generator | Felucca, pinned tree1727537624390cc3f9c7dff2456b43b3a8edb005 | GPL-3.0-only; original headers and retained upstream excerpts |
| RAMBEAVER game logic, code drawings, score adapters, tests and recorder | Original project work based on the components above | GPL-3.0-only source code |
| Terminus bitmap fonts | Dimitar Toshkov Zhekov | SIL OFL 1.1; assets/fonts/Terminus-LICENSE.txt |
| Barlow Semi Condensed resources | The Barlow Project Authors | SIL OFL 1.1; assets/fonts/Barlow-OFL.txt |
| Three JieLi AC79 package inputs | JieLi AC79 SDK commitd179b4484759423312073f5fbb232501aa491047 | Apache-2.0 attribution from pinned SDK root license and upstream project credits; vendor/ac79/NOTICE.md |
| Browser runtime | Emscripten 4.0.23, musl, compiler-rt | LICENSES/Emscripten-LICENSE.txt, musl-COPYRIGHT.txt, compiler-rt-LICENSE.txt |

Felucca → X0X → FoMni is the platform lineage. RAMBEAVER uses only the retained ANALOG/NOISE DSP excerpts, not Felucca's other synth engines or sample libraries. No Chordian data, externally downloaded rescue loader, toolchain, full SDK, or system framework binary is distributed here.

The three SDK inputs are opaque vendor files, reproduced byte-for-byte from the pinned SDK. Their contents and upstream redistribution authority were not independently audited beyond the retained license/credit evidence.

The beaver image was generated with OpenAI's image tool without reference images, then reduced and articulated using project code. The prompt, source image and derivation records are in assets/generated/. Generated-image provenance does not establish uniqueness or copyright ownership.

The score derives from a project-owner-supplied song; no source MP3 or extracted song audio is included. Eight project-owner-recorded voice clips are represented by the exact 8-bit PCM source bank in firmware/src/dsp/voice_bank.h. Raw microphone recordings and playthrough captures are excluded. Song and character voice recordings supplied by Adam Wilson; project asset provenance and public-distribution authorization are retained in release/PUBLICATION-CHECKLIST.md.

O Canada uses the public-domain melody identified by Canadian Heritage, with original synthesized accompaniment. No lyrics, recorded performance, or published accompaniment was imported. The reference URLs and melody transcription are retained in music/o-canada-arrangement.json.
