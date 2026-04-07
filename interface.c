#include <stdio.h>
#include <stdlib.h>
#include "interface.h"

#define COLOR_RED "\033[1;31m"
#define COLOR_BLUE "\033[94m"

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

void flushInput() {
#ifdef _WIN32
    while (_kbhit()) _getch();
#else
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    newt.c_cc[VMIN] = 0;
    newt.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    while (getchar() != EOF);
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif
}

// Generează dinamic numele jucătorilor în funcție de mod
const char* getPlayerName(int id, int numHumans, int numPlayers) {
    static char names[MAX_PLAYERS][20];
    int isTeamMode = (numPlayers == 4);

    if (numHumans == 1) {
        if (id == 0) return "Tu";
        if (isTeamMode && id == 2) return "Partener";
        sprintf(names[id], "CPU %d", (isTeamMode && id > 2) ? id - 1 : id);
    } else {
        if (id < numHumans) sprintf(names[id], "Jucător %d", id + 1);
        else sprintf(names[id], "CPU %d", id - numHumans + 1);
    }
    return names[id];
}

void promptNextPlayer(int id, int numHumans, int numPlayers) {
    clearScreen();
    printf("\n====================================================\n");
    printf("   PREGĂTIRE: Urmează %s\n", getPlayerName(id, numHumans, numPlayers));
    printf("====================================================\n");
    printf("\nAsigurați-vă că ceilalți jucători nu se uită la ecran!\n");
    waitForAnyKey();
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
    for (int i = 0; i < HAND_SIZE; i++) if (hand[i].rank != -1) printf("%s  %d: +-----+ %s", getSuitColor(hand[i].suit), i + 1, COLOR_RESET);
    printf("\n");
    for (int i = 0; i < HAND_SIZE; i++) if (hand[i].rank != -1) printf("%s     |%-2s   | %s", getSuitColor(hand[i].suit), RANKS[hand[i].rank], COLOR_RESET);
    printf("\n");
    for (int i = 0; i < HAND_SIZE; i++) if (hand[i].rank != -1) printf("%s     |  %s  | %s", getSuitColor(hand[i].suit), getSuitSymbol(hand[i].suit), COLOR_RESET);
    printf("\n");
    for (int i = 0; i < HAND_SIZE; i++) if (hand[i].rank != -1) printf("%s     |   %2s| %s", getSuitColor(hand[i].suit), RANKS[hand[i].rank], COLOR_RESET);
    printf("\n");
    for (int i = 0; i < HAND_SIZE; i++) if (hand[i].rank != -1) printf("%s     +-----+ %s", getSuitColor(hand[i].suit), COLOR_RESET);
    printf("\n");
}

void displayDeadCards(Card playedCards[MAX_PLAYERS][MAX_DECK_SIZE], int playedCount[MAX_PLAYERS], int difficulty, int numHumans, int numPlayers) {
    if (difficulty == 1) return;

    int isTeamMode = (numPlayers == 4);
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
            printf("Echipa 1: ");
            for (int p = 0; p <= 2; p += 2) {
                for (int i = 0; i < playedCount[p]; i++) {
                    printf("%s[%s%s]%s ", getSuitColor(playedCards[p][i].suit), RANKS[playedCards[p][i].rank], getSuitSymbol(playedCards[p][i].suit), COLOR_RESET);
                }
            }
            printf("\nEchipa 2: ");
            for (int p = 1; p <= 3; p += 2) {
                for (int i = 0; i < playedCount[p]; i++) {
                    printf("%s[%s%s]%s ", getSuitColor(playedCards[p][i].suit), RANKS[playedCards[p][i].rank], getSuitSymbol(playedCards[p][i].suit), COLOR_RESET);
                }
            }
            printf("\n");
        } else {
            for (int p = 0; p < numPlayers; p++) {
                printf("%-10s: ", getPlayerName(p, numHumans, numPlayers));
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
    printf("MODURI: Singur sau Local (2-4 jucători pe același PC).\n\n");
    printf("REGULILE:\n");
    printf("1. Primul jucător pune o carte, stabilind valoarea 'țintă'.\n");
    printf("2. Pune o carte de aceeași valoare SAU un 7 pentru a 'tăia'.\n");
    printf("3. Dacă e tăiat, liderul poate 'răspunde' pentru a forța\n");
    printf("   pe toată lumea să mai dea o carte.\n");
    printf("4. Ultimul care a tăiat câștigă mâna și punctele!\n");
    printf("5. IN 3 JUCATORI: 2 Optari sunt scoși, ceilalți 2 taie ca 7.\n\n");
    waitForAnyKey();
}
