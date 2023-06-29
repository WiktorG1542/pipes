Once you run the executable and see the window,

press p to gerenate new puzzle uzing prims algorithm

press d to generate new puzzle using dfs esque algorithm

press s to solve (not yet completed)

left click to rotate

right click to lock/unlock (once you are sure of the blocks position)


command to compile is in command.txt


to play do
./game x, y, s, d

where x and y is the size of the puzzle and s should be either 32 or 64 (size of the sprites), and d is the delay in miliseconds that the solver takes to show you its moves

so for example

./game 10 15 32 1 generates a 10x15 puzzle with small sprites, that will get solved quickly

./game 5 5 64 100 generates a 5x5 puzzle with big sprites, that will let you see the solver's moves in more detail
