#include <stdio.h>
#include <stdlib.h>
#include "interface.h"

// --- OS-SPECIFIC SETUP ---
#ifdef _WIN32
    #include <conio.h>
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

// get the correct suit symbol based on OS
const char* getSuitSymbol(int suitIdx) {
#ifdef _WIN32
    // Windows
    const char *suits[] = {"H", "D", "C", "S"};
#else
    // Mac/Linux
    const char *suits[] = {"♥", "♦", "♣", "♠"};
#endif
    return suits[suitIdx];
}

// Renders a single played card as ASCII art
void printCard(Card c) {
    if (c.rank == -1) {
        printf("[Empty]\n");
        return;
    }

    printf("\n");
    printf("   +-----+\n");
    printf("   |%-2s   |\n", RANKS[c.rank]);
    printf("   |  %s  |\n", getSuitSymbol(c.suit));
    printf("   |   %2s|\n", RANKS[c.rank]);
    printf("   +-----+\n");
}

// Renders the entire hand side-by-side
void printHand(Card hand[]) {
    printf("\n");

    // Row 0: Top border & Choice numbers
    for (int i = 0; i < HAND_SIZE; i++) {
        if (hand[i].rank != -1) printf("  %d: +-----+ ", i + 1);
    }
    printf("\n");

    // Row 1: Top-left Rank
    for (int i = 0; i < HAND_SIZE; i++) {
        if (hand[i].rank != -1) printf("     |%-2s   | ", RANKS[hand[i].rank]);
    }
    printf("\n");

    // Row 2: Center Suit
    for (int i = 0; i < HAND_SIZE; i++) {
        if (hand[i].rank != -1) printf("     |  %s  | ", getSuitSymbol(hand[i].suit));
    }
    printf("\n");

    // Row 3: Bottom-right Rank
    for (int i = 0; i < HAND_SIZE; i++) {
        if (hand[i].rank != -1) printf("     |   %2s| ", RANKS[hand[i].rank]);
    }
    printf("\n");

    // Row 4: Bottom border
    for (int i = 0; i < HAND_SIZE; i++) {
        if (hand[i].rank != -1) printf("     +-----+ ");
    }
    printf("\n");
}

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

int readSingleDigit() {
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
    printf("\nPress any key to continue...");
    _getch();
    printf("\n");
}

void displayRules() {
    clearScreen();
    printf("====================================================\n");
    printf("                  HOW TO PLAY                       \n");
    printf("====================================================\n\n");
    printf("THE GOAL:\n");
    printf("Collect points! There are 8 total points in the deck:\n");
    printf("All four 10s (1 pt each) and all four Aces (1 pt each).\n\n");
    
    printf("THE RULES:\n");
    printf("1. The leader plays a card. The rank of this card becomes\n");
    printf("   the 'target' for the trick.\n");
    printf("2. The other players must play a card.\n");
    printf("3. If someone plays a card matching the target rank, OR\n");
    printf("   plays a 7, they 'cut' the trick and are currently winning.\n");
    printf("4. If the trick is cut, the leader gets a chance to 'reply'\n");
    printf("   by playing another matching card or a 7. If they do, the\n");
    printf("   trick continues and the opponents MUST play another card.\n");
    printf("5. Whoever last cut the trick wins all the points inside it!\n");
    printf("6. Players draw back up to 4 cards, and the winner leads next.\n\n");
    
    waitForAnyKey();
}
