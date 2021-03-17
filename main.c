#include <stdio.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define EMPTY 0
#define CIRCLE 1
#define CROSS 2

struct result{
    double win, lose, draw; // probability of win/lose/draw (between 0 and 1)
};


struct result cross_round(int s[4][20000], int step, int board, int p3[10]);    //forward declaration

int hasWinner(int gameBoard[3][3]) {
    int ok;
    for (int i = 1; i <= 2; ++i){
        for (int j = 0; j < 3; ++j){
            ok = 1;
            for (int k = 0; k < 3; ++k) if (gameBoard[j][k] != i) ok = 0;
            if (ok == 1) return 1;
            ok = 1;
            for (int k = 0; k < 3; ++k) if (gameBoard[k][j] != i) ok = 0;
            if (ok == 1) return 1;
        }
        ok = 1;
        for (int j = 0; j < 3; ++j) if (gameBoard[j][j] != i) ok = 0;
        if (ok == 1) return 1;
        ok = 1;
        for (int j = 0; j < 3; ++j) if (gameBoard[j][2-j] != i) ok = 0;
        if (ok == 1) return 1;
    }
    return 0;
}

// check if the game is ended under the game status passed
int check_winner(int board, int p3[10]){
    /*
    convert the board encoded as integer to array form
        - note that the winning condition of the game is symmetrical,
          so that the conversion need not be in same orientation
    then check if the game has ended using hasWinner function written in part 1
    */
    int gameBoard[3][3];
    for (int i = 8; i >= 0; --i){
        gameBoard[i/3][i%3] = board / p3[i];
        board %= p3[i];
    }
    //printf("hello\n");
    return hasWinner(gameBoard);
}

// state array, xth step (3/5/7/9), encoded board state, p3 array
struct result circle_round(int s[4][20000], int step, int board, int p3[10]){
    if (check_winner(board, p3)){   //won after previous step, stop searching
        struct result r = {1, 0, 0};    // return "win result" to show that the previous step is optimal
        return r;
    }

    if (step == 9){ // terminal condition of the recursion
        struct result tmp = {0, 0, 1};  // preset draw state
        for (int i = 0; i < 9; ++i){
            if (board % p3[i+1] / p3[i] == 0){  //grid i is empty
                /*
                    player 1 wins if the mark is placed here, this implies that the previous move
                    is not optimal since it led to a possibility of losing
                    under the assumption that there always exist a non-losing method, this
                    branch should not be arrived, so "lose result" is returned
                    in such case, the previous step would not be chosen
                */
                if (check_winner(board + p3[i], p3)){
                    tmp.win = 0;
                    tmp.lose = 1;
                    tmp.draw = 0;
                    return tmp;
                }
                break;
            }
        }
        return tmp; //since player 1 is the last mover, the game must draw if AI did not lose
    }

    struct result tmp, r = {0, 0, 0};
    for (int i = 0; i < 9; ++i){
        if (board % p3[i+1] / p3[i] == 0){  //grid i is empty
            tmp = cross_round(s, step + 1, board + p3[i], p3);
            /*
            as explained previously, any possibility of losing can be eliminated
            hence, if the probability of losing > 0, the step should never be chosen
            */
            if (tmp.lose > 0){
                tmp.win = 0;
                tmp.lose = 1;   // continue to return "losing result" to avoid this branch
                tmp.draw = 0;
                return tmp;
            }
            // else, compute the probability of win and draw respectively
            // (assuming an equal chance of choosing any empty grid)
            r.win += tmp.win / (9 - step);
            r.draw += tmp.draw / (9 - step);
        }
    }
    // return the results under the inputted game status
    return r;
};

// state array, xth step (2/4/6/8), encoded board state, p3 array
struct result cross_round(int s[4][20000], int step, int board, int p3[10]){
    // check if the game is ended at current state
    // if yes, it means that the previous player won, which is human player
    if (check_winner(board, p3)){
        struct result r = {0, 1, 0};    // result indicates losing
        return r;
    }
    // best stores the best result obtained out of all possible steps
    struct result tmp, best = {-1, 0, -1};
    int best_step;  // the mark location of the optimal step
    for (int i = 0; i < 9; ++i){ // exhaust all possible mark location
        if (board % p3[i+1] / p3[i] == 0){  //grid i is empty
            // search the next step to obtain the result of applying the mark
            tmp = circle_round(s, step + 1, board + p3[i] * 2, p3);
            // if this step is the best step until now, update best and best_step
            // (the best step means that losing is avoided, and the chance of winning is the highest)
            if (tmp.lose == 0 && tmp.win > best.win){
                best_step = i;
                best = tmp;
            }
        }
    }
    /* update state array: under this step and board status,
       placing the mark at best_step yields the best result */
    s[step/2-1][board] = best_step;
    //return the result when the optimal step is chosen
    return best;
}

// starting point of dfs
void start(int s[4][20000], int p3[10]){
    // generates all possible game state in first step
    for (int i = 0; i < 9; ++i){
        cross_round(s, 2, p3[i], p3);
    }
}

void init_power(int p3[10]){
    int tmp = 1;
    for (int i = 0; i < 10; ++i){
        p3[i] = tmp;
        tmp *= 3;
    }
}

void initGameBoard(int gameBoard[3][3]) {
    for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 3; ++j)
        gameBoard[i][j] = EMPTY;
}


