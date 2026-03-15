#ifndef SEPTICA_H
#define SEPTICA_H

// Game configuration constants
#define DECK_SIZE 32
#define HAND_SIZE 4
#define PLAYERS 3

// Data structures
typedef struct {
    int rank;
    int suit;
} Card;

// Global arrays (extern tells the compiler these are defined in the .c file)
extern const char *RANKS[];
extern const char *SUITS[];
extern const char *NAMES[];

// Function Prototypes (The "Table of Contents")
int getPoints(Card c);
int isCut(Card c, int originalRank);
void shuffle(Card *deck);
void printCard(Card c);
void printHand(Card hand[]);
void clearScreen();
int readSingleDigit();
void waitForAnyKey();
void displayRules();
int chooseCardAI(Card hand[], int originalRank, int trickPoints, int isLeader);

#endif // SEPTICA_H
