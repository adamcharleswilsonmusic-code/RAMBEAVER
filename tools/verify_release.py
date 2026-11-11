#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Verify a downloaded source tree against its release hashes; no network or device access."""
import hashlib,json,re,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parent.parent
IGNORED={'.git','build','recordings','__pycache__','.venv'}
def files():
 return sorted(p for p in ROOT.rglob('*') if p.is_file() and not any(v in p.relative_to(ROOT).parts for v in IGNORED) and p.name!='.DS_Store')
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 expected={}
 for line in (ROOT/'SHA256SUMS').read_text().splitlines():
  h,name=line.split('  ',1);expected[name]=h
 actual={str(p.relative_to(ROOT)) for p in files()}-{'SHA256SUMS'}
 if actual!=set(expected):raise SystemExit('Release file list differs: '+str(sorted(actual^set(expected))))
 for name,h in expected.items():
  p=ROOT/name
  if p.is_symlink() or sha(p)!=h:raise SystemExit('Hash mismatch: '+name)
  if p.suffix.lower() in {'.mov','.mp3','.wav','.npz','.pyc'}:raise SystemExit('Private/generated recording in release: '+name)
  try:text=p.read_text()
  except UnicodeError:continue
  # Assemble private-path patterns so this verifier does not flag its own source.
  if re.search('/'+'Users/'+r'[^/\s]+/|/'+'Volumes/',text):raise SystemExit('Private source path: '+name)
  if re.search(r'(?:ghp_|github_pat_|sk-proj-)[A-Za-z0-9_]{20,}',text):raise SystemExit('Credential-like content: '+name)
 source=json.loads((ROOT/'provenance/installed-745-source.json').read_text())
 for name,h in source['firmware_files'].items():
  if sha(ROOT/name)!=h:raise SystemExit('Corresponding firmware source differs: '+name)
 manifest=json.loads((ROOT/'release/manifest.json').read_text())
 if sha(ROOT/manifest['firmware']['path'])!=manifest['firmware']['sha256']:raise SystemExit('Firmware does not match manifest')
 print(f"PASS {len(actual)+1} release files, hashes, corresponding firmware source, private-path and recording exclusions")
if __name__=='__main__':main()