void printGameBoard(int gameBoard[3][3]){
    printf("=========\n");
    for (int i = 0; i < 3; ++i){
        for (int j = 0; j < 3; ++j){
            if (gameBoard[i][j] == EMPTY)
                printf("|%d|", (2-i)*3 + j + 1);
            else if (gameBoard[i][j] == CIRCLE)
                printf("|O|");
            else
                printf("|X|");
        }
        printf("\n");
    }
    printf("=========\n");
}

int input_mark(char c[300], int gameBoard[3][3]){
    // trim string
    int length = strlen(c);
    char s[300];
    int st = 0, ed = length - 1;    //start and end location of trimmed string
    while (c[st] == ' ') ++st;
    while (!isalnum(c[ed])) --ed;
    for (int i = st; i <= ed; ++i) s[i-st] = c[i];
    // check if the string is number
    for (int i = 0; i < strlen(s); ++i){
        if (!isdigit(s[i])) return 0;
        // 0 is a flag to indicate that the input is invalid
    }
    // convert string to integer
    int res = 0;
    for (int i = 0; i < strlen(s); ++i){
        res *= 10;
        res += (s[i] - '0');
    }
    // range check (1-9)
    if (res < 1 || res > 9) return 0;
    // check if the grid is empty
    if (gameBoard[2-(res-1)/3][(res-1)%3] != 0) return 0;
    else return res;    //validation check passed
}

void placeMarkByHumanPlayer(int gameBoard[3][3], int mark) {
    printf("Player %d, please place your mark [1-9]:\n", mark);

    int move;
    char input[300];    //input string
    //do-while loop that prompts user to input until a valid input is obtained
    do{
        fgets(input, 300, stdin);
        move = input_mark(input, gameBoard);
        if (move != 0) break;
        printf("Invalid input! Please enter the index of an empty grid\n");
    } while (1);

    gameBoard[2-(move-1)/3][(move-1)%3] = mark;
}

int isFull(int gameBoard[3][3]){
    for (int i = 0; i < 3; ++i) for (int j =0 ; j < 3; ++j)
        if (gameBoard[i][j] == EMPTY) return 0;
    return 1;
}


void placeMarkByComputerPlayer(int gameBoard[3][3], int s[4][20000], int step, int p3[10]){
    printf("Computer places the mark:\n");
    int st = 0, mark;
    // generate encoded game state
    for (int i = 0; i < 9; ++i){
        st += p3[i] * gameBoard[i/3][i%3];
    }
    // determine where to place the mark based on the generated result
    mark = s[step/2-1][st];
    // place the mark in gameboard
    gameBoard[mark/3][mark%3] = CROSS;
}

int input_number_of_players(char c[300]){
    // trim string
    int length = strlen(c);
    char s[300];
    int st = 0, ed = length - 1;    //start and end location of trimmed string
    while (c[st] == ' ') ++st;
    while (!isalnum(c[ed])) --ed;
    for (int i = st; i <= ed; ++i) s[i-st] = c[i];
    // check if the string is number
    for (int i = 0; i < strlen(s); ++i){
        if (!isdigit(s[i])) return 0;
        // 0 is a flag to indicate that the input is invalid
    }
    // convert string to integer
    int res = 0;
    for (int i = 0; i < strlen(s); ++i){
        res *= 10;
        res += (s[i] - '0');
    }
    // fixed value check
    if (res == 1 || res == 2) return res;
    else return 0;
}


int main() {
    int gameBoard[3][3];    // Each element stores 0 (EMPTY), 1 (CIRCLE), or 2 (CROSS)
    int currentPlayer;      // 1: Player 1 (Human)   2: Player 2 (Human) or Computer Player
    int gameEnds;           // 0: The game continues   1: The game ends
    int numOfHumanPlayers;  // 1 or 2
    int state[4][20000] = {0}; // stores generated result of dfs
    int gameCount = 0;      // it is the xth step
    int p3[10];             // power of 3

    init_power(p3);         // initialize the power 3 lookup array
    initGameBoard(gameBoard);
    currentPlayer = 1;
    gameEnds = 0;

    start(state, p3);   //initialize AI by dfs

    // input number of players
    printf("How many human players [1-2]?\n");
    char input[300];    //input string
    //do-while loop that prompts user to input until a valid input is obtained
    do{
        fgets(input, 300, stdin);
        numOfHumanPlayers = input_number_of_players(input);
        if (numOfHumanPlayers != 0) break;
        printf("Invalid input! Please enter 1 or 2\n");
    } while (1);

    printGameBoard(gameBoard);
    while (isFull(gameBoard) == 0 && hasWinner(gameBoard) == 0){
        ++gameCount;
        if (currentPlayer == 1){
            placeMarkByHumanPlayer(gameBoard, CIRCLE);
        }
        else if (numOfHumanPlayers == 2){
            placeMarkByHumanPlayer(gameBoard, CROSS);
        }
        else{
            placeMarkByComputerPlayer(gameBoard, state, gameCount, p3);
        }
        currentPlayer = currentPlayer % 2 + 1;
        printGameBoard(gameBoard);
    }
    if (hasWinner(gameBoard)){
        currentPlayer = currentPlayer % 2 + 1;
        if (currentPlayer == 1)
            printf("Player 1 wins! Congratulations!\n");
        else if (numOfHumanPlayers == 2)
            printf("Player 2 wins! Congratulations!\n");
        else
            printf("Computer wins!\n");
    }
    else {
        printf("Draw game!\n");
    }
    return 0;
}
