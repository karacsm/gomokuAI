# gomoku
An AI for the freestyle variant of gomoku also known as connect 5. The goal of the game is to connect five crosses or circles in a row before your opponent does and there are no additional rules. The AI is a minimax algorithm with alpha-beta pruning, a static evaluation function, move generation, move ordering and a transposition table. You can read about these algorithms here: https://www.chessprogramming.org.

System requirements:

Windows 64 bit OS and at least 4 GB RAM.

To compile the program use VS2019 to open the project file in the Gomoku folder and set the configuration to Release x64 and then build the project. You can launch the program from VS or copy the executable from the Gomoku/x64/Release folder to somewhere else and also copy the Gomoku/resource folder and the Gomoku/threat_table.txt file to the same folder next to the executable and then you can launch the executable.
