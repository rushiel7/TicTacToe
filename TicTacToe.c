#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>

// Game History for Replay System
typedef struct {
    int move_number;
    int row;
    int col;
    char player;
} GameMove;

typedef struct {
    GameMove* moves;
    int move_count;
    int move_capacity;
    int board_size;
    char** initial_board; // Store initial board state for replay
} GameHistory;

// Function prototypes
char** createBoard(int size);
void freeBoard(char** board, int size);
int initializeBoard(char** board, int size);
int printBoard(char** board, int size);
int checkWin(char** board, int size);
int checkDraw(char** board, int size);
int playerMove(char** board, int size, char player, GameHistory* history);
int aiMove(char** board, int size, char ai_char, GameHistory* history);

// Replay System functions
GameHistory* createGameHistory(int board_size);
void addMoveToHistory(GameHistory* history, int row, int col, char player);
void saveReplay(const GameHistory* history, const char* filename);
void playReplay(const char* filename);
void freeGameHistory(GameHistory* history);
void saveInitialBoard(GameHistory* history, char** board, int size);
void replayGame(const GameHistory* history);

int main() {
    srand(time(NULL));
    
    int choice;
    printf("=== Enhanced Tic-Tac-Toe with Replay System ===\n");
    
    do {
        printf("\nMain Menu:\n");
        printf("1. New Game\n");
        printf("2. Play Replay\n");
        printf("3. Exit\n");
        printf("Choice: ");
        scanf("%d", &choice);
        
        switch(choice) {
            case 1: {
                // New Game
                int size;
                printf("Enter board size (3-15): ");
                scanf("%d", &size);
                
                if (size < 3 || size > 15) {
                    printf("Invalid size! Using default 3x3.\n");
                    size = 3;
                }
                
                char** board = createBoard(size);
                if (!board) {
                    printf("Failed to create board!\n");
                    break;
                }
                
                initializeBoard(board, size);
                GameHistory* history = createGameHistory(size);
                saveInitialBoard(history, board, size);
                
                // Game mode selection
                int game_mode;
                printf("\nGame Modes:\n");
                printf("1. Player vs Player\n");
                printf("2. Player vs AI\n");
                printf("Choice: ");
                scanf("%d", &game_mode);
                
                char current_player = 'X';
                int game_over = 0;
                
                while (!game_over) {
                    printf("\nCurrent Board:\n");
                    printBoard(board, size);
                    
                    int valid_move = 0;
                    if (game_mode == 1 || (game_mode == 2 && current_player == 'X')) {
                        // Human player
                        printf("Player %c's turn\n", current_player);
                        valid_move = playerMove(board, size, current_player, history);
                    } else {
                        // AI player
                        printf("AI's turn (Player %c)\n", current_player);
                        valid_move = aiMove(board, size, current_player, history);
                    }
                    
                    if (valid_move) {
                        // Check for win
                        if (checkWin(board, size)) {
                            printf("\nFinal Board:\n");
                            printBoard(board, size);
                            printf("Player %c wins!\n", current_player);
                            game_over = 1;
                        }
                        // Check for draw
                        else if (checkDraw(board, size)) {
                            printf("\nFinal Board:\n");
                            printBoard(board, size);
                            printf("It's a draw!\n");
                            game_over = 1;
                        }
                        // Switch player
                        current_player = (current_player == 'X') ? 'O' : 'X';
                    }
                }
                
                // Save replay
                char replay_filename[50];
                printf("\nEnter filename to save replay (without extension): ");
                scanf("%s", replay_filename);
                strcat(replay_filename, ".txt");
                
                saveReplay(history, replay_filename);
                printf("Game replay saved to %s\n", replay_filename);
                
                // Optional: Play the replay immediately
                char play_now;
                printf("Play replay now? (y/n): ");
                scanf(" %c", &play_now);
                if (play_now == 'y' || play_now == 'Y') {
                    playReplay(replay_filename);
                }
                
                // Cleanup
                freeBoard(board, size);
                freeGameHistory(history);
                break;
            }
            
            case 2: {
                // Play Replay
                char filename[100];
                printf("Enter replay filename: ");
                scanf("%s", filename);
                playReplay(filename);
                break;
            }
                
            case 3:
                printf("Goodbye!\n");
                break;
                
            default:
                printf("Invalid choice!\n");
        }
    } while (choice != 3);
    
    return 0;
}

// Dynamic Board Allocation
char** createBoard(int size) {
    char** board = (char**)malloc(size * sizeof(char*));
    if (!board) return NULL;
    
    for (int i = 0; i < size; i++) {
        board[i] = (char*)malloc(size * sizeof(char));
        if (!board[i]) {
            // Cleanup on failure
            for (int j = 0; j < i; j++) {
                free(board[j]);
            }
            free(board);
            return NULL;
        }
    }
    return board;
}

