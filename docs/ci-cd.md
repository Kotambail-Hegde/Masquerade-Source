# Masquerade CI/CD Pipeline Documentation

## Overview

The Masquerade project uses two GitHub repositories with an automated CI/CD pipeline connecting them:

| Repository | Purpose |
|---|---|
| **Masquerade-Source** | Source code, build workflows, release automation |
| **Masquerade-Emulator** | Distribution repo — binaries, web hosting, submodule pointer to Source |

### Repository Structure

```
Masquerade-Source/
├── .github/workflows/
│   ├── build.yml              ← Regular CI builds
│   ├── release.yml            ← Release automation
│   └── update-submodule.yml   ← Submodule sync after release
├── helpers/helpers.h          ← Contains VERSION macro
└── assets/                    ← Game assets (uploaded as artifact)

Masquerade-Emulator/
├── .github/workflows/
│   └── cleanup-on-pr-close.yml ← Cleanup when release PR is rejected
├── src/Masquerade-Source/     ← Git submodule → Masquerade-Source
├── windows/                   ← Windows binaries + assets
├── linux/                     ← Linux binaries + assets
├── emscripten/                ← WebAssembly artifacts
├── hosting/                   ← GitHub Pages web hosting
└── macos/                     ← (future)
```

---

## Secrets Required

| Secret Name | Repo | Description |
|---|---|---|
| `EMULATOR_REPO_PAT` | Masquerade-Source | PAT with `repo` + `workflow` scope — used for cross-repo operations |
| `EMULATOR_CLEANUP_PAT` | Masquerade-Emulator | Same PAT — used for cleanup on PR rejection |
| `SUBMODULE_TOKEN` | Masquerade-Emulator | Same PAT — used by GitHub Pages Jekyll build to checkout submodule |

---

## Workflow 1: build.yml — Regular CI Builds

### Trigger Conditions

```
✅ Triggers:
  - workflow_dispatch (manual)
  - pull_request (any branch EXCEPT release/*)
  - push to master (when source files change)
  - schedule (every Friday 5PM IST)

❌ Does NOT trigger on:
  - PRs targeting release/* branches
  - Push commits starting with "chore: bump VERSION"
  - Push commits starting with "Revert"
```

### Flow Diagram

```
┌─────────────────────────────────────────────────────────┐
│                     build.yml                           │
│                                                         │
│  Trigger: push/PR/schedule/manual                       │
│                                                         │
│  ┌──────────────────────────────────────────────────┐   │
│  │  Job-level guard (for push trigger only):        │   │
│  │  Skip if commit msg starts with:                 │   │
│  │    • "chore: bump VERSION"  (release automation) │   │
│  │    • "Revert"               (cleanup revert)     │   │
│  └──────────────────────────────────────────────────┘   │
│           │                                             │
│           ▼                                             │
│  ┌───────────────────────────────────────────────────┐  │
│  │  build_windows  │  build_linux  │ build_emscripten│  │
│  │  (parallel, independent)                          │  │
│  └───────────────────────────────────────────────────┘  │
│           │                                             │
│           ▼                                             │
│  Upload artifacts (windows-build, linux-build,          │
│                    emscripten-build)                    │
│                                                         │
│  Deploy to GitHub Pages (only on push to master)        │
└─────────────────────────────────────────────────────────┘
```

### Build Matrix

```
build_windows  → Visual Studio 18 2026, MSVC, x64
                 Output: build/bin/Release/masquerade.exe + SDL3.dll + assets/

build_linux    → Ninja, GCC
                 Output: build/bin/masquerade + assets/

build_emscripten → Emscripten 3.1.66
                   Output: masquerade.data/.html/.js/.wasm
```

### External Dependencies (cached)

```
imgui, miniz, stb, rapidjson, boost (1.89.0), SDL3, nativefiledialog-extended
+ emscripten-browser-file (Emscripten only)
```

---

## Workflow 2: release.yml — Full Release Pipeline

### Trigger

