#include <stdio.h>
#include <stdlib.h>
#include "interface.h"

// --- ANSI COLOR CODES --- 
#define COLOR_RED "\033[1;31m"
#define COLOR_BLUE "\033[94m"
#define COLOR_RESET "\033[0m"

#ifdef _WIN32
    #include <conio.h>
    #include <windows.h>
#else
    #include <termios.h>
    #include <unistd.h>
    int _getch(void) {
        struct termios oldt, newt;
        int ch;
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        ch = getchar();
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        return ch;
    }
#endif

void delayCPU(int milliseconds) {
#ifdef _WIN32
    Sleep(milliseconds);
#else
    usleep(milliseconds * 1000);
#endif
}

//functie pentru a goli input buffer ,could break the game before
void flushInput() {
#ifdef _WIN32
    while (_kbhit()) {
        _getch();
    }
#else
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    newt.c_cc[VMIN] = 0;  // Don't wait for characters
    newt.c_cc[VTIME] = 0; // Timeout immediately
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    while (getchar() != EOF); // Drain all pending characters
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif
}

const char* getPlayerName(int id, int isTeamMode) {
    if (isTeamMode) {
        const char *names4[] = {"Tu", "CPU 1", "Partener", "CPU 2"};
        return names4[id];
    } else {
        const char *names3[] = {"Tu", "CPU 1", "CPU 2"};
        return names3[id];
    }
}

const char* getSuitSymbol(int suitIdx) {
#ifdef _WIN32
    const char *suits[] = {"H", "D", "C", "S"};
#else
    const char *suits[] = {"♥", "♦", "♣", "♠"};
#endif
    return suits[suitIdx];
}

const char* getSuitColor(int suitIdx) {
    if (suitIdx == 0 || suitIdx == 1) return COLOR_RED;
    return COLOR_BLUE;
}

void printCard(Card c) {
    if (c.rank == -1) {
        printf("[Gol]\n");
        return;
    }
    printf("%s\n   +-----+\n   |%-2s   |\n   |  %s  |\n   |   %2s|\n   +-----+%s\n",
           getSuitColor(c.suit), RANKS[c.rank], getSuitSymbol(c.suit), RANKS[c.rank], COLOR_RESET);
}

void printHand(Card hand[]) {
    printf("\n");
    for (int i = 0; i < HAND_SIZE; i++) {
        if (hand[i].rank != -1) printf("%s  %d: +-----+ %s", getSuitColor(hand[i].suit), i + 1, COLOR_RESET);
    }
    printf("\n");
    for (int i = 0; i < HAND_SIZE; i++) {
        if (hand[i].rank != -1) printf("%s     |%-2s   | %s", getSuitColor(hand[i].suit), RANKS[hand[i].rank], COLOR_RESET);
    }
    printf("\n");
    for (int i = 0; i < HAND_SIZE; i++) {
        if (hand[i].rank != -1) printf("%s     |  %s  | %s", getSuitColor(hand[i].suit), getSuitSymbol(hand[i].suit), COLOR_RESET);
    }
    printf("\n");
    for (int i = 0; i < HAND_SIZE; i++) {
        if (hand[i].rank != -1) printf("%s     |   %2s| %s", getSuitColor(hand[i].suit), RANKS[hand[i].rank], COLOR_RESET);
    }
    printf("\n");
    for (int i = 0; i < HAND_SIZE; i++) {
        if (hand[i].rank != -1) printf("%s     +-----+ %s", getSuitColor(hand[i].suit), COLOR_RESET);
    }
    printf("\n");
}

void displayDeadCards(Card playedCards[MAX_PLAYERS][DECK_SIZE], int playedCount[MAX_PLAYERS], int difficulty, int isTeamMode, int numPlayers) {
    if (difficulty == 1) return;

    printf("\n--- CĂRȚI JUCATE ANTERIOR ---\n");

    if (difficulty == 2) {
        printf("Echipa Ta: ");
        for (int i = 0; i < playedCount[0]; i++) {
            printf("%s[%s%s]%s ", getSuitColor(playedCards[0][i].suit), RANKS[playedCards[0][i].rank], getSuitSymbol(playedCards[0][i].suit), COLOR_RESET);
        }
        if (isTeamMode) {
            for (int i = 0; i < playedCount[2]; i++) {
                printf("%s[%s%s]%s ", getSuitColor(playedCards[2][i].suit), RANKS[playedCards[2][i].rank], getSuitSymbol(playedCards[2][i].suit), COLOR_RESET);
            }
        }
        printf("\n");
    } else if (difficulty == 3) {
        if (isTeamMode) {
            printf("Echipa NOASTRĂ: ");
            for (int p = 0; p <= 2; p += 2) {
                for (int i = 0; i < playedCount[p]; i++) {
                    printf("%s[%s%s]%s ", getSuitColor(playedCards[p][i].suit), RANKS[playedCards[p][i].rank], getSuitSymbol(playedCards[p][i].suit), COLOR_RESET);
                }
            }
            printf("\nEchipa LOR:     ");
            for (int p = 1; p <= 3; p += 2) {
                for (int i = 0; i < playedCount[p]; i++) {
                    printf("%s[%s%s]%s ", getSuitColor(playedCards[p][i].suit), RANKS[playedCards[p][i].rank], getSuitSymbol(playedCards[p][i].suit), COLOR_RESET);
                }
            }
            printf("\n");
        } else {
            for (int p = 0; p < numPlayers; p++) {
                printf("%-7s: ", getPlayerName(p, isTeamMode));
                for (int i = 0; i < playedCount[p]; i++) {
                    printf("%s[%s%s]%s ", getSuitColor(playedCards[p][i].suit), RANKS[playedCards[p][i].rank], getSuitSymbol(playedCards[p][i].suit), COLOR_RESET);
                }
                printf("\n");
            }
        }
    }
    printf("-------------------------------\n");
}

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

int readSingleDigit() {
    flushInput();

    while (1) {
        int ch = _getch();
        if (ch >= '0' && ch <= '9') {
            int val = ch - '0';
            printf("%d\n", val);
            return val;
        }
    }
}

void waitForAnyKey() {
    flushInput();
    printf("\nApasă orice tastă pentru a continua...");
    _getch();
    printf("\n");
}

void displayRules() {
    clearScreen();
    printf("====================================================\n");
    printf("                  CUM SE JOACĂ                      \n");
    printf("====================================================\n\n");
    printf("SCOPUL: Adună 10 și Ași (1 punct fiecare).\n");
    printf("MODURI: Joacă la liber (1v1v1) sau Pe Echipe (2v2).\n\n");
    printf("REGULILE:\n");
    printf("1. Primul jucător pune o carte, stabilind valoarea 'țintă'.\n");
    printf("2. Pune o carte de aceeași valoare SAU un 7 pentru a 'tăia'.\n");
    printf("3. Dacă e tăiat, primul jucător poate 'răspunde' (cu aceeași\n");
    printf("   valoare sau 7) pentru a forța pe toată lumea să mai dea o carte.\n");
    printf("4. Ultimul care a tăiat câștigă mâna și toate punctele!\n\n");
    waitForAnyKey();
}