void freeBoard(char** board, int size) {
    if (!board) return;
    
    for (int i = 0; i < size; i++) {
        free(board[i]);
    }
    free(board);
}

// Sets up the board by filling it with empty spaces
int initializeBoard(char** board, int size) {
    for(int i = 0; i < size; i++) {
        for(int j = 0; j < size; j++) {
            board[i][j] = ' ';
        }
    }
    return 0;
}

// Prints the current state of the board
int printBoard(char** board, int size) {
    printf("\n");
    
    // Print column numbers
    printf("   ");
    for (int j = 0; j < size; j++) {
        printf(" %d  ", j + 1);
    }
    printf("\n");
    
    // Print top border
    printf("   ");
    for (int j = 0; j < size; j++) {
        printf("--- ");
    }
    printf("\n");
    
    // Print board rows with row numbers
    for(int i = 0; i < size; i++) {
        printf("%d |", i + 1);
        for(int j = 0; j < size; j++) {
            printf(" %c |", board[i][j]);
        }
        printf("\n");
        
        // Print row separator
        printf("   ");
        for (int j = 0; j < size; j++) {
            printf("--- ");
        }
        printf("\n");
    }
    return 0;
}

// Checks the board for a win condition
int checkWin(char** board, int size) {
    // Check rows
    for(int i = 0; i < size; i++) {
        char first = board[i][0];
        if (first == ' ') continue;
        
        int rowWin = 1;
        for(int j = 1; j < size; j++) {
            if (board[i][j] != first) {
                rowWin = 0;
                break;
            }
        }
        if (rowWin) return 1;
    }
    
    // Check columns
    for(int j = 0; j < size; j++) {
        char first = board[0][j];
        if (first == ' ') continue;
        
        int colWin = 1;
        for(int i = 1; i < size; i++) {
            if (board[i][j] != first) {
                colWin = 0;
                break;
            }
        }
        if (colWin) return 1;
    }
    
    // Check main diagonal
    char first = board[0][0];
    if (first != ' ') {
        int diagWin = 1;
        for(int i = 1; i < size; i++) {
            if (board[i][i] != first) {
                diagWin = 0;
                break;
            }
        }
        if (diagWin) return 1;
    }
    
    // Check anti-diagonal
    first = board[0][size-1];
    if (first != ' ') {
        int antiDiagWin = 1;
        for(int i = 1; i < size; i++) {
            if (board[i][size-1-i] != first) {
                antiDiagWin = 0;
                break;
            }
        }
        if (antiDiagWin) return 1;
    }
    
    return 0;
}

// Checks if the board is full without any winner
int checkDraw(char** board, int size) {
    if (checkWin(board, size)) {
        return 0;
    }
    
    for(int i = 0; i < size; i++) {
        for(int j = 0; j < size; j++) {
            if (board[i][j] == ' ') {
                return 0;
            }
        }
    }
    
    return 1;
}

// Handles player input and validates the move
int playerMove(char** board, int size, char player, GameHistory* history) {
    int row, col;
    
    while (1) {
        printf("Enter row and column (1-%d): ", size);
        if (scanf("%d %d", &row, &col) != 2) {
            printf("Invalid input! Please enter numbers only.\n");
            while (getchar() != '\n'); // Clear input buffer
            continue;
        }
        
        row--; col--; // Convert to 0-based indexing
        
        if (row < 0 || row >= size || col < 0 || col >= size) {
            printf("Invalid position! Please enter values between 1 and %d.\n", size);
        } else if (board[row][col] != ' ') {
            printf("Position already occupied! Choose another.\n");
        } else {
            board[row][col] = player;
            if (history) {
                addMoveToHistory(history, row, col, player);
            }
            return 1;
        }
    }
}

// Generates a random valid move for the AI player
int aiMove(char** board, int size, char ai_char, GameHistory* history) {
    int empty_cells = 0;
    
    // Count empty cells
    for(int i = 0; i < size; i++) {
        for(int j = 0; j < size; j++) {
            if (board[i][j] == ' ') {
                empty_cells++;
            }
        }
    }
    
    if (empty_cells == 0) return 0;
    
    // Choose random empty cell
    int move_index = rand() % empty_cells;
    int current_index = 0;
    
    for(int i = 0; i < size; i++) {
        for(int j = 0; j < size; j++) {
            if (board[i][j] == ' ') {
                if (current_index == move_index) {
                    board[i][j] = ai_char;
                    if (history) {
                        addMoveToHistory(history, i, j, ai_char);
                    }
                    return 1;
                }
                current_index++;
            }
        }
    }
    
    return 0;
}