```
workflow_dispatch with inputs:
  version           (required) e.g. "0.7009"
  release_windows   (boolean, default: true)
  release_linux     (boolean, default: true)
  release_emscripten(boolean, default: true)
  release_macos     (boolean, default: false — not yet supported)
```

> **Note:** The platform checkboxes control what goes INTO the Masquerade-Emulator PR.
> All three platforms ALWAYS build — there is no way to skip a build.

### Complete Flow Diagram

```
You trigger release.yml
Input: version=0.7009, release_windows=true, others=false
         │
         ▼
┌─────────────────────────────────────────────────────────────┐
│  JOB: bump_version                                          │
│                                                             │
│  1. Checkout Masquerade-Source                              │
│  2. Create branch: release/0.7009                           │
│  3. Patch helpers/helpers.h:                                │
│     #define VERSION  static_cast<float>(0.7009)             │
│  4. Commit: "chore: bump VERSION to 0.7009"                 │
│  5. Push release/0.7009 to Masquerade-Source                │
│  6. Upload assets/ as artifact                              │
│  7. Open PR #1 in Masquerade-Source                         │
│     release/0.7009 → master                                 │
└─────────────────────────────────────────────────────────────┘
         │
         │ (bump_version must succeed)
         │
    ┌────┴────────────────────────┐
    ▼         ▼                  ▼
┌──────────┐ ┌──────────┐ ┌──────────────┐
│  build_  │ │  build_  │ │    build_    │
│ windows  │ │  linux   │ │ emscripten   │
│          │ │          │ │              │
│ Checks   │ │ Checks   │ │ Checks out   │
│ out      │ │ out      │ │ release/     │
│ release/ │ │ release/ │ │ 0.7009       │
│ 0.7009   │ │ 0.7009   │ │              │
│          │ │          │ │ Builds WASM  │
│ Builds   │ │ Builds   │ │ artifacts    │
│ MSVC x64 │ │ Ninja    │ │              │
└────┬─────┘ └────┬─────┘ └──────┬───────┘
     │            │               │
     │   All three must PASS      │
     └────────────┬───────────────┘
                  │
                  ▼
┌─────────────────────────────────────────────────────────────┐
│  JOB: release                                               │
│                                                             │
│  1. Clean up existing branch/tags (idempotent re-runs):     │
│     - Delete release/0.7009 in Emulator (if exists)         │
│     - Delete tag 0.7009 in Emulator (if exists)             │
│     - Delete tag 0.7009 in Source (if exists)               │
│                                                             │
│  2. Download all 4 artifacts:                               │
│     assets/, windows-build/, linux-build/, emscripten-build/│
│                                                             │
│  3. Checkout Masquerade-Emulator → emulator-repo/           │
│     (path: emulator-repo keeps artifacts/ safe)             │
│                                                             │
│  4. Create branch: release/0.7009 in Emulator               │
│                                                             │
│  5. Place artifacts (selective by input):                   │
│     hosting/         ← always updated (masquerade.* files)  │
│     LICENSE.md       ← always updated (from Windows build)  │
│     windows/         ← if release_windows=true              │
│     linux/           ← if release_linux=true                │
│     emscripten/      ← if release_emscripten=true           │
│                                                             │
│     Asset rules:                                            │
│     - rsync overlay (NO rm -rf, preserves placeholders)     │
│     - Excludes: gb/gbc/gba bios *.bin (legal)               │
│     - Excludes: saves/ (user data)                          │
│     - Excludes: spaceInvaders/audio/ (legal)                │
│     - Excludes: c8/db/chip-8-database/ (submodule)          │
│     - Excludes: internal/cheats.txt                         │
│                                                             │
│  6. git add (only touched folders)                          │
│  7. Commit + Push release/0.7009 → Masquerade-Emulator      │
│                                                             │
│  8. Create tag 0.7009 on Masquerade-Emulator                │
│  9. Create tag 0.7009 on Masquerade-Source                  │
│                                                             │
│  10. Set PENDING status on Emulator branch HEAD             │
│      context: "submodule-update"                            │
│      → Blocks merge button on Emulator PR                   │
│                                                             │
│  11. Open PR #2 in Masquerade-Emulator                      │
│      release/0.7009 → main                                  │
│                                                             │
│  12. Auto-merge PR #1 in Masquerade-Source                  │
│  13. Delete release/0.7009 branch from Masquerade-Source    │
│  14. Trigger update-submodule.yml                           │
│      (version=0.7009, triggered_by_release=true)            │
└─────────────────────────────────────────────────────────────┘
         │
         ▼
  Emulator PR open, PENDING ← merge blocked until submodule updated
  Source PR merged ✅
```

