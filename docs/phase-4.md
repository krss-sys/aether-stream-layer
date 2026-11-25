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

## 📅 Day 2 — Protocol v1 specification

**What was done:**
- Rewrote `docs/PROTOCOL.md` for v1: request/response envelopes, command table, status codes, semantics (a)–(g), size limits, worked hex examples.
- Moved `kMaxFrameSize` from `frame.hpp` to `config.hpp`; added `kMaxMessageSize`, `kMaxTopics`, `kMaxGroups`, `kMaxFetchMessages` and two `static_assert`s.
- Added `#include "config.hpp"` to `src/frame.cpp` and `src/client.cpp`.

**Decisions:**
- **D4 — Protocol v1 lives inside the frame `Payload`.** No changes to `frame.cpp` or `FrameDecoder`; v1 is a structure inside the existing 1 MiB frame. Keeps the framing layer stable and testable.
- **D5 — `kOk` on `PRODUCE` means "accepted into broker memory", not "stored on disk".** Matches the AP design: fast ACK, data loss on crash is accepted and documented.
- **D6 — `COMMIT(k)` means "next FETCH starts at k".** Offset semantics are off-by-one-prone; writing it down once prevents a classic bug.

**Verification:**
- `cmake --build build -j` → green.
- `ctest --test-dir build --output-on-failure` → green.
- `bash tools/check_docs.sh` → green.

---

## 📅 Day 3 — Protocol v1 codec

**What was done:**
- Added `include/message.hpp` + `src/message.cpp`: `Command`, `StatusCode`, `Request`, `decode_request`, `encode_response`, `encode_request`, `decode_response`, `BodyReader`.
- Added `tests/test_message.cpp`.
- Patched PROTOCOL.md semantics: (h) ACK ≠ visible to FETCH; (i) only PRODUCE creates topics.
- Patched PROTOCOL.md size limits: `kMaxTopics` is a hard limit, not soft.

**Decisions:**
- **D7 — ACK does not imply visibility.** `kOk` is returned before the worker appends to the log; consumers must poll. Tests and e2e use bounded waits, not fixed sleeps.
- **D8 — Only PRODUCE creates topics.** `FETCH`/`COMMIT` on a non-existent topic return an empty result, never create. Prevents untrusted clients from creating topics by asking.
- **D9 — `kMaxTopics = 64` is a hard limit**, enforced at `PRODUCE` (Day 7). Supersedes the "soft" wording from Day 2.
- **D10 — `kIngestCapacity` is defined on Day 6**, not Day 2; it is a broker constant, not a protocol constant.

**Verification:**
- *(to be filled when the codec and tests are done)*

---

## 📅 Day N — *(next entry when there is something to document)*