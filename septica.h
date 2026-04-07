#ifndef SEPTICA_H
#define SEPTICA_H

#define MAX_DECK_SIZE 32
#define HAND_SIZE 4
#define MAX_PLAYERS 4

typedef struct {
    int rank;
    int suit;
} Card;

extern const char *RANKS[];
extern const char *SUITS[];

int getPoints(Card c);
int isCut(Card c, int originalRank, int numPlayers);
void shuffle(Card *deck, int size);

#endif // SEPTICA_H
