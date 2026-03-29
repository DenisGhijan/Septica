#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "septica.h"
#include "interface.h"
#include "ai.h"

#ifdef _WIN32
#include <windows.h>
#endif

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8); // Asigură afișarea corectă a diacriticelor pe Windows
#endif

    srand(time(NULL));

    int numPlayers = 3;
    int isTeamMode = 0;
    int difficulty = 1;

    // --- MAIN MENU LOOP ---
    while (1) {
        clearScreen();
        printf("====================================================\n");
        printf("               Ș E P T I C Ă                        \n");
        printf("====================================================\n\n");
        printf("  1. Joacă la liber (1 v 1 v 1)\n");
        printf("  2. Joacă pe echipe (2 v 2)\n");
        printf("  3. Cum se joacă\n");
        printf("  4. Ieșire\n\n");
        printf("Alege o opțiune: ");

        int choice = readSingleDigit();

        if (choice == 1) {
            numPlayers = 3; isTeamMode = 0; break;
        } else if (choice == 2) {
            numPlayers = 4; isTeamMode = 1; break;
        } else if (choice == 3) {
            displayRules();
        } else if (choice == 4) {
            printf("Îți mulțumim pentru joc!\n");
            return 0;
        } else {
            printf("Alegere invalidă.");
            waitForAnyKey();
        }
    }

    // --- DIFFICULTY MENU LOOP ---
    while (1) {
        clearScreen();
        printf("====================================================\n");
        printf("             SELECTEAZĂ DIFICULTATEA                \n");
        printf("====================================================\n\n");
        printf("  1. Normal      (Fără istoric de cărți)\n");
        printf("  2. Ușor        (Arată cărțile jucate de echipa ta)\n");
        printf("  3. Foarte Ușor (Arată TOATE cărțile jucate)\n\n");
        printf("Alege o opțiune: ");

        difficulty = readSingleDigit();
        if (difficulty >= 1 && difficulty <= 3) break;

        printf("Alegere invalidă.");
        waitForAnyKey();
    }

    // --- INITIALIZE GAME ---
    Card deck[DECK_SIZE];
    int top = 0;

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 4; j++) { deck[top++] = (Card){i, j}; }
    }
    shuffle(deck);
    top = 0;

    Card hands[MAX_PLAYERS][HAND_SIZE];
    int points[MAX_PLAYERS] = {0, 0, 0, 0};

    Card playedCards[MAX_PLAYERS][DECK_SIZE];
    int playedCount[MAX_PLAYERS] = {0};

    for (int i = 0; i < HAND_SIZE; i++) {
        for (int p = 0; p < numPlayers; p++) hands[p][i] = deck[top++];
    }

    int turn = 0;
    int gameDone = 0;

    while (!gameDone) {
        clearScreen();
        printf("====================================================\n");
        if (isTeamMode) {
            printf("   SCOR -> Echipa NOASTRĂ: %d | Echipa LOR: %d\n", points[0] + points[2], points[1] + points[3]);
        } else {
            printf("   SCOR -> Tu: %d | CPU 1: %d | CPU 2: %d\n", points[0], points[1], points[2]);
        }
        printf("   CĂRȚI ÎN PACHET: %d\n", DECK_SIZE - top);
        printf("====================================================\n");

        displayDeadCards(playedCards, playedCount, difficulty, isTeamMode, numPlayers);

        int trickPoints = 0, originalRank = -1, leader = turn, currentWinner = leader, trickActive = 1, isFirstPass = 1;

        while (trickActive) {
            int cardsPlayedThisPass = 0;

            int leaderHasCards = 0;
            for(int i = 0; i < HAND_SIZE; i++) if(hands[leader][i].rank != -1) leaderHasCards = 1;

            if (!leaderHasCards) break;

            if (isFirstPass) {
                if (leader == 0) {
                    printf("\nE rândul tău să începi.\nMâna ta: ");
                    printHand(hands[0]);
                    while(1) {
                        printf("\nAlege o carte (1-4): ");
                        int cardChoice = readSingleDigit();
                        if (cardChoice >= 1 && cardChoice <= 4 && hands[0][cardChoice-1].rank != -1) {
                            Card playedCard = hands[0][cardChoice-1];
                            hands[0][cardChoice-1].rank = -1;

                            playedCards[0][playedCount[0]++] = playedCard;

                            printf("Ai jucat: "); printCard(playedCard);
                            trickPoints += getPoints(playedCard);
                            originalRank = playedCard.rank;
                            currentWinner = leader;
                            cardsPlayedThisPass++;
                            break;
                        }
                        printf("Alegere invalidă sau loc gol.\n");
                    }
                } else {
                    int cardChoice = chooseCardAI(hands[leader], -1, trickPoints, 1, currentWinner, leader, isTeamMode);
                    Card playedCard = hands[leader][cardChoice];
                    hands[leader][cardChoice].rank = -1;

                    playedCards[leader][playedCount[leader]++] = playedCard;
                    delayCPU(800);

                    printf("\n%s începe cu: ", getPlayerName(leader, isTeamMode)); printCard(playedCard);
                    trickPoints += getPoints(playedCard);
                    originalRank = playedCard.rank;
                    currentWinner = leader;
                    cardsPlayedThisPass++;
                }
            } else {
                if (leader == 0) {
                    printf("\n>>> %s a tăiat! <<<\n", getPlayerName(currentWinner, isTeamMode));
                    printf("Mâna ta: "); printHand(hands[0]);
                    printf("\nAlege o carte pentru a răspunde (1-4) sau 0 pentru a renunța: ");
                    while(1) {
                        int replyChoice = readSingleDigit();
                        if (replyChoice == 0) {
                            trickActive = 0; break;
                        } else if (replyChoice >= 1 && replyChoice <= 4 && hands[0][replyChoice-1].rank != -1) {
                            Card replyCard = hands[0][replyChoice-1];
                            if(isCut(replyCard, originalRank)) {
                                hands[0][replyChoice-1].rank = -1;

                                playedCards[0][playedCount[0]++] = replyCard;

                                printf("Ai răspuns cu: "); printCard(replyCard);
                                trickPoints += getPoints(replyCard);
                                currentWinner = leader;
                                cardsPlayedThisPass++;
                                break;
                            } else printf("Această carte nu taie! Încearcă alta sau apasă 0:\n");
                        } else printf("Alegere invalidă sau loc gol. Mai încearcă:\n");
                    }
                } else {
                    int replyChoice = -1;
                    for (int i = 0; i < HAND_SIZE; i++) {
                        if (hands[leader][i].rank != -1 && isCut(hands[leader][i], originalRank)) {
                            int isPartnerWinning = (isTeamMode && (currentWinner % 2 == leader % 2));
                            if (hands[leader][i].rank == 0 && (trickPoints == 0 || isPartnerWinning)) continue;
                            replyChoice = i; break;
                        }
                    }

                    delayCPU(800);

                    if (replyChoice != -1) {
                        Card replyCard = hands[leader][replyChoice];
                        hands[leader][replyChoice].rank = -1;

                        playedCards[leader][playedCount[leader]++] = replyCard;

                        printf("\n%s RĂSPUNDE cu: ", getPlayerName(leader, isTeamMode)); printCard(replyCard);
                        trickPoints += getPoints(replyCard);
                        currentWinner = leader;
                        cardsPlayedThisPass++;
                    } else {
                        printf("\n%s renunță.\n", getPlayerName(leader, isTeamMode));
                        trickActive = 0;
                    }
                }
            }

            if (!trickActive) break;

            for (int offset = 1; offset < numPlayers; offset++) {
                int p = (leader + offset) % numPlayers;

                int hasCards = 0;
                for (int i = 0; i < HAND_SIZE; i++) if (hands[p][i].rank != -1) hasCards = 1;
                if (!hasCards) continue;

                if (p == 0) {
                    printf("\nE rândul tău.\nMâna ta: ");
                    printHand(hands[0]);
                    while(1) {
                        printf("\nAlege o carte (1-4): ");
                        int cardChoice = readSingleDigit();
                        if (cardChoice >= 1 && cardChoice <= 4 && hands[0][cardChoice-1].rank != -1) {
                            Card playedCard = hands[0][cardChoice-1];
                            hands[0][cardChoice-1].rank = -1;

                            playedCards[0][playedCount[0]++] = playedCard;

                            printf("Ai jucat: "); printCard(playedCard);
                            trickPoints += getPoints(playedCard);
                            if (isCut(playedCard, originalRank)) currentWinner = p;
                            cardsPlayedThisPass++;
                            break;
                        }
                        printf("Alegere invalidă sau loc gol.\n");
                    }
                } else {
                    int cardChoice = chooseCardAI(hands[p], originalRank, trickPoints, 0, currentWinner, p, isTeamMode);
                    Card playedCard = hands[p][cardChoice];
                    hands[p][cardChoice].rank = -1;

                    playedCards[p][playedCount[p]++] = playedCard;
                    delayCPU(800);

                    printf("%s a jucat: ", getPlayerName(p, isTeamMode)); printCard(playedCard);
                    trickPoints += getPoints(playedCard);
                    if (isCut(playedCard, originalRank)) currentWinner = p;
                    cardsPlayedThisPass++;
                }
            }

            if (currentWinner == leader) trickActive = 0;
            isFirstPass = 0;
        }

        printf("\n====================================================\n");
        printf("*** %s câștigă mâna și ia %d puncte! ***\n", getPlayerName(currentWinner, isTeamMode), trickPoints);
        printf("====================================================\n");
        points[currentWinner] += trickPoints;

        for (int offset = 0; offset < numPlayers; offset++) {
            int p = (currentWinner + offset) % numPlayers;
            for (int i = 0; i < HAND_SIZE; i++) {
                if (hands[p][i].rank == -1 && top < DECK_SIZE) {
                    hands[p][i] = deck[top++];
                }
            }
        }

        turn = currentWinner;

        gameDone = 1;
        for(int p = 0; p < numPlayers; p++) {
            for(int i = 0; i < HAND_SIZE; i++) {
                if(hands[p][i].rank != -1) gameDone = 0;
            }
        }

        waitForAnyKey();
    }

    clearScreen();
    printf("\n====================================================\n");
    printf("              === JOCUL S-A TERMINAT ===            \n");
    printf("====================================================\n");

    if (isTeamMode) {
        int finalTeamUs = points[0] + points[2];
        int finalTeamThem = points[1] + points[3];
        printf("Scor Final -> Echipa NOASTRĂ: %d | Echipa LOR: %d\n", finalTeamUs, finalTeamThem);

        if (finalTeamUs > finalTeamThem) printf("\nFelicitări! Tu și Partenerul ați câștigat!\n");
        else if (finalTeamUs < finalTeamThem) printf("\nEchipa LOR a fost mai bună de data aceasta.\n");
        else printf("\nEste o remiză perfectă (4 la 4)!\n");
    } else {
        printf("Scor Final -> Tu: %d | CPU 1: %d | CPU 2: %d\n", points[0], points[1], points[2]);
        int maxScore = points[0];
        if (points[1] > maxScore) maxScore = points[1];
        if (points[2] > maxScore) maxScore = points[2];

        int winnersCount = 0;
        for (int i = 0; i < numPlayers; i++) if (points[i] == maxScore) winnersCount++;

        if (winnersCount > 1) printf("\nEste egalitate pe primul loc cu %d puncte!\n", maxScore);
        else if (points[0] == maxScore) printf("\nFelicitări, ai câștigat!\n");
        else if (points[1] == maxScore) printf("\nCPU 1 a câștigat de data aceasta.\n");
        else printf("\nCPU 2 a câștigat de data aceasta.\n");
    }

    return 0;
}
