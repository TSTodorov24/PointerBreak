# Design decisions (agreed by the team)

| Topic | Decision |
|-------|----------|
| Theme | Retro arcade, pixel-art monsters as avatars, Press Start 2P font |
| Window | Fixed 1280x720, cell 64 px (avatars are 192x192 = 3 x 64, so pixel art stays sharp) |
| Size | 10x10 .. 50x50, chosen with buttons (cannot be invalid) |
| View | Camera follows the player. Holding **M** shows the whole maze (no moving, timer runs) |
| Maze data | `int**`, bit flags per cell for walls and doors |
| Generator | Recursive Backtracker (iterative, with a stack) |
| Doors | Passable walls, placed only where they save at least 10 steps (checked with BFS) |
| Door count | Easy: size, Medium: size/2, Hard: max(2, size/5) |
| Movement | Logic jumps cell by cell, drawing and camera slide smoothly; held key repeats |
| Hint (H) | Once per game, 3 s, whole maze + path, costs 20% of the score |
| Difficulty | Time limit = shortest path * seconds per step (Easy none, Medium 4, Hard 2) and number of doors |
| Score | 50% route length, 50% speed, max 1000; top 5 per difficulty, 3-letter name |
| Pause | Hides the maze and stops the timer |
| Text view | **T** switches to a text maze drawn with the monospace font |
| Avatars | Only cosmetic (no effect on the game) |
| Sound, bonus features | Not included (Minotaur, keys, music, themes) - only if the core is finished |
| Memory | Manual `new` / `delete[]`, copy forbidden, tested with AddressSanitizer |
| Screens | Big stateful screen = class (`GameplayScreen`), simple screens = functions in `Screens.cpp` |

## Roles
- Backend: `GameLogic`, `MazeGenerator`, `PlayerAvatar`
- Frontend: `GameplayScreen`, `AvatarRenderer`, `UiHelper`, `GameManager`
- QA: `Tests/Tests.cpp`, `Docs/QA_ManualTestPlan.md`
- Scrum Trainer: GitHub, documentation, presentation, `ScoreBoard`, `Screens.cpp`
