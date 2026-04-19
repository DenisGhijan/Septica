#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "septica.h"
#include "interface.h"
#include "ai.h"

#ifdef _WIN32
#include <windows.h>
#endif


void printScoreHeader(int points[], int numHumans, int numPlayers, int actualDeckSize, int top) {
    printf("====================================================\n");
    if (numPlayers == 4) {
        printf("   SCOR -> Echipa 1: %d | Echipa 2: %d\n", points[0] + points[2], points[1] + points[3]);
    } else {
        printf("   SCOR -> ");
        for(int i = 0; i < numPlayers; i++) {
            printf("%s: %d ", getPlayerName(i, numHumans, numPlayers), points[i]);
            if (i < numPlayers - 1) printf("| ");
        }
        printf("\n");
    }
    printf("   CĂRȚI ÎN PACHET: %d\n", actualDeckSize - top);
    printf("====================================================\n");
}


void drawGameState(int points[], int numHumans, int numPlayers, int actualDeckSize, int top,
                   Card playedCards[MAX_PLAYERS][MAX_DECK_SIZE], int playedCount[MAX_PLAYERS],
                   int difficulty, Card tableCards[], int tablePlayers[], int tableCount) {
    clearScreen();
    printScoreHeader(points, numHumans, numPlayers, actualDeckSize, top);
    displayDeadCards(playedCards, playedCount, difficulty, numHumans, numPlayers);

    if (tableCount > 0) {
        printf("\n--- MASA CURENTĂ ---\n");
        for(int i = 0; i < tableCount; i++) {
            printf("%-10s a jucat: %s[%s%s]%s\n",
                getPlayerName(tablePlayers[i], numHumans, numPlayers),
                getSuitColor(tableCards[i].suit), RANKS[tableCards[i].rank],
                getSuitSymbol(tableCards[i].suit), COLOR_RESET);
        }
        printf("--------------------\n");
    }
}

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    srand(time(NULL));

    int numPlayers = 3;
    int numHumans = 1;
    int difficulty = 1;

    // --- MAIN MENU LOOP ---
    while (1) {
        clearScreen();
        printf("====================================================\n");
        printf("               Ș E P T I C Ă                        \n");
        printf("====================================================\n\n");
        printf("  1. Joacă Singur (1 Om vs CPU)\n");
        printf("  2. Multiplayer Local (Hot-Seat)\n");
        printf("  3. Cum se joacă\n");
        printf("  4. Ieșire\n\n");
        printf("Alege o opțiune: ");

        int mode = readSingleDigit();

        if (mode == 3) {
            displayRules();
            continue;
        } else if (mode == 4) {
            printf("Îți mulțumim pentru joc!\n");
            return 0;
        } else if (mode == 1 || mode == 2) {

            clearScreen();
            printf("Alege tipul de meci:\n");
            printf("  2. 2 Jucători (1v1)\n");
            printf("  3. 3 Jucători (1v1v1)\n");
            printf("  4. 4 Jucători (Echipe 2v2)\n\n");
            printf("Alege (2-4): ");
            while(1) {
                numPlayers = readSingleDigit();
                if(numPlayers >= 2 && numPlayers <= 4) break;
                printf("Invalid. Alege (2-4): ");
            }

            if (mode == 1) {
                numHumans = 1;
            } else {
                printf("\nCâți jucători umani vor juca? (2 - %d): ", numPlayers);
                while(1) {
                    numHumans = readSingleDigit();
                    if(numHumans >= 2 && numHumans <= numPlayers) break;
                    printf("Invalid. Alege (2 - %d): ", numPlayers);
                }
            }
            break;
        } else {
            printf("Alegere invalidă.");
            waitForAnyKey();
        }
    }

    // --- DIFFICULTY MENU ---
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
    Card deck[MAX_DECK_SIZE];
    int top = 0;

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 4; j++) {
            if (numPlayers == 3 && i == 1 && (j == 2 || j == 3)) continue;
            deck[top++] = (Card){i, j};
        }
    }

    int actualDeckSize = top;
    shuffle(deck, actualDeckSize);
    top = 0;

    Card hands[MAX_PLAYERS][HAND_SIZE];
    int points[MAX_PLAYERS] = {0, 0, 0, 0};
    Card playedCards[MAX_PLAYERS][MAX_DECK_SIZE];
    int playedCount[MAX_PLAYERS] = {0};

    for (int i = 0; i < HAND_SIZE; i++) {
        for (int p = 0; p < numPlayers; p++) hands[p][i] = deck[top++];
    }

    int turn = 0;
    int gameDone = 0;
    int lastHumanViewer = -1; // Tine minte cine a privit ultimul ecranul

    while (!gameDone) {
        int trickPoints = 0, originalRank = -1, leader = turn, currentWinner = leader, trickActive = 1, isFirstPass = 1;

        Card tableCards[MAX_DECK_SIZE];
        int tablePlayers[MAX_DECK_SIZE];
        int tableCount = 0;

        while (trickActive) {
            int leaderHasCards = 0;
            for(int i = 0; i < HAND_SIZE; i++) if(hands[leader][i].rank != -1) leaderHasCards = 1;

            if (!leaderHasCards) break;

            // ================= Faza 1: LIDERUL ÎNCEPE =================
            if (isFirstPass) {
                if (leader < numHumans) {
                    if (numHumans > 1 && lastHumanViewer != leader) {
                        promptNextPlayer(leader, numHumans, numPlayers);
                        lastHumanViewer = leader;
                    }
                    drawGameState(points, numHumans, numPlayers, actualDeckSize, top, playedCards, playedCount, difficulty, tableCards, tablePlayers, tableCount);

                    printf("\n%s, e rândul tău să începi.\nMâna ta: ", getPlayerName(leader, numHumans, numPlayers));
                    printHand(hands[leader]);

                    while(1) {
                        printf("\nAlege o carte (1-4): ");
                        int cardChoice = readSingleDigit();
                        if (cardChoice >= 1 && cardChoice <= 4 && hands[leader][cardChoice-1].rank != -1) {
                            Card playedCard = hands[leader][cardChoice-1];
                            hands[leader][cardChoice-1].rank = -1;

                            playedCards[leader][playedCount[leader]++] = playedCard;
                            tableCards[tableCount] = playedCard;
                            tablePlayers[tableCount++] = leader;

                            trickPoints += getPoints(playedCard);
                            originalRank = playedCard.rank;
                            currentWinner = leader;

                            // Desenăm starea nouă și afișăm cartea selectată
                            drawGameState(points, numHumans, numPlayers, actualDeckSize, top, playedCards, playedCount, difficulty, tableCards, tablePlayers, tableCount);
                            printf("\nAi jucat:\n"); printCard(playedCard);
                            delayCPU(800); // Pauză ca jucătorul să își admire mișcarea
                            break;
                        }
                        printf("Alegere invalidă sau loc gol.\n");
                    }
                } else {
                    drawGameState(points, numHumans, numPlayers, actualDeckSize, top, playedCards, playedCount, difficulty, tableCards, tablePlayers, tableCount);
                    printf("\n%s se gândește...\n", getPlayerName(leader, numHumans, numPlayers));
                    delayCPU(1000);

                    int cardChoice = chooseCardAI(hands[leader], -1, trickPoints, 1, currentWinner, leader, numPlayers);
                    Card playedCard = hands[leader][cardChoice];
                    hands[leader][cardChoice].rank = -1;

                    playedCards[leader][playedCount[leader]++] = playedCard;
                    tableCards[tableCount] = playedCard;
                    tablePlayers[tableCount++] = leader;

                    trickPoints += getPoints(playedCard);
                    originalRank = playedCard.rank;
                    currentWinner = leader;

                    // Arătăm cartea jucată de CPU
                    drawGameState(points, numHumans, numPlayers, actualDeckSize, top, playedCards, playedCount, difficulty, tableCards, tablePlayers, tableCount);
                    printf("\n%s a jucat:\n", getPlayerName(leader, numHumans, numPlayers)); printCard(playedCard);
                    delayCPU(1000);
                }
            }
            // ================= Faza 2: LIDERUL RĂSPUNDE LA O TĂIETURĂ =================
            else {
                if (leader < numHumans) {
                    if (numHumans > 1 && lastHumanViewer != leader) {
                        promptNextPlayer(leader, numHumans, numPlayers);
                        lastHumanViewer = leader;
                    }
                    drawGameState(points, numHumans, numPlayers, actualDeckSize, top, playedCards, playedCount, difficulty, tableCards, tablePlayers, tableCount);

                    printf("\n>>> %s a tăiat! <<<\n", getPlayerName(currentWinner, numHumans, numPlayers));
                    printf("Mâna ta: "); printHand(hands[leader]);
                    printf("\nAlege o carte pentru a răspunde (1-4) sau 0 pentru a renunța: ");
                    while(1) {
                        int replyChoice = readSingleDigit();
                        if (replyChoice == 0) {
                            trickActive = 0; break;
                        } else if (replyChoice >= 1 && replyChoice <= 4 && hands[leader][replyChoice-1].rank != -1) {
                            Card replyCard = hands[leader][replyChoice-1];
                            if(isCut(replyCard, originalRank, numPlayers)) {
                                hands[leader][replyChoice-1].rank = -1;

                                playedCards[leader][playedCount[leader]++] = replyCard;
                                tableCards[tableCount] = replyCard;
                                tablePlayers[tableCount++] = leader;

                                trickPoints += getPoints(replyCard);
                                currentWinner = leader;

                                drawGameState(points, numHumans, numPlayers, actualDeckSize, top, playedCards, playedCount, difficulty, tableCards, tablePlayers, tableCount);
                                printf("\nAi răspuns cu:\n"); printCard(replyCard);
                                delayCPU(800);
                                break;
                            } else printf("Această carte nu taie! Încearcă alta sau apasă 0:\n");
                        } else printf("Alegere invalidă sau loc gol. Mai încearcă:\n");
                    }
                } else {
                    drawGameState(points, numHumans, numPlayers, actualDeckSize, top, playedCards, playedCount, difficulty, tableCards, tablePlayers, tableCount);
                    printf("\n%s se gândește la un răspuns...\n", getPlayerName(leader, numHumans, numPlayers));
                    delayCPU(1000);

                    int replyChoice = -1;
                    for (int i = 0; i < HAND_SIZE; i++) {
                        if (hands[leader][i].rank != -1 && isCut(hands[leader][i], originalRank, numPlayers)) {
                            int isPartnerWinning = (numPlayers == 4 && (currentWinner % 2 == leader % 2));
                            int isTrump = (hands[leader][i].rank == 0 || (numPlayers == 3 && hands[leader][i].rank == 1));

                            if (isTrump && (trickPoints == 0 || isPartnerWinning)) continue;
                            replyChoice = i; break;
                        }
                    }

                    if (replyChoice != -1) {
                        Card replyCard = hands[leader][replyChoice];
                        hands[leader][replyChoice].rank = -1;

                        playedCards[leader][playedCount[leader]++] = replyCard;
                        tableCards[tableCount] = replyCard;
                        tablePlayers[tableCount++] = leader;

                        trickPoints += getPoints(replyCard);
                        currentWinner = leader;

                        drawGameState(points, numHumans, numPlayers, actualDeckSize, top, playedCards, playedCount, difficulty, tableCards, tablePlayers, tableCount);
                        printf("\n%s RĂSPUNDE cu:\n", getPlayerName(leader, numHumans, numPlayers)); printCard(replyCard);
                        delayCPU(1000);
                    } else {
                        printf("\n%s renunță.\n", getPlayerName(leader, numHumans, numPlayers));
                        delayCPU(800);
                        trickActive = 0;
                    }
                }
            }

            if (!trickActive) break;

            // ================= Faza 3: CEILALȚI JUCĂTORI RĂSPUND =================
            for (int offset = 1; offset < numPlayers; offset++) {
                int p = (leader + offset) % numPlayers;

                int hasCards = 0;
                for (int i = 0; i < HAND_SIZE; i++) if (hands[p][i].rank != -1) hasCards = 1;
                if (!hasCards) continue;

                if (p < numHumans) {
                    if (numHumans > 1 && lastHumanViewer != p) {
                        promptNextPlayer(p, numHumans, numPlayers);
                        lastHumanViewer = p;
                    }
                    drawGameState(points, numHumans, numPlayers, actualDeckSize, top, playedCards, playedCount, difficulty, tableCards, tablePlayers, tableCount);

                    printf("\n%s, e rândul tău.\nMâna ta: ", getPlayerName(p, numHumans, numPlayers));
                    printHand(hands[p]);
                    while(1) {
                        printf("\nAlege o carte (1-4): ");
                        int cardChoice = readSingleDigit();
                        if (cardChoice >= 1 && cardChoice <= 4 && hands[p][cardChoice-1].rank != -1) {
                            Card playedCard = hands[p][cardChoice-1];
                            hands[p][cardChoice-1].rank = -1;

                            playedCards[p][playedCount[p]++] = playedCard;
                            tableCards[tableCount] = playedCard;
                            tablePlayers[tableCount++] = p;

                            trickPoints += getPoints(playedCard);
                            if (isCut(playedCard, originalRank, numPlayers)) currentWinner = p;

                            drawGameState(points, numHumans, numPlayers, actualDeckSize, top, playedCards, playedCount, difficulty, tableCards, tablePlayers, tableCount);
                            printf("\nAi jucat:\n"); printCard(playedCard);
                            delayCPU(800);
                            break;
                        }
                        printf("Alegere invalidă sau loc gol.\n");
                    }
                } else {
                    drawGameState(points, numHumans, numPlayers, actualDeckSize, top, playedCards, playedCount, difficulty, tableCards, tablePlayers, tableCount);
                    printf("\n%s se gândește...\n", getPlayerName(p, numHumans, numPlayers));
                    delayCPU(1000);

                    int cardChoice = chooseCardAI(hands[p], originalRank, trickPoints, 0, currentWinner, p, numPlayers);
                    Card playedCard = hands[p][cardChoice];
                    hands[p][cardChoice].rank = -1;

                    playedCards[p][playedCount[p]++] = playedCard;
                    tableCards[tableCount] = playedCard;
                    tablePlayers[tableCount++] = p;

                    trickPoints += getPoints(playedCard);
                    if (isCut(playedCard, originalRank, numPlayers)) currentWinner = p;

                    drawGameState(points, numHumans, numPlayers, actualDeckSize, top, playedCards, playedCount, difficulty, tableCards, tablePlayers, tableCount);
                    printf("\n%s a jucat:\n", getPlayerName(p, numHumans, numPlayers)); printCard(playedCard);
                    delayCPU(1000);
                }
            }

            if (currentWinner == leader) trickActive = 0;
            isFirstPass = 0;
        }

        drawGameState(points, numHumans, numPlayers, actualDeckSize, top, playedCards, playedCount, difficulty, tableCards, tablePlayers, tableCount);
        printf("\n====================================================\n");
        printf("*** %s câștigă mâna și ia %d puncte! ***\n", getPlayerName(currentWinner, numHumans, numPlayers), trickPoints);
        printf("====================================================\n");
        points[currentWinner] += trickPoints;

        for (int offset = 0; offset < numPlayers; offset++) {
            int p = (currentWinner + offset) % numPlayers;
            for (int i = 0; i < HAND_SIZE; i++) {
                if (hands[p][i].rank == -1 && top < actualDeckSize) {
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

    if (numPlayers == 4) {
        int finalTeamUs = points[0] + points[2];
        int finalTeamThem = points[1] + points[3];
        printf("Scor Final -> Echipa 1: %d | Echipa 2: %d\n", finalTeamUs, finalTeamThem);

        if (finalTeamUs > finalTeamThem) printf("\nFelicitări! Echipa 1 a câștigat!\n");
        else if (finalTeamUs < finalTeamThem) printf("\nEchipa 2 a fost mai bună de data aceasta.\n");
        else printf("\nEste o remiză perfectă (4 la 4)!\n");
    } else {
        printf("Scor Final -> ");
        for (int i = 0; i < numPlayers; i++) {
            printf("%s: %d ", getPlayerName(i, numHumans, numPlayers), points[i]);
            if (i < numPlayers - 1) printf("| ");
        }
        printf("\n");

        int maxScore = points[0];
        for(int i = 1; i < numPlayers; i++) if(points[i] > maxScore) maxScore = points[i];

        int winnersCount = 0;
        int lastWinnerId = -1;
        for (int i = 0; i < numPlayers; i++) {
            if (points[i] == maxScore) {
                winnersCount++;
                lastWinnerId = i;
            }
        }

        if (winnersCount > 1) printf("\nEste egalitate pe primul loc cu %d puncte!\n", maxScore);
        else printf("\nFelicitări, %s a câștigat!\n", getPlayerName(lastWinnerId, numHumans, numPlayers));
    }

    return 0;
}
