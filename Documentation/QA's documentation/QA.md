# QA Documentation — PointerBreak

**Author:** Vasilen Gospodinov
**Role:** Quality Engineer
**Files of the QA part:** `Tests/Tests.cpp`, `Docs/QA_ManualTestPlan.md`, `Docs/PointerBreak_QA_TestCases.xlsx`, this document

---

## 1. Goal

To prove that the game works as the assignment requires and that it never crashes:

- the maze always has a way out, for every size from 10x10 to 50x50;
- the player cannot walk through walls, but can cross special doors;
- time limit, score and high score table are calculated correctly;
- wrong input and broken files are handled without a crash;
- the dynamic memory (`new` / `delete[]`) has no leaks.

## 2. Test strategy

The project is split in two parts, and each part is tested in a different way.

| Part | Classes | How it is tested |
|------|---------|------------------|
| **Game logic** (no raylib, no `cin` / `cout`) | `GameLogic`, `MazeGenerator`, `PlayerAvatar`, `ScoreBoard` | **Automatic tests** — our own test functions in `Tests/Tests.cpp` |
| **Graphics and controls** (raylib) | `GameplayScreen`, `Screens`, `AvatarRenderer`, `UiHelper`, `GameManager` | **Manual tests** — plan in `Docs/QA_ManualTestPlan.md` |

Because the logic does not use a window or the console, the tests run in a few seconds on any computer and need no extra libraries.

**Why the tests are repeatable:** the mazes are random, but the test program starts with a fixed random seed (`std::srand(12345)`), so every run builds the same mazes. If a test fails, it fails the same way every time and the error can be found.

## 3. Automatic tests

The test program is `PointerBreakTests`. Every check prints `[PASS]` or `[FAIL]` with a description, and at the end the totals. The exit code is 0 only when everything passed.

**How to run (Windows):**

- Visual Studio: choose `PointerBreakTests.exe` in the startup item list and press `Ctrl+F5`.
- Command line: `build\Release\PointerBreakTests.exe`

### 3.1 Test functions

| # | Test function | Checks | What it proves |
|---|---------------|--------|----------------|
| 1 | `TestSizeValidation` | 5 | Sizes 9, 51 and -5 are rejected, 10 and 50 are accepted |
| 2 | `TestEveryMazeIsSolvable` | 2 | Every size 10..50 has a path from (0,0) to the exit; the shortest path never crosses a wall |
| 3 | `TestBorderAndSymmetry` | 3 | The outer border is closed; walls and doors are the same seen from both cells; no doors in the border |
| 4 | `TestDoors` | 2 | Doors are not more than requested and always sit on a wall |
| 5 | `TestWallCollision` | 3 | A wall blocks, an open side does not, a door can be crossed |
| 6 | `TestWinCondition` | 3 | Only the exit cell wins |
| 7 | `TestPlayerAvatar` | 4 | Start position, movement, reset, invalid avatar id is ignored |
| 8 | `TestTimeLimitAndDoorCount` | 5 | Easy has no limit, Hard is shorter than Medium, the limit grows with the path, door counts Easy > Medium > Hard |
| 9 | `TestScore` | 4 | Perfect run = 1000, slow run is lower, hint costs 20%, score never above 1000 |
| 10 | `TestScoreBoard` | 12 | Top 5 only, sorted, qualification, separate tables, name validation, broken and missing file |
| 11 | `TestTextMaze` | 4 | Text maze is 21x21 for size 10, `@` at the start, `E` at the exit, corners are walls |
| 12 | `TestMemoryCreateAndDestroy` | 1 | 200 mazes are created and destroyed without a crash |
| | **Total** | **48** | |

A detailed table with test data and expected results for every check is in `Docs/PointerBreak_QA_TestCases.xlsx`.

### 3.2 Test techniques used

- **Boundary values:** sizes 9 / 10 / 50 / 51, name lengths 2 / 3 / 4.
- **Equivalence classes:** valid and invalid names, good and bad lines in the score file.
- **Invariants on generated data:** for all 41 sizes the maze is solvable, the border is closed, walls and doors are symmetric.
- **White-box check:** `TestWallCollision` changes bits in the grid directly to test every case of a wall, an open side and a door.
- **Negative tests:** broken file, missing file, invalid avatar id, impossible sizes.

### 3.3 Results

| Date | Computer | Compiler | Result |
|------|----------|----------|--------|
| 31.10.2026 | Linux virtual | g++ 17, `-Wall -Wextra`, AddressSanitizer on | **48 passed, 0 failed**, no warnings, no memory errors |
| *date* | *Windows, Vasilen Gospodinov* | *Visual Studio 2022, `/W4`* | *fill in after running* |

## 4. Memory check

The maze is a dynamic `int**` array, so leaks are the main risk. We use **AddressSanitizer**, which finds memory leaks, double `delete`, and reading outside of an array.

    cmake -B build -DENABLE_ASAN=ON
    cmake --build build --config Release
    build\Release\PointerBreakTests.exe

What protects the memory in the code:

- `MazeGenerator` frees the grid in its destructor and before every new allocation;
- copying of `MazeGenerator` is forbidden (`= delete`), so the memory can never be freed twice;
- `TestMemoryCreateAndDestroy` creates and destroys 200 mazes of different sizes.

Result on Linux virtual: no leaks and no invalid access were reported.

## 5. Manual tests

Things that need a window cannot be tested with the program above: menus, camera, smooth movement, hint, text view, pause, game end and the behaviour when files are missing. They are listed as 18 steps in `Docs/QA_ManualTestPlan.md` and in the sheet **Manual Tests** of the Excel table. Every step has a status: `PASSED`, `FAILED` or `NOT RUN`.

## 6. Input validation (checked by tests)

| Input | Rule | Where it is tested |
|-------|------|--------------------|
| Maze size | Only 10..50; the buttons cannot go outside | `TestSizeValidation`, manual test 2 and 3 |
| Player name | Exactly 3 letters A-Z | `TestScoreBoard` |
| `Scores.txt` | Bad lines and wrong values are skipped; missing file means empty tables | `TestScoreBoard`, manual test 15 |
| Avatar image / font | Missing file does not crash the game | manual test 16 |
| Avatar id | Only 0..5 | `TestPlayerAvatar` |

## 7. What is not covered

Honest limits of our testing:

- The drawing code is not tested automatically, only by eye (manual plan).
- `TestDoors` checks that doors are on walls and not too many, but does not prove that every door really saves 10 steps. That is guaranteed by the code of `MazeGenerator::PlaceDoors`.
- Time is not tested in real seconds (the time limit is only calculated); this is checked by hand on Hard.
- Speed on 50x50 is checked only by playing (manual test 18).

## 8. Defects found

| # | Found by | Description | Fix | Status |
|---|----------|-------------|-----|--------|
| 1 | Looking at the screen | On the size screen the big text `20 x 20` was so wide that it went under the `-` and `+` buttons | Font made smaller (72 → 48) | Fixed |
| | | *add new defects here: what happened, how to repeat it, who fixes it* | | |
