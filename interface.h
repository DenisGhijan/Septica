#ifndef INTERFACE_H
#define INTERFACE_H

#include "septica.h"

const char* getPlayerName(int id, int isTeamMode);
void printCard(Card c);
void printHand(Card hand[]);
void clearScreen();
int readSingleDigit();
void waitForAnyKey();
void displayRules();
void delayCPU(int milliseconds);

// Carti jucate anterior
void displayDeadCards(Card playedCards[MAX_PLAYERS][DECK_SIZE], int playedCount[MAX_PLAYERS], int difficulty, int isTeamMode, int numPlayers);

#endif // INTERFACE_H