### Artifact Placement Rules

```
Artifact Source              Destination        Notes
─────────────────────────────────────────────────────────────────────
artifacts/emscripten/        hosting/           Always. Only masquerade.*
                                                files replaced. index.html
                                                and others preserved.

artifacts/windows/Release/   windows/           If release_windows=true.
                                                rsync overlay. Excludes
                                                *.pgc, *.pgd, LICENSE.md.
                                                Assets placed separately
                                                with legal exclusions.

artifacts/linux/             linux/             If release_linux=true.
                                                rsync overlay. Assets
                                                placed separately.

artifacts/emscripten/        emscripten/        If release_emscripten=true.
masquerade.* only                               Only masquerade.* replaced.

artifacts/windows/           ./LICENSE.md       Always. Placed at Emulator
Release/LICENSE.md                              repo root.
```

### Failure / Cleanup Behavior

```
Any job fails or is cancelled
         │
         ▼
┌─────────────────────────────────┐
│  JOB: cleanup_on_failure        │
│                                 │
│  - Close open Source PR         │
│    (with failure comment)       │
│  - Delete Source release branch │
│                                 │
│  NOTE: Emulator branch/tags     │
│  are NOT deleted here —         │
│  release.yml cleans those up    │
│  automatically on next run.     │
└─────────────────────────────────┘
```

---

## Workflow 3: update-submodule.yml — Submodule Sync

### Overview

This workflow updates the `src/Masquerade-Source` submodule pointer in Masquerade-Emulator to point to the latest master commit, then sets the PR status to SUCCESS to unblock the merge button.

### Trigger Conditions

```
✅ Triggered by:
  1. release.yml explicitly via:
     gh workflow run update-submodule.yml
       --field version=0.7009
       --field triggered_by_release=true

  2. Manual workflow_dispatch:
     - version: (optional, auto-detects from helpers.h if empty)
     - triggered_by_release: false (default)

❌ Does NOT trigger automatically on any push
   (previously caused recursive trigger issues)
```

### Path A — Release Flow (triggered_by_release=true)

```
release.yml triggers update-submodule.yml
         │
         ▼
┌─────────────────────────────────────────────────────────┐
│  Checkout Masquerade-Source master                      │
│  Resolve version (from input)                           │
│  Capture master HEAD SHA                                │
│                                                         │
│  Checkout Masquerade-Emulator → emulator-repo/          │
│                                                         │
│  Verify release/0.7009 exists in Emulator               │
│  (fails if release.yml didn't run first)                │
│                                                         │
│  git checkout release/0.7009                            │
│                                                         │
│  cd src/Masquerade-Source                               │
│  git fetch origin master                                │
│  git checkout origin/master   ← latest master HEAD      │
│  cd ../..                                               │
│                                                         │
│  git add src/                                           │
│  git commit "chore: update src/ submodule to 0.7009"    │
│  git push origin release/0.7009                         │
│                                                         │
│  Set SUCCESS status on:                                 │
│    - New submodule commit (HEAD)                        │
│    - Original release commit (HEAD~1)                   │
│  context: "submodule-update"                            │
│  → Both commits show ✅, merge button unblocked         │
└─────────────────────────────────────────────────────────┘
         │
         ▼
  Emulator PR: ✅ submodule-update — ready to merge
  You manually review and merge the PR
```

### Path B — Standalone (triggered_by_release=false)