// Replay System Implementation
GameHistory* createGameHistory(int board_size) {
    GameHistory* history = (GameHistory*)malloc(sizeof(GameHistory));
    if (!history) return NULL;
    
    history->move_count = 0;
    history->move_capacity = 10;
    history->board_size = board_size;
    history->initial_board = NULL;
    
    history->moves = (GameMove*)malloc(history->move_capacity * sizeof(GameMove));
    if (!history->moves) {
        free(history);
        return NULL;
    }
    
    return history;
}

void saveInitialBoard(GameHistory* history, char** board, int size) {
    if (!history) return;
    
    // Create a copy of the initial board state
    history->initial_board = createBoard(size);
    if (!history->initial_board) return;
    
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            history->initial_board[i][j] = board[i][j];
        }
    }
}

void addMoveToHistory(GameHistory* history, int row, int col, char player) {
    if (!history) return;
    
    // Resize if needed
    if (history->move_count >= history->move_capacity) {
        history->move_capacity *= 2;
        history->moves = (GameMove*)realloc(history->moves, 
                                          history->move_capacity * sizeof(GameMove));
    }
    
    history->moves[history->move_count].move_number = history->move_count + 1;
    history->moves[history->move_count].row = row;
    history->moves[history->move_count].col = col;
    history->moves[history->move_count].player = player;
    history->move_count++;
}

void saveReplay(const GameHistory* history, const char* filename) {
    if (!history) return;
    
    FILE* file = fopen(filename, "w");
    if (!file) {
        printf("Error: Could not create file %s\n", filename);
        return;
    }
    
    fprintf(file, "=== Tic-Tac-Toe Game Replay ===\n");
    fprintf(file, "Board Size: %d x %d\n", history->board_size, history->board_size);
    fprintf(file, "Total Moves: %d\n", history->move_count);
    fprintf(file, "===============================\n\n");
    
    // Save initial board state
    fprintf(file, "Initial Board:\n");
    for (int i = 0; i < history->board_size; i++) {
        fprintf(file, "|");
        for (int j = 0; j < history->board_size; j++) {
            fprintf(file, " %c |", history->initial_board[i][j]);
        }
        fprintf(file, "\n");
    }
    fprintf(file, "\n");
    
    // Save move-by-move replay
    fprintf(file, "Move History:\n");
    fprintf(file, "-------------\n");
    
    // Create a temporary board to simulate the game
    char** temp_board = createBoard(history->board_size);
    if (temp_board) {
        // Copy initial state
        for (int i = 0; i < history->board_size; i++) {
            for (int j = 0; j < history->board_size; j++) {
                temp_board[i][j] = history->initial_board[i][j];
            }
        }
        
        // Replay each move
        for (int move_idx = 0; move_idx < history->move_count; move_idx++) {
            GameMove move = history->moves[move_idx];
            temp_board[move.row][move.col] = move.player;
            
            fprintf(file, "Move %d: Player %c -> (%d, %d)\n", 
                    move.move_number, move.player, move.row + 1, move.col + 1);
            
            // Print board after this move
            for (int i = 0; i < history->board_size; i++) {
                fprintf(file, "|");
                for (int j = 0; j < history->board_size; j++) {
                    fprintf(file, " %c |", temp_board[i][j]);
                }
                fprintf(file, "\n");
            }
            fprintf(file, "\n");
            
            // Check if this move resulted in a win
            if (checkWin(temp_board, history->board_size)) {
                fprintf(file, ">>> Player %c WINS with this move! <<<\n\n", move.player);
                break;
            } else if (checkDraw(temp_board, history->board_size)) {
                fprintf(file, ">>> GAME ENDS IN A DRAW! <<<\n\n");
                break;
            }
        }
        
        freeBoard(temp_board, history->board_size);
    }
    
    fprintf(file, "=== End of Replay ===\n");
    fclose(file);
}

void playReplay(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error: Could not open replay file: %s\n", filename);
        return;
    }
    
    printf("\n=== Playing Replay: %s ===\n", filename);
    
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), file)) {
        printf("%s", buffer);
    }
    
    fclose(file);
    printf("=== End of Replay ===\n");
}

void freeGameHistory(GameHistory* history) {
    if (!history) return;
    
    if (history->moves) {
        free(history->moves);
    }
    
    if (history->initial_board) {
        freeBoard(history->initial_board, history->board_size);
    }
    
    free(history);
}