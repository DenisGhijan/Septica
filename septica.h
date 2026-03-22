#ifndef GAME_H
#define GAME_H

#define DECK_SIZE 32
#define HAND_SIZE 4
#define PLAYERS 3

typedef struct {
    int rank;
    int suit;
} Card;

// Global constants
extern const char *RANKS[];
extern const char *SUITS[];
extern const char *NAMES[];

// Core mechanics
int getPoints(Card c);
int isCut(Card c, int originalRank);
void shuffle(Card *deck);

#endif // GAME_H
