#include "ai.h"

int chooseCardAI(Card hand[], int originalRank, int trickPoints, int isLeader, int currentWinner, int myID, int isTeamMode) {
    int hasMatch = -1, hasSeven = -1, hasTrash = -1, hasPoints = -1, fallback = -1;

    for (int i = 0; i < HAND_SIZE; i++) {
        if (hand[i].rank != -1) {
            fallback = i;
            if (hand[i].rank == originalRank) hasMatch = i;
            else if (hand[i].rank == 0) hasSeven = i;
            else if (hand[i].rank == 3 || hand[i].rank == 7) hasPoints = i;
            else hasTrash = i;
        }
    }

    if (isLeader) {
        if (hasTrash != -1) return hasTrash;
        return fallback;
    }

    // 2v2 logic
    int isPartnerWinning = (isTeamMode && (currentWinner % 2 == myID % 2));

    if (isPartnerWinning) {
        if (hasPoints != -1 && originalRank != -1) return hasPoints;
        if (hasTrash != -1) return hasTrash;
        return fallback;
    }

    if (hasMatch != -1) return hasMatch;
    if (hasSeven != -1 && trickPoints > 0) return hasSeven;

    if (originalRank != -1) {
        if (hasTrash != -1) return hasTrash; 
    }
    
    return fallback;
}
