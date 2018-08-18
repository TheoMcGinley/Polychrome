#ifndef SCORING_H
#define SCORING_H
#define GRIDSIZE 16
double calc_positional_score(int gridx, int gridy);
double calculate_score(int grid[GRIDSIZE][GRIDSIZE], int gridx, int gridy, int windowwidth, int windowheight);
#endif // SCORING_H