```
Manual workflow_dispatch
         │
         ▼
┌─────────────────────────────────────────────────────────┐
│  Checkout Masquerade-Source master                      │
│  Resolve version (from input or helpers.h)              │
│  Capture master HEAD SHA                                │
│                                                         │
│  Checkout Masquerade-Emulator → emulator-repo/          │
│                                                         │
│  Create NEW branch:                                     │
│  submodule-update/0.7009-20260315143022                 │
│  (timestamp suffix prevents collisions)                 │
│                                                         │
│  cd src/Masquerade-Source                               │
│  git fetch origin master                                │
│  git checkout origin/master                             │
│  cd ../..                                               │
│                                                         │
│  git add src/                                           │
│  git commit + push                                      │
│                                                         │
│  Open new PR in Masquerade-Emulator                     │
│  (no status check set — nothing to unblock)             │
└─────────────────────────────────────────────────────────┘
```

### Failure Handling (Release Flow Only)

```
update_submodule job FAILS (triggered_by_release=true)
         │
         ▼
┌─────────────────────────────────────────────────────────┐
│  JOB: cleanup_on_failure                                │
│                                                         │
│  1. Revert version bump commit on Source master         │
│     (git revert → new commit, no force push)            │
│                                                         │
│  2. Delete Emulator tag 0.7009                          │
│  3. Delete Source tag 0.7009                            │
│                                                         │
│  NOTE: Emulator PR and branch are PRESERVED             │
│  Re-trigger update-submodule.yml manually to retry      │
│  without re-running the full release pipeline           │
│                                                         │
│  Summary step prints re-trigger instructions            │
└─────────────────────────────────────────────────────────┘
```

---

## Workflow 4: cleanup-on-pr-close.yml — PR Rejection Cleanup

### Location: Masquerade-Emulator repo

### Trigger

```
pull_request event: closed (without merging)
on branches: main
branch filter: head ref starts with "release/"
```

### Flow

```
You close/reject the Emulator PR without merging
         │
         ▼
┌─────────────────────────────────────────────────────────┐
│  JOB: cleanup                                           │
│                                                         │
│  Extract version from branch name                       │
│  (release/0.7009 → 0.7009)                              │
│                                                         │
│  1. Delete Emulator release branch                      │
│  2. Delete Emulator tag (if exists)                     │
│  3. Delete Source tag (if exists)                       │
│  4. Clone Masquerade-Source                             │
│     Find bump commit for this version                   │
│     git revert it on master                             │
│     Push to Source master                               │
└─────────────────────────────────────────────────────────┘
```

---

## Complete End-to-End Release Flow

```
YOU
│
│  trigger release.yml
│  (version=0.7009, release_windows=true)
│
▼
Masquerade-Source
├── bump_version job
│   ├── creates branch release/0.7009
│   ├── patches helpers.h → VERSION=0.7009
│   ├── uploads assets artifact
│   └── opens PR #1 (release/0.7009 → master)
│
├── build_windows ──┐
├── build_linux   ──┤ all checkout release/0.7009
└── build_emscripten┘ all must PASS
         │
         ▼
Masquerade-Emulator
├── release job
│   ├── cleans up stale branch/tags (if any)
│   ├── creates branch release/0.7009
│   ├── copies artifacts (windows only, + hosting always)
│   ├── sets PENDING status → merge button BLOCKED 🔴
│   ├── opens PR #2 (release/0.7009 → main)
│   ├── auto-merges PR #1 in Source
│   ├── deletes Source release branch
│   └── triggers update-submodule.yml
│
Masquerade-Source
└── update-submodule.yml
    ├── checks out Emulator release/0.7009
    ├── cd src/Masquerade-Source
    ├── git fetch origin master
    ├── git checkout origin/master  ← bump commit now on master
    ├── commits submodule pointer update
    ├── sets SUCCESS on both commits → merge button UNBLOCKED 🟢
    └── done

YOU
├── review Emulator PR #2
│   (submodule updated, binaries correct, assets correct)
└── merge PR #2 → main
    │
    └── GitHub Pages deploys /hosting → web version live ✅
```

---

## Build Trigger Decision Table

