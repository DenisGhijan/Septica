#include "ai.h"

int chooseCardAI(Card hand[], int originalRank, int trickPoints, int isLeader) {
    int hasMatch = -1, hasSeven = -1, hasTrash = -1, fallback = -1;

    for (int i = 0; i < HAND_SIZE; i++) {
        if (hand[i].rank != -1) {
            fallback = i;
            if (hand[i].rank == originalRank) hasMatch = i;
            else if (hand[i].rank == 0) hasSeven = i;
            else if (hand[i].rank != 3 && hand[i].rank != 7) hasTrash = i;
        }
    }

    if (isLeader) {
        if (hasTrash != -1) return hasTrash; 
        return fallback; 
    }

    if (hasMatch != -1) return hasMatch; 
    if (hasSeven != -1 && trickPoints > 0) return hasSeven; 
    
    if (originalRank != -1 && !isLeader) {
        if (hasTrash != -1) return hasTrash; 
    }
    
    return fallback;
}