#ifndef INTERFACE_H
#define INTERFACE_H

#include "septica.h"

void printCard(Card c);
void printHand(Card hand[]);
void clearScreen();
int readSingleDigit();
void waitForAnyKey();
void displayRules();

#endif // INTERFACE_H
