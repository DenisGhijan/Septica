#include <stdlib.h>
#include "septica.h"

const char *RANKS[] = {"7", "8", "9", "10", "J", "Q", "K", "A"};
const char *SUITS[] = {"Hearts", "Diamonds", "Clubs", "Spades"};

int getPoints(Card c) {
    return (c.rank == 3 || c.rank == 7) ? 1 : 0;
}

int isCut(Card c, int originalRank, int numPlayers) {
    if (c.rank == originalRank || c.rank == 0) return 1;
    // În 3 jucători, cei 8 (indexul 1) taie și ei!
    if (numPlayers == 3 && c.rank == 1) return 1;
    return 0;
}

void shuffle(Card *deck, int size) {
    for (int i = 0; i < size; i++) {
        int j = i + rand() / (RAND_MAX / (size - i) + 1);
        Card t = deck[j]; deck[j] = deck[i]; deck[i] = t;
    }
}
