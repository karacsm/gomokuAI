# gomoku
An AI for the freestyle variant of gomoku also known as connect 5. The goal of the game is to connect five crosses or circles in a row before your opponent does and there are no additional rules. The AI is a minimax algorithm with alpha-beta pruning, a static evaluation function, move generation, move ordering and a transposition table. You can read about these algorithms here: https://www.chessprogramming.org.

System requirements:

Windows 64 bit OS

The executable can be built using the commands 

`cd gomoku/source`

`cmake -DCMAKE_INSTALL_PREFIX=<install prefix> -B ./build .`

`cmake --build ./build --config=Release`

To run the executable it needs to be installed using the command

`cmake --install ./build`
