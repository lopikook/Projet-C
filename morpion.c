#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <limits.h>
#include <string.h>

#define COLOR_RESET   "\x1b[0m"
#define COLOR_RED     "\x1b[31m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_CYAN    "\x1b[36m"

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
char getch_custom() { return _getch(); }
void sleep_ms(int milliseconds) { Sleep(milliseconds); }
void clear_screen() { system("cls"); }
#else
#include <termios.h>
#include <unistd.h>
char getch_custom() {
    struct termios oldt, newt;
    char ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    if (read(STDIN_FILENO, &ch, 1) <= 0) {
        perror("read()");
        ch = EOF;
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}
void sleep_ms(int milliseconds) { usleep(milliseconds * 1000); }
void clear_screen() { printf("\033[2J\033[1;1H"); fflush(stdout); }
#endif

typedef struct {
    int r;
    int c;
} Move;

typedef struct {
    char **board;
    int size;
    char currentPlayer;
} GameState;

bool init_game(GameState *s, int size);
void free_game(GameState *s);
GameState copy_game_state(const GameState *original);
void draw_board(const GameState *s, int cursor_r, int cursor_c);
bool is_valid_move(const GameState *s, int r, int c);
void make_move(GameState *s, int r, int c);
char check_winner(const GameState *s);
Move player_input(const GameState *s);
Move ai_easy(const GameState *s);
Move ai_hard(const GameState *s);
int evaluate(const GameState *s, char ai_player);
int minimax(GameState current_state, int depth, bool maximizingPlayer, char ai_player);
void play_game(bool vs_ai, bool hard_mode);

int max(int a, int b) { return (a > b) ? a : b; }
int min(int a, int b) { return (a < b) ? a : b; }


bool init_game(GameState *s, int size) {
    s->size = size;
    s->currentPlayer = 'X';
    s->board = (char **)malloc(size * sizeof(char *));
    if (s->board == NULL) {
        perror("Echec allocation mémoire (lignes)");
        return false;
    }

    for (int i = 0; i < size; ++i) {
        s->board[i] = (char *)malloc(size * sizeof(char));
        if (s->board[i] == NULL) {
            perror("Echec allocation mémoire (colonnes)");
            for (int j = 0; j < i; ++j) {
                free(s->board[j]);
            }
            free(s->board);
            s->board = NULL;
            return false;
        }
        for (int j = 0; j < size; ++j) {
            s->board[i][j] = ' ';
        }
    }
    return true;
}

void free_game(GameState *s) {
    if (s->board != NULL) {
        for (int i = 0; i < s->size; ++i) {
            free(s->board[i]);
        }
        free(s->board);
        s->board = NULL;
    }
}

GameState copy_game_state(const GameState *original) {
    GameState copy;
    copy.size = original->size;
    copy.currentPlayer = original->currentPlayer;
    copy.board = (char **)malloc(copy.size * sizeof(char *));
    // Gérer l'échec d'allocation si nécessaire (simplifié ici)
    for (int i = 0; i < copy.size; ++i) {
        copy.board[i] = (char *)malloc(copy.size * sizeof(char));
        // Copier le contenu de la ligne
        memcpy(copy.board[i], original->board[i], copy.size * sizeof(char));
    }
    return copy;
}


void draw_board(const GameState *s, int cursor_r, int cursor_c) {
    clear_screen();
    printf(COLOR_CYAN "====== Morpion C ======\n\n" COLOR_RESET);

    for (int i = 0; i < s->size; ++i) {
        printf("  ");
        for (int j = 0; j < s->size; ++j) printf("+---");
        printf("+\n  ");
        for (int j = 0; j < s->size; ++j) {
            char symbol = s->board[i][j];
            const char* color = COLOR_RESET;
            if (symbol == 'X') color = COLOR_BLUE;
            else if (symbol == 'O') color = COLOR_RED;

            if (i == cursor_r && j == cursor_c) {
                 printf("|" COLOR_YELLOW "[%s%c%s]" COLOR_RESET, color, symbol, COLOR_YELLOW);
            } else {
                printf("| %s%c%s ", color, symbol, COLOR_RESET);
            }
        }
        printf("|\n");
    }

    printf("  ");
    for (int j = 0; j < s->size; ++j) printf("+---");
    printf("+\n\n");

    printf("Joueur actuel : %s%c%s\n",
           (s->currentPlayer == 'X' ? COLOR_BLUE : COLOR_RED),
           s->currentPlayer,
           COLOR_RESET);
    printf("Utilisez les flèches pour bouger, 'e' pour placer, 'q' pour quitter.\n");
    fflush(stdout);
}

bool is_valid_move(const GameState *s, int r, int c) {
    return r >= 0 && r < s->size && c >= 0 && c < s->size && s->board[r][c] == ' ';
}

void make_move(GameState *s, int r, int c) {
    if (is_valid_move(s, r, c)) {
        s->board[r][c] = s->currentPlayer;
        s->currentPlayer = (s->currentPlayer == 'X') ? 'O' : 'X';
    }
}

bool check_line(const GameState *s, int r, int c, int dr, int dc, int N) {
    char player = s->board[r][c];

    for (int k = 1; k < N; ++k) {
        int nr = r + k * dr;
        int nc = c + k * dc;
        if (nr < 0 || nr >= s->size || nc < 0 || nc >= s->size || s->board[nr][nc] != player) {
            return false;
        }
    }
    return true;
}

char check_winner(const GameState *s) {
    int N = (s->size >= 5) ? 4 : 3;
    bool board_full = true;

    for (int i = 0; i < s->size; ++i) {
        for (int j = 0; j < s->size; ++j) {
            if (s->board[i][j] == ' ') {
                board_full = false;
            } else {
                if (j <= s->size - N && check_line(s, i, j, 0, 1, N)) return s->board[i][j];
                if (i <= s->size - N && check_line(s, i, j, 1, 0, N)) return s->board[i][j];
                if (i <= s->size - N && j <= s->size - N && check_line(s, i, j, 1, 1, N)) return s->board[i][j];
                if (i <= s->size - N && j >= N - 1 && check_line(s, i, j, 1, -1, N)) return s->board[i][j];
            }
        }
    }

    if (board_full) {
        return 'D';
    }

    return 0;
}


Move player_input(const GameState *s) {
    int r = s->size / 2, c = s->size / 2;

    if (s->board[r][c] != ' ') {
       bool found_start = false;
       for(int i=0; i<s->size && !found_start; ++i) {
           for(int j=0; j<s->size; ++j) {
               if(s->board[i][j] == ' ') {
                   r = i; c = j;
                   found_start = true;
                   break;
               }
           }
       }
       if (!found_start) return (Move){-1, -1};
    }


    draw_board(s, r, c);

    while (true) {
        char input = getch_custom();
        int nr = r, nc = c;

        if (input == '\033') {
            if (getch_custom() == '[') {
                char arrow = getch_custom();
                if (arrow == 'A') nr = max(0, r - 1);
                else if (arrow == 'B') nr = min(s->size - 1, r + 1);
                else if (arrow == 'D') nc = max(0, c - 1);
                else if (arrow == 'C') nc = min(s->size - 1, c + 1);
            }
        } else if (input == -32 || input == 0 || input == 224 ) {
             char arrow = getch_custom();
             if (arrow == 72) nr = max(0, r - 1);
             else if (arrow == 80) nr = min(s->size - 1, r + 1);
             else if (arrow == 75) nc = max(0, c - 1);
             else if (arrow == 77) nc = min(s->size - 1, c + 1);
        } else if (input == 'e' || input == '\r' || input == '\n') {
            if (is_valid_move(s, r, c)) {
                return (Move){r, c};
            } else {
                printf("\a");
            }
        } else if (input == 'q') {
            return (Move){-1, -1};
        }

        if (nr != r || nc != c) {
            r = nr;
            c = nc;
            draw_board(s, r, c);
        }
    }
}

Move ai_easy(const GameState *s) {
    Move possible_moves[s->size * s->size];
    int count = 0;

    for (int i = 0; i < s->size; ++i) {
        for (int j = 0; j < s->size; ++j) {
            if (s->board[i][j] == ' ') {
                possible_moves[count++] = (Move){i, j};
            }
        }
    }

    if (count > 0) {
        int random_index = rand() % count;
        return possible_moves[random_index];
    }

    return (Move){-1, -1};
}
int evaluate(const GameState *s, char ai_player) {
    char winner = check_winner(s);
    char human_player = (ai_player == 'X') ? 'O' : 'X';

    if (winner == ai_player) {
        return 10;
    } else if (winner == human_player) {
        return -10;
    } else {
        return 0;
    }
}

int minimax(GameState current_state, int depth, bool maximizingPlayer, char ai_player) {
    char winner = check_winner(&current_state);
    char human_player = (ai_player == 'X') ? 'O' : 'X';

    if (depth == 0 || winner != 0) {
        int score = evaluate(&current_state, ai_player);
        if (score == 10) return score + depth;
        if (score == -10) return score - depth;
        return score;
    }

    if (maximizingPlayer) {
        int best_val = INT_MIN;
        for (int i = 0; i < current_state.size; ++i) {
            for (int j = 0; j < current_state.size; ++j) {
                if (current_state.board[i][j] == ' ') {
                    current_state.board[i][j] = ai_player;
                    current_state.currentPlayer = human_player;
                    int val = minimax(current_state, depth - 1, false, ai_player);
                    current_state.board[i][j] = ' ';
                    current_state.currentPlayer = ai_player;
                    best_val = max(best_val, val);
                }
            }
        }
        return best_val;
    } else {
        int best_val = INT_MAX;
        for (int i = 0; i < current_state.size; ++i) {
            for (int j = 0; j < current_state.size; ++j) {
                if (current_state.board[i][j] == ' ') {
                    current_state.board[i][j] = human_player;
                    current_state.currentPlayer = ai_player;
                    int val = minimax(current_state, depth - 1, true, ai_player);
                    current_state.board[i][j] = ' ';
                    current_state.currentPlayer = human_player;
                    best_val = min(best_val, val);
                }
            }
        }
        return best_val;
    }
}


Move ai_hard(const GameState *s) {
    char ai_player = s->currentPlayer;
    int best_val = INT_MIN;
    Move best_move = {-1, -1};
    int depth = (s->size <= 3) ? 6 : 4;

    GameState temp_state = copy_game_state(s);

    for (int i = 0; i < s->size; ++i) {
        for (int j = 0; j < s->size; ++j) {
            if (temp_state.board[i][j] == ' ') {
                temp_state.board[i][j] = ai_player;
                temp_state.currentPlayer = (ai_player == 'X' ? 'O' : 'X');

                int move_val = minimax(temp_state, depth - 1, false, ai_player);

                temp_state.board[i][j] = ' ';
                temp_state.currentPlayer = ai_player;

                if (move_val > best_val) {
                    best_val = move_val;
                    best_move = (Move){i, j};
                }
            }
        }
    }

    free_game(&temp_state);

    if (best_move.r == -1) {
        fprintf(stderr, "Warning: ai_hard n'a pas trouvé de meilleur coup, choix aléatoire.\n");
        best_move = ai_easy(s);
    }

    return best_move;
}

void play_game(bool vs_ai, bool hard_mode) {
    int size = (vs_ai && hard_mode) ? 5 : 3;
    GameState game_state;

    if (!init_game(&game_state, size)) {
        fprintf(stderr, "Impossible d'initialiser le jeu. Manque de mémoire ?\n");
        return;
    }

    char winner = 0; // 0 = en cours, 'X'/'O' = gagnant, 'D' = nul, 'Q' = Quitter, 'E' = Erreur IA
    Move current_move;

    while (winner == 0) {
        bool ai_turn = vs_ai && (game_state.currentPlayer == 'O');

        draw_board(&game_state, (ai_turn ? -1 : game_state.size / 2), (ai_turn ? -1 : game_state.size / 2));

        if (ai_turn) {
            printf("L'IA (%s%c%s) réfléchit...\n", COLOR_RED, game_state.currentPlayer, COLOR_RESET);
            fflush(stdout);
            sleep_ms(200);

            if (hard_mode) {
                current_move = ai_hard(&game_state);
            } else {
                current_move = ai_easy(&game_state);
            }

            if (current_move.r == -1) {
                fprintf(stderr, "Erreur : L'IA n'a pas pu jouer.\n");
                winner = 'E';
            } else {
                 printf("L'IA joue en (%d, %d)\n", current_move.r, current_move.c);
                 sleep_ms(500);
                 make_move(&game_state, current_move.r, current_move.c);
            }

        } else {
            printf("Tour du joueur %s%c%s.\n",
                   (game_state.currentPlayer == 'X' ? COLOR_BLUE : COLOR_RED),
                   game_state.currentPlayer, COLOR_RESET);
            current_move = player_input(&game_state);

            if (current_move.r == -1) {
                winner = 'Q';
            } else {
                make_move(&game_state, current_move.r, current_move.c);
            }
        }

        if (winner == 0) {
            winner = check_winner(&game_state);
        }
    }

    draw_board(&game_state, -1, -1);

    switch(winner) {
        case 'D': printf(COLOR_YELLOW "Match nul !\n" COLOR_RESET); break;
        case 'Q': printf(COLOR_GREEN "Partie abandonnée.\n" COLOR_RESET); break;
        case 'E': printf(COLOR_RED "Une erreur est survenue pendant le tour de l'IA.\n" COLOR_RESET); break;
        case 'X': printf("Le gagnant est : " COLOR_BLUE "%c !\n" COLOR_RESET, winner); break;
        case 'O': printf("Le gagnant est : " COLOR_RED "%c !\n" COLOR_RESET, winner); break;
        default: printf("État final inconnu ? (%c)\n", winner);
    }


    printf("Appuyez sur Entrée pour retourner au menu...\n");
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    getchar();

    free_game(&game_state);
}

int main() {
    srand(time(NULL));
    
    int choix;
    while (true) {
        clear_screen();
        printf(COLOR_CYAN "====== Morpion ======\n" COLOR_RESET);
        printf("1. Joueur vs Joueur (3x3)\n");
        printf("2. Joueur vs IA Facile (3x3)\n");
        printf("3. Joueur vs IA Difficile (5x5)\n");
        printf("4. Quitter\n");
        printf("Choix : ");

        if (scanf("%d", &choix) != 1) {
            printf(COLOR_RED "\nEntrée invalide. Veuillez entrer un nombre (1-4).\n" COLOR_RESET);
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            sleep_ms(1500);
            continue;
        } else {
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
        }

        switch (choix) {
            case 1:
                play_game(false, false);
                break;
            case 2:
                play_game(true, false);
                break;
            case 3:
                play_game(true, true);
                break;
            case 4:
                printf(COLOR_GREEN "\nAu revoir !\n" COLOR_RESET);
                return 0;
            default:
                printf(COLOR_RED "\nChoix invalide. Veuillez réessayer (1-4).\n" COLOR_RESET);
                sleep_ms(1000);
                break;
        }
    }
    return 0;
} 
