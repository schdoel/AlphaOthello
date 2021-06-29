# AlphaOthello

1. Initialize boardweight
   The corner of the board is the most valuable spot, and stable.
   The stability and score of the board can be represented by the boardweight.

2. Count heuristic value
    Calculate the score on each board position, 
      *all opponent values will be (-) instead of (+) to total points*
   If winner == player, +more points
   If current player == player, points + size of the next valid spot
   Add the total boardweight with respective disks (your disks - opponent disks)
   Check every corner:
    - If the row and collumn of that corner has the same colored disk, points ++
    - if the whole row / collumn is filled already, add more points by multiplying the total points
    - if the diagonal X square is also filled, add more points
    - For opponent do the opp

3. Apply minimax algorithm
    Create a struct, with points and the value
    apply minimax and return struct

4. Apply alpha beta pruning to minimax algorithm
    Just apply alpha beta to minimax, and break according algo
    
5. Create MTC to optimize AI (not enough time)
    they say dream big lol