| Event | Condition | build.yml | release.yml | update-submodule.yml |
|---|---|---|---|---|
| PR opened | branch != release/* | ✅ builds | ❌ | ❌ |
| PR opened | branch == release/* | ❌ skipped | ❌ | ❌ |
| Push to master | normal code commit | ✅ builds | ❌ | ❌ |
| Push to master | "chore: bump VERSION" | ❌ skipped | ❌ | ❌ |
| Push to master | "Revert ..." | ❌ skipped | ❌ | ❌ |
| workflow_dispatch | — | ✅ builds | ✅ releases | ✅ updates |
| Schedule (Friday) | — | ✅ builds | ❌ | ❌ |
| release.yml calls | triggered_by_release=true | ❌ | ❌ | ✅ updates |

---

## Re-run / Recovery Scenarios

### Scenario 1: Build fails

```
release.yml fails at build_windows/linux/emscripten
    │
    ▼
cleanup_on_failure fires:
  - closes Source PR
  - deletes Source branch

Action: Fix the build issue, trigger release.yml again
```

### Scenario 2: Emulator PR creation fails

```
release.yml fails at release job
    │
    ▼
cleanup_on_failure fires:
  - closes Source PR
  - deletes Source branch

Action: Trigger release.yml again
  (release.yml will clean up stale tags/branch automatically)
```

### Scenario 3: update-submodule.yml fails

```
Submodule update fails
    │
    ▼
cleanup_on_failure fires:
  - reverts bump commit on Source master
  - deletes both tags

Emulator PR and branch still exist.

Action: Fix the issue, then re-run update-submodule.yml manually:
  version = 0.7009
  triggered_by_release = true
```

### Scenario 4: You reject the Emulator PR

```
You close PR without merging
    │
    ▼
cleanup-on-pr-close.yml fires:
  - deletes Emulator release branch
  - deletes Emulator tag
  - deletes Source tag
  - reverts Source bump commit

Action: Trigger release.yml again when ready
```

### Scenario 5: Re-running release for same version

```
release.yml always runs clean-up at start of release job:
  - deletes existing Emulator release branch (if any)
  - deletes existing Emulator tag (if any)
  - deletes existing Source tag (if any)

So re-triggering is always safe — no manual cleanup needed.
```

---

## Asset Legal Exclusions

The following files are excluded from all release artifacts for legal reasons:

| Path | Reason |
|---|---|
| `assets/gb/bios/*.bin` | BIOS ROM — legal |
| `assets/gbc/bios/*.bin` | BIOS ROM — legal |
| `assets/gba/bios/*.bin` | BIOS ROM — legal |
| `assets/saves/` | User save data |
| `assets/spaceInvaders/audio/` | Audio samples — legal |
| `assets/c8/db/chip-8-database/` | Git submodule — pointer kept, contents not copied |
| `assets/internal/cheats.txt` | Internal config |

---

## Status Check: submodule-update

A GitHub commit status check named `submodule-update` is used to block the Emulator PR merge until the submodule has been correctly updated.

```
State     Set by                    Meaning
─────────────────────────────────────────────────────────
PENDING   release.yml               PR created, waiting for submodule sync
SUCCESS   update-submodule.yml      Submodule updated, safe to merge
```

To use this as a required check (prevents accidental merges):
1. Masquerade-Emulator → Settings → Branches → Add rule on `main`
2. Enable "Require status checks to pass before merging"
3. Search for and add: `submodule-update`

> **Note:** The check name only appears in search after it has been reported at
> least once. Run a release first, then add the branch protection rule.

---

## Files Summary

| File | Repo | Purpose |
|---|---|---|
| `.github/workflows/build.yml` | Masquerade-Source | Regular CI builds |
| `.github/workflows/release.yml` | Masquerade-Source | Full release pipeline |
| `.github/workflows/update-submodule.yml` | Masquerade-Source | Submodule sync + PR unblock |
| `.github/workflows/cleanup-on-pr-close.yml` | Masquerade-Emulator | Cleanup on PR rejection |