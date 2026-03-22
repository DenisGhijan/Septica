#include <stdio.h>
#include <stdlib.h>
#include "interface.h"

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

void printCard(Card c) {
    if (c.rank == -1) printf("[Empty]");
    else printf("[%s of %s]", RANKS[c.rank], SUITS[c.suit]);
}

void printHand(Card hand[]) {
    for (int i = 0; i < HAND_SIZE; i++) {
        if (hand[i].rank != -1) {
            printf("%d:", i + 1); printCard(hand[i]); printf("  ");
        }
    }
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