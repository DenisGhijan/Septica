#ifndef INTERFACE_H
#define INTERFACE_H

#include "septica.h"

const char* getPlayerName(int id, int numHumans, int numPlayers);
void promptNextPlayer(int id, int numHumans, int numPlayers);
void printCard(Card c);
void printHand(Card hand[]);
void clearScreen();
void flushInput();
int readSingleDigit();
void waitForAnyKey();
void displayRules();
void delayCPU(int milliseconds);
void displayDeadCards(Card playedCards[MAX_PLAYERS][MAX_DECK_SIZE], int playedCount[MAX_PLAYERS], int difficulty, int numHumans, int numPlayers);

// Helpers pentru printarea textelor colorate
const char* getSuitSymbol(int suitIdx);
const char* getSuitColor(int suitIdx);
#define COLOR_RESET "\033[0m"

#endif // INTERFACE_H
