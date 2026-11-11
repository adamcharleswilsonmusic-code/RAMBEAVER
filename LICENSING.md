# RAMBEAVER licensing and source correspondence

The project source code is licensed under **GPL-3.0-only**, with the full text in LICENSE. Inherited copyright and license headers are retained. Component-specific licenses remain in assets/fonts/, LICENSES/, and vendor/ac79/; see THIRD_PARTY_NOTICES.md.

This candidate targets **FM-1_745**, the firmware installed and verified on October 17, 2026. It includes the application, loader, HAL, DSP engines and tables, fonts, package inputs, source asset headers, build/installation scripts, and matching browser renderer. The public source and binary inventories are recorded in release/manifest.json, release/source-inventory.json and SHA256SUMS. Rebuild results are in release/validation.json.

The platform is derived from FoMni commit405ef41aef75f7e70afd6decc01505df07c708e9 and the Felucca tree1727537624390cc3f9c7dff2456b43b3a8edb005. Pinned upstream notices and DSP excerpts are retained in provenance/upstream/. No new third-party implementation was imported for this release preparation.

The exact compiled game's firmware sources and data headers are preserved. Release-only changes make launchers, recorder configuration, documentation and asset tooling portable; they do not change game art, music, collision geometry or firmware behavior. The firmware clean rebuild must match the installed package's SHA256. The browser runtime notices are included beside the bundled WASM.

GPL source availability does not establish rights in a song composition, sound recording, font, generated image or vendor binary. The owner identified the song and voices as their own and explicitly authorized public distribution of this game. That session authorization is recorded in the publication checklist. It is not an independent chain-of-title audit or a blanket claim about vendor internals.
