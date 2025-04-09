#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <thread>
#include <limits>

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

using namespace std;

struct GameState {
    vector<vector<char>> board;
    int size;
    char currentPlayer;
};

#ifdef _WIN32
char getch_custom() { return _getch(); }
#else
char getch_custom() {
    struct termios oldt, newt;
    char ch;
    tcgetattr(0, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(0, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(0, TCSANOW, &oldt);
    return ch;
}
#endif

void clear_screen() {
#ifdef _WIN32
    system("cls");
#else
    cout << "\033[2J\033[1;1H";
#endif
}

GameState init_game(int size) {
    return {vector<vector<char>>(size, vector<char>(size, ' ')), size, 'X'};
}

void draw_board(const GameState& s, int r, int c) {
    clear_screen();

    for (int i = 0; i < s.size; ++i) {
        cout << "  ";
        for (int j = 0; j < s.size; ++j) {
            cout << "+---";
        }
        cout << "+\n   ";
        for (int j = 0; j < s.size; ++j) {
            if (i == r && j == c)
                cout << "|[" << s.board[i][j] << "]";
            else
                cout << "| " << s.board[i][j] << " ";
        }
        cout << "|\n";
    }

    cout << "  ";
    for (int j = 0; j < s.size; ++j) cout << "+---";
    cout << "+\n";

    cout << "\nJoueur actuel : " << s.currentPlayer << endl;
    cout << "Utilisez les flèches pour bouger, 'e' pour placer, 'q' pour quitter.\n";
}


bool is_valid_move(const GameState& s, int r, int c) {
    return r >= 0 && c >= 0 && r < s.size && c < s.size && s.board[r][c] == ' ';
}

GameState make_move(GameState s, int r, int c) {
    s.board[r][c] = s.currentPlayer;
    s.currentPlayer = (s.currentPlayer == 'X') ? 'O' : 'X';
    return s;
}

char check_winner(const GameState& s) {
    int N = (s.size == 5) ? 4 : s.size;

    auto is_winner = [&](int i, int j, int di, int dj) {
        char start = s.board[i][j];
        if (start == ' ') return false;
        for (int k = 1; k < N; ++k) {
            int ni = i + k * di, nj = j + k * dj;
            if (ni < 0 || nj < 0 || ni >= s.size || nj >= s.size || s.board[ni][nj] != start)
                return false;
        }
        return true;
    };

    for (int i = 0; i < s.size; ++i)
        for (int j = 0; j < s.size; ++j)
            if (is_winner(i, j, 0, 1) || is_winner(i, j, 1, 0) ||
                is_winner(i, j, 1, 1) || is_winner(i, j, 1, -1))
                return s.board[i][j];

    for (const auto& row : s.board)
        for (char c : row)
            if (c == ' ') return 0;

    return 'D';
}

pair<int, int> player_input(GameState& s) {
    int r = 0, c = 0;
    draw_board(s, r, c);

    while (true) {
        char input = getch_custom();

        if (input == '\033') {
            getch_custom(); // skip [
            char arrow = getch_custom();
            if (arrow == 'A') r = max(0, r - 1);       // up
            if (arrow == 'B') r = min(s.size - 1, r + 1); // down
            if (arrow == 'D') c = max(0, c - 1);       // left
            if (arrow == 'C') c = min(s.size - 1, c + 1); // right
        } else if (input == 'e') {
            if (is_valid_move(s, r, c)) return {r, c};
        } else if (input == 'q') return {-1, -1};

        draw_board(s, r, c);
    }
}

pair<int, int> ai_easy(const GameState& s) {
    vector<pair<int, int>> empty;
    for (int i = 0; i < s.size; ++i)
        for (int j = 0; j < s.size; ++j)
            if (s.board[i][j] == ' ') empty.push_back({i, j});

    if (!empty.empty()) {
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(0, empty.size() - 1);
        return empty[dis(gen)];
    }
    return {-1, -1};
}

// Minimax simplifié pour profondeur 2
int evaluate(const GameState& s, char ai) {
    char winner = check_winner(s);
    if (winner == ai) return 10;
    if (winner == 'D') return 0;
    if (winner != 0 && winner != ai) return -10;
    return 0;
}

int minimax(GameState s, int depth, bool maximizing, char ai) {
    if (depth == 0 || check_winner(s) != 0) return evaluate(s, ai);

    int best = maximizing ? -1000 : 1000;
    for (int i = 0; i < s.size; ++i) {
        for (int j = 0; j < s.size; ++j) {
            if (s.board[i][j] == ' ') {
                s.board[i][j] = maximizing ? ai : ((ai == 'X') ? 'O' : 'X');
                int val = minimax(s, depth - 1, !maximizing, ai);
                s.board[i][j] = ' ';
                best = maximizing ? max(best, val) : min(best, val);
            }
        }
    }
    return best;
}

pair<int, int> ai_hard(GameState s) {
    char ai = s.currentPlayer;
    int bestVal = -1000;
    pair<int, int> bestMove = {-1, -1};

    for (int i = 0; i < s.size; ++i) {
        for (int j = 0; j < s.size; ++j) {
            if (s.board[i][j] == ' ') {
                s.board[i][j] = ai;
                int moveVal = minimax(s, 2, false, ai);
                s.board[i][j] = ' ';

                if (moveVal > bestVal) {
                    bestVal = moveVal;
                    bestMove = {i, j};
                }
            }
        }
    }
    return bestMove;
}

void play_game(bool vs_ai, bool hard) {
    int size = hard ? 5 : 3;
    GameState s = init_game(size);
    char winner = 0;

    while (!winner) {
        draw_board(s, -1, -1);
        if (s.currentPlayer == 'X' || !vs_ai) {
            auto [r, c] = player_input(s);
            if (r == -1) return;
            if (is_valid_move(s, r, c)) s = make_move(s, r, c);
        } else {
            cout << "L'IA réfléchit...\n";
            this_thread::sleep_for(chrono::milliseconds(500));
            auto [r, c] = hard ? ai_hard(s) : ai_easy(s);
            s = make_move(s, r, c);
        }

        winner = check_winner(s);
    }

    draw_board(s, -1, -1);
    if (winner == 'D') cout << "Match nul !\n";
    else cout << "Le gagnant est : " << winner << "\n";

    cout << "Appuyez sur Entrée pour continuer...\n";
    cin.ignore(); cin.get();
}

int main() {
    while (true) {
        clear_screen();
        cout << "== Morpion ==" << endl;
        cout << "1. Joueur vs Joueur\n";
        cout << "2. Joueur vs IA (facile)\n";
        cout << "3. Joueur vs IA (difficile)\n";
        cout << "4. Quitter\n";
        cout << "Choix : ";

        int choix;
        cin >> choix;

        if (choix == 1) play_game(false, false);
        else if (choix == 2) play_game(true, false);
        else if (choix == 3) play_game(true, true);
        else break;
    }

    return 0;
}
