# gomokuAI

![](https://github.com/karacsm/gomokuAI/blob/main/images/won.png?raw=true)

An AI for the freestyle variant of gomoku also known as connect 5. The goal of the game is to connect five crosses or circles in a row before your opponent does and there are no additional rules. The AI is a minimax algorithm with alpha-beta pruning, a static evaluation function, move generation, move ordering and a transposition table. You can read about these algorithms here: https://www.chessprogramming.org.

System requirements:

Windows 64 bit OS

Library dependencies:

Direct2D

### Build instructions

The executable can be built using the commands 

`cd gomoku/source`

`cmake -DCMAKE_INSTALL_PREFIX=<install prefix> -B ./build .`

`cmake --build ./build --config=Release`

The executable needs to be installed using the command

`cmake --install ./build`

### How to play

Clicking the executable in the install folder will launch the AI. Always the player with the X symbol starts after pressing PLAY. Clicking the box above the PLAY button in the menu changes the AI's symbol from X to O or from O to X. When playing you can make moves by clicking the tile where you want place your symbol. After your move the AI thinks for about a minute then plays its move. The game continues until somebody wins or the playing area is filled completely in, in that case the game ends in a draw.

