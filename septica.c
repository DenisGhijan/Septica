#include <stdlib.h>
#include "septica.h"

const char *RANKS[] = {"7", "8", "9", "10", "J", "Q", "K", "A"};
const char *SUITS[] = {"Hearts", "Diamonds", "Clubs", "Spades"};

int getPoints(Card c) {
    return (c.rank == 3 || c.rank == 7) ? 1 : 0;
}

int isCut(Card c, int originalRank) {
    return (c.rank == originalRank || c.rank == 0);
}

void shuffle(Card *deck) {
    for (int i = 0; i < DECK_SIZE; i++) {
        int j = i + rand() / (RAND_MAX / (DECK_SIZE - i) + 1);
        Card t = deck[j]; deck[j] = deck[i]; deck[i] = t;
    }
}
