#include <stdio.h>
#include <stdlib.h> // Pour malloc, free, rand, srand, system
#include <stdbool.h> // Pour le type bool (C99+)
#include <time.h>    // Pour srand(time(NULL))
#include <limits.h>  // Pour INT_MIN, INT_MAX (remplace numeric_limits)

#ifdef _WIN32
#include <conio.h> // Pour _getch
#include <windows.h> // Pour Sleep
#else
#include <termios.h> // Pour termios, tcgetattr, tcsetattr
#include <unistd.h>  // Pour read, usleep
#endif

// Structure pour représenter un coup (remplace std::pair<int, int>)
typedef struct {
    int r;
    int c;
} Move;

// Structure pour l'état du jeu
typedef struct {
    char **board; // Remplacé vector<vector<char>> par char**
    int size;
    char currentPlayer;
} GameState;

// --- Gestion de l'entrée non bufferisée ---
#ifdef _WIN32
char getch_custom() { return _getch(); }
void sleep_ms(int milliseconds) { Sleep(milliseconds); }
#else
char getch_custom() {
    struct termios oldt, newt;
    char ch;
    tcgetattr(STDIN_FILENO, &oldt); // Utiliser STDIN_FILENO
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    // Utiliser read pour éviter les problèmes potentiels de bufferisation avec getchar
    if (read(STDIN_FILENO, &ch, 1) < 0) {
         perror("read()");
         ch = EOF; // Retourner une erreur en cas de problème
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}
void sleep_ms(int milliseconds) { usleep(milliseconds * 1000); }
#endif

// --- Fonctions utilitaires ---
void clear_screen() {
#ifdef _WIN32
    system("cls");
#else
    // Utiliser printf au lieu de cout
    printf("\033[2J\033[1;1H");
    fflush(stdout); // S'assurer que la sortie est immédiate
#endif
}

// Fonction pour trouver le max de deux entiers
int max(int a, int b) {
    return (a > b) ? a : b;
}

// Fonction pour trouver le min de deux entiers
int min(int a, int b) {
    return (a < b) ? a : b;
}


// --- Fonctions du jeu ---

// Initialise l'état du jeu (alloue la mémoire pour le plateau)
// Retourne true en cas de succès, false si l'allocation échoue
bool init_game(GameState *s, int size) {
    s->size = size;
    s->currentPlayer = 'X';
    s->board = (char **)malloc(size * sizeof(char *));
    if (s->board == NULL) {
        perror("Failed to allocate memory for board rows");
        return false;
    }

    for (int i = 0; i < size; ++i) {
        s->board[i] = (char *)malloc(size * sizeof(char));
        if (s->board[i] == NULL) {
            perror("Failed to allocate memory for board columns");
            // Libérer la mémoire déjà allouée avant de retourner l'échec
            for (int j = 0; j < i; ++j) {
                free(s->board[j]);
            }
            free(s->board);
            s->board = NULL; // Marquer comme invalide
            return false;
        }
        // Initialiser les cases à ' '
        for (int j = 0; j < size; ++j) {
            s->board[i][j] = ' ';
        }
    }
    return true;
}

// Libère la mémoire allouée pour le plateau
void free_game(GameState *s) {
    if (s->board != NULL) {
        for (int i = 0; i < s->size; ++i) {
            free(s->board[i]); // Libérer chaque ligne
        }
        free(s->board); // Libérer le tableau de pointeurs
        s->board = NULL;
    }
}

// Dessine le plateau de jeu
void draw_board(const GameState *s, int cursor_r, int cursor_c) {
    clear_screen();

    for (int i = 0; i < s->size; ++i) {
        printf("  "); // Indentation
        for (int j = 0; j < s->size; ++j) {
            printf("+---");
        }
        printf("+\n  "); // Indentation pour la ligne des cellules
        for (int j = 0; j < s->size; ++j) {
            if (i == cursor_r && j == cursor_c)
                printf("|[%c]", s->board[i][j]); // Met en évidence le curseur
            else
                printf("| %c ", s->board[i][j]);
        }
        printf("|\n");
    }

    printf("  "); // Indentation pour la dernière ligne
    for (int j = 0; j < s->size; ++j) printf("+---");
    printf("+\n");

    printf("\nJoueur actuel : %c\n", s->currentPlayer);
    printf("Utilisez les flèches pour bouger, 'e' pour placer, 'q' pour quitter.\n");
    fflush(stdout); // S'assurer que la sortie est visible immédiatement
}

// Vérifie si un coup est valide
bool is_valid_move(const GameState *s, int r, int c) {
    return r >= 0 && c >= 0 && r < s->size && c < s->size && s->board[r][c] == ' ';
}

// Applique un coup (modifie l'état passé en pointeur)
void make_move(GameState *s, int r, int c) {
    if (is_valid_move(s, r, c)) { // Vérification interne (bonne pratique)
       s->board[r][c] = s->currentPlayer;
       s->currentPlayer = (s->currentPlayer == 'X') ? 'O' : 'X';
    }
}

// Helper pour check_winner (remplace la lambda)
bool check_line(const GameState *s, int r, int c, int dr, int dc, int N) {
    char start = s->board[r][c];
    if (start == ' ') return false;
    for (int k = 1; k < N; ++k) {
        int nr = r + k * dr;
        int nc = c + k * dc;
        // Vérifier les limites AVANT d'accéder à s->board[nr][nc]
        if (nr < 0 || nc < 0 || nr >= s->size || nc >= s->size || s->board[nr][nc] != start)
            return false;
    }
    return true;
}


// Vérifie s'il y a un gagnant ou un match nul
char check_winner(const GameState *s) {
    int N = (s->size == 5) ? 4 : s->size; // Condition de victoire (4 pour 5x5, N pour NxN)
    bool board_full = true;

    for (int i = 0; i < s->size; ++i) {
        for (int j = 0; j < s->size; ++j) {
             // Vérification des lignes gagnantes possibles à partir de (i, j)
             // Seulement si la case n'est pas vide pour optimiser un peu
             if (s->board[i][j] != ' ') {
                // Horizontal (vers la droite)
                if (j <= s->size - N && check_line(s, i, j, 0, 1, N)) return s->board[i][j];
                // Vertical (vers le bas)
                if (i <= s->size - N && check_line(s, i, j, 1, 0, N)) return s->board[i][j];
                // Diagonal (bas-droite)
                if (i <= s->size - N && j <= s->size - N && check_line(s, i, j, 1, 1, N)) return s->board[i][j];
                // Anti-diagonal (bas-gauche)
                if (i <= s->size - N && j >= N - 1 && check_line(s, i, j, 1, -1, N)) return s->board[i][j];
            }

            // Vérifier si le plateau est plein
            if (s->board[i][j] == ' ') {
                board_full = false;
            }
        }
    }

    if (board_full) return 'D'; // Draw

    return 0; // Pas de gagnant, partie en cours
}


// Gère l'entrée du joueur humain
Move player_input(const GameState *s) {
    int r = 0, c = 0;
    // Trouver la première case vide comme position initiale du curseur
    bool found_start = false;
     for(int i=0; i<s->size; ++i) {
        for(int j=0; j<s->size; ++j) {
            if(s->board[i][j] == ' ') {
                r = i; c = j;
                found_start = true;
                break;
            }
        }
        if(found_start) break;
    }
    if(!found_start && s->size > 0) { // Si plateau plein, mettre au centre
        r = s->size / 2; c = s->size / 2;
    }


    draw_board(s, r, c);

    while (true) {
        char input = getch_custom();

        int nr = r, nc = c; // Nouvelles positions potentielles

        // Gérer les séquences d'échappement pour les flèches (peut varier un peu)
        if (input == '\033') { // Séquence d'échappement (Linux/macOS)
             if (getch_custom() == '[') { // Ignorer le '['
                char arrow = getch_custom();
                if (arrow == 'A') nr = max(0, r - 1);           // Haut
                else if (arrow == 'B') nr = min(s->size - 1, r + 1); // Bas
                else if (arrow == 'D') nc = max(0, c - 1);           // Gauche (attention D/C inversés vs C++)
                else if (arrow == 'C') nc = min(s->size - 1, c + 1); // Droite
            }
        } else if (input == -32 || input == 0 || input == 224 ) { // Séquence spéciale Windows (peut varier)
            char arrow = getch_custom();
             if (arrow == 72) nr = max(0, r - 1);           // Haut
             else if (arrow == 80) nr = min(s->size - 1, r + 1); // Bas
             else if (arrow == 75) nc = max(0, c - 1);           // Gauche
             else if (arrow == 77) nc = min(s->size - 1, c + 1); // Droite
        } else if (input == 'e') {
            if (is_valid_move(s, r, c)) {
                Move move = {r, c};
                return move;
            }
        } else if (input == 'q') {
             Move quit_move = {-1, -1};
             return quit_move;
        }

        // Mettre à jour la position si elle a changé
        if (nr != r || nc != c) {
            r = nr;
            c = nc;
            draw_board(s, r, c);
        }
    }
}

// IA Facile: choisit une case vide au hasard
Move ai_easy(const GameState *s) {
    Move possible_moves[s->size * s->size]; // Tableau pour stocker les coups possibles
    int count = 0;

    // Trouver toutes les cases vides
    for (int i = 0; i < s->size; ++i) {
        for (int j = 0; j < s->size; ++j) {
            if (s->board[i][j] == ' ') {
                possible_moves[count].r = i;
                possible_moves[count].c = j;
                count++;
            }
        }
    }

    if (count > 0) {
        int random_index = rand() % count; // Choisir un index aléatoire
        return possible_moves[random_index];
    }

    // Ne devrait pas arriver si check_winner est appelé avant, mais sécurité
    Move no_move = {-1, -1};
    return no_move;
}


// --- IA Difficile (Minimax) ---

int evaluate(const GameState *s, char ai_player) {
    char winner = check_winner(s);
    if (winner == ai_player) return 10;
    if (winner == 'D') return 0;
    // Si l'autre joueur gagne (winner != 0 && winner != 'D' && winner != ai_player)
    if (winner != 0 && winner != ai_player) return -10;
    return 0; // Partie en cours, score neutre pour cette profondeur limitée
}

// Fonction Minimax (attention à la gestion de l'état du jeu)
// On passe une copie locale de GameState pour ne pas modifier l'original
// pendant l'exploration récursive.
// NOTE: Ceci copie la structure, y compris le POINTEUR board, mais PAS
// les données pointées. Modifier board[i][j] affecte l'original.
// Il faut donc annuler la modification après l'appel récursif.
int minimax(GameState current_state, int depth, bool maximizing, char ai_player) {
    char winner = check_winner(&current_state);
     // Condition d'arrêt: profondeur atteinte ou partie terminée
    if (depth == 0 || winner != 0) {
        return evaluate(&current_state, ai_player);
    }

    char human_player = (ai_player == 'X') ? 'O' : 'X';

    if (maximizing) {
        int best_val = INT_MIN; // Utiliser INT_MIN de <limits.h>
        for (int i = 0; i < current_state.size; ++i) {
            for (int j = 0; j < current_state.size; ++j) {
                if (current_state.board[i][j] == ' ') {
                    current_state.board[i][j] = ai_player; // Faire le coup pour l'IA
                    int val = minimax(current_state, depth - 1, false, ai_player);
                    current_state.board[i][j] = ' '; // Annuler le coup
                    best_val = max(best_val, val);
                }
            }
        }
        return best_val;
    } else { // Minimizing player (Human's turn in simulation)
        int best_val = INT_MAX; // Utiliser INT_MAX de <limits.h>
        for (int i = 0; i < current_state.size; ++i) {
            for (int j = 0; j < current_state.size; ++j) {
                if (current_state.board[i][j] == ' ') {
                    current_state.board[i][j] = human_player; // Faire le coup pour l'humain
                    int val = minimax(current_state, depth - 1, true, ai_player);
                    current_state.board[i][j] = ' '; // Annuler le coup
                    best_val = min(best_val, val);
                }
            }
        }
        return best_val;
    }
}


// IA Difficile: trouve le meilleur coup en utilisant Minimax
Move ai_hard(const GameState *s) {
    char ai_player = s->currentPlayer;
    int best_val = INT_MIN;
    Move best_move = {-1, -1}; // Initialiser à un coup invalide

    GameState temp_state; // Créer une copie temporaire pour l'exploration
    if (!init_game(&temp_state, s->size)) {
         fprintf(stderr, "Erreur d'allocation mémoire dans ai_hard\n");
         return best_move; // Retourner un coup invalide en cas d'erreur
    }
    // Copier l'état actuel
    temp_state.currentPlayer = s->currentPlayer;
     for(int i=0; i<s->size; ++i) {
        for(int j=0; j<s->size; ++j) {
            temp_state.board[i][j] = s->board[i][j];
        }
    }


    for (int i = 0; i < s->size; ++i) {
        for (int j = 0; j < s->size; ++j) {
            if (temp_state.board[i][j] == ' ') {
                temp_state.board[i][j] = ai_player; // Essayer le coup
                // Lancer minimax pour le coup suivant de l'adversaire (minimisant)
                // Profondeur limitée à 4 pour le 5x5, 6 ou plus pour 3x3 si on veut être parfait
                int move_val = minimax(temp_state, (s->size == 5 ? 2 : 4) , false, ai_player);
                temp_state.board[i][j] = ' '; // Annuler le coup

                if (move_val > best_val) {
                    best_val = move_val;
                    best_move.r = i;
                    best_move.c = j;
                }
            }
        }
    }

    free_game(&temp_state); // Libérer la mémoire de l'état temporaire

     // Si aucun coup n'a amélioré le score (ex: plateau presque plein, seule perte évitable)
     // ou si une erreur est survenue, choisir un coup aléatoire parmi les restants.
    if (best_move.r == -1) {
        fprintf(stderr, "Warning: ai_hard n'a pas trouvé de meilleur coup, choix aléatoire.\n");
        best_move = ai_easy(s); // Utiliser l'IA facile comme fallback
    }

    return best_move;
}


// Fonction principale pour une partie
void play_game(bool vs_ai, bool hard) {
    int size = hard ? 5 : 3;
    GameState game_state;
    if (!init_game(&game_state, size)) {
        fprintf(stderr, "Impossible d'initialiser le jeu. Manque de mémoire ?\n");
        return;
    }

    char winner = 0;
    Move current_move;

    while (winner == 0) {
         // Passer -1, -1 pour ne pas afficher de curseur avant le tour du joueur
        draw_board(&game_state, -1, -1);

        if (game_state.currentPlayer == 'X' || !vs_ai) { // Tour du joueur 1 ou JcJ
            printf("Tour du joueur %c.\n", game_state.currentPlayer);
            current_move = player_input(&game_state);
            if (current_move.r == -1) { // Le joueur a quitté
                winner = 'Q'; // Marquer comme abandon
                break;
            }
            // player_input garantit que le coup est valide via sa boucle
             make_move(&game_state, current_move.r, current_move.c);
        } else { // Tour de l'IA
            printf("L'IA (%c) réfléchit...\n", game_state.currentPlayer);
            fflush(stdout);
            sleep_ms(500); // Pause pour simuler la réflexion

            if (hard) {
                current_move = ai_hard(&game_state);
            } else {
                current_move = ai_easy(&game_state);
            }

             if (current_move.r == -1) { // Si l'IA ne trouve pas de coup (ne devrait pas arriver)
                 fprintf(stderr, "Erreur : L'IA n'a pas pu jouer.\n");
                 winner = 'E'; // Erreur
                 break;
            }
            make_move(&game_state, current_move.r, current_move.c);
        }

        winner = check_winner(&game_state);
    }

    // Afficher le résultat final
    draw_board(&game_state, -1, -1); // Afficher le plateau final sans curseur
    if (winner == 'D') {
        printf("Match nul !\n");
    } else if (winner == 'Q') {
         printf("Partie abandonnée.\n");
    } else if (winner == 'E') {
         printf("Une erreur est survenue pendant le tour de l'IA.\n");
    } else {
        printf("Le gagnant est : %c !\n", winner);
    }

    printf("Appuyez sur Entrée pour continuer...\n");
    // Vider le buffer d'entrée avant de lire l'entrée finale
    int c;
    while ((c = getchar()) != '\n' && c != EOF); // Lire et jeter jusqu'à newline ou EOF
    getchar(); // Attendre l'appui sur Entrée

    free_game(&game_state); // Très important: libérer la mémoire
}

// Fonction main
int main() {
    srand(time(NULL)); // Initialiser le générateur de nombres aléatoires une seule fois

    int choix;
    while (true) {
        clear_screen();
        printf("====== Morpion C ======\n");
        printf("1. Joueur vs Joueur (3x3)\n");
        printf("2. Joueur vs IA Facile (3x3)\n");
        printf("3. Joueur vs IA Difficile (5x5)\n"); // Note: L'IA difficile est sur 5x5
        printf("4. Quitter\n");
        printf("Choix : ");

        // Lire l'entrée de manière plus sûre
        if (scanf("%d", &choix) != 1) {
             printf("Entrée invalide. Veuillez entrer un nombre.\n");
             // Vider le buffer d'entrée en cas d'erreur de scanf
             int c;
             while ((c = getchar()) != '\n' && c != EOF);
             sleep_ms(1500); // Petite pause pour voir le message d'erreur
             choix = 0; // Réinitialiser le choix pour éviter une boucle infinie si lettre entrée
        } else {
             // Vider le buffer après un scanf réussi aussi, au cas où l'utilisateur tape "1abc"
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
                printf("Au revoir !\n");
                return 0; // Sortir du programme
            default:
                printf("Choix invalide. Veuillez réessayer.\n");
                sleep_ms(1000);
                break;
        }
    }

    return 0; // Ne devrait pas être atteint à cause de la boucle infinie et du return dans case 4
}
