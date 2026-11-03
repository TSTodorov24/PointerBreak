# Manual test plan (things the automatic tests cannot check)

The automatic tests are in `Tests/Tests.cpp` (logic only). These checks are done by hand.

| # | Steps | Expected result |
|---|-------|-----------------|
| 1 | Start the game | Main menu with 4 buttons and 6 avatars |
| 2 | New game, press `-` at size 10 | Size stays 10 |
| 3 | New game, press `+` at size 50 | Size stays 50 |
| 4 | Choose each of the 6 avatars and start | The chosen avatar is shown in the maze |
| 5 | Press arrow keys against a wall | Player does not move |
| 6 | Hold an arrow key in a corridor | Player walks on its own and slides smoothly |
| 7 | Walk through a blinking orange wall | Player passes through it |
| 8 | Hold `M` | Whole maze visible, player cannot move, timer runs; releasing returns the camera |
| 9 | Press `H` | Whole maze and a yellow path are shown for 3 s; second `H` does nothing |
| 10 | Press `T` | Text maze is shown; `T` again returns to graphics |
| 11 | Press `P` / `ESC` | Pause screen, maze hidden, timer stopped; `P` resumes |
| 12 | Play Hard on size 10 and wait | "TIME IS UP!" appears when the time reaches 0 |
| 13 | Reach the exit | "YOU ESCAPED!" with moves, time and score |
| 14 | Win with a good score | Name entry: only letters are accepted, Enter needs 3 letters |
| 15 | Delete `Scores.txt` or fill it with garbage | Game starts normally with empty tables |
| 16 | Rename the `Assets` folder | Game starts (colored squares and default font) |
| 17 | Start the game from another folder | Assets are still found |
| 18 | Play size 50 for 5 minutes | No slowdown or crash |
