# 🧠 Phase 4: Broker Core & Protocol v1

> **Status:** 🚧 in progress · **Roadmap:** `plan/ROADMAP.vi.md` (Day numbers refer to the Phase 4–7 roadmap, not to `phase-1..3.md`)

---

## 📅 Day 1 — Documentation drift check

**What was done:**
- Added `plan/ROADMAP.vi.md` (Vietnamese planning doc, tracked in git, outside `docs/`).
- Froze historical docs: `docs/phase-1.md`, `phase-2.md`, `phase-3.md` now start with `<!-- historical -->`.
- Added `tools/check_docs.sh` (4 rules: path existence, test files in CMake, relative links, test-count in README).
- Wired `check_docs.sh` into the Release job of `.github/workflows/build.yml`.

**Decisions:**
- **D1 — Roadmap in `plan/`, not `docs/`.** `docs/` is English-only repo documentation; the roadmap is a personal Vietnamese planning doc. Avoids mixing languages and avoids two sources of truth.
- **D2 — `check_docs.sh` skips `plan/`.** Roadmap paths are *planned*, not *current*; they must not be checked against the working tree.
- **D3 — Removed `docs/journal/`.** `docs/journal/audit-plan.md` and `audit-log.md` were created only to keep commits green during the audit period, not because they carried value. Same category as the previously removed `DECISIONS.md`, `inventory.md`, `TODO.md`. `check_docs.sh` now scans only `README.md` and `docs/*.md`.

**Verification:**
- `bash tools/check_docs.sh` → red on the old tree (broken links, `audit/clean` in CI), green after fixes.
- CI: Release job runs the script.

---

## 📅 Day N — *(next entry when there is something to document)*