#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// --- OS-SPECIFIC INSTANT INPUT SETUP ---
#ifdef _WIN32
    #include <conio.h>
#else
    #include <termios.h>
    #include <unistd.h>
    // Custom _getch for Mac/Linux
    int _getch(void) {
        struct termios oldt, newt;
        int ch;
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO); // Disable line buffering and echo
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        ch = getchar();
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        return ch;
    }
#endif

#define DECK_SIZE 32
#define HAND_SIZE 4
#define PLAYERS 3

const char *RANKS[] = {"7", "8", "9", "10", "J", "Q", "K", "A"};
const char *SUITS[] = {"Hearts", "Diamonds", "Clubs", "Spades"};
const char *NAMES[] = {"You", "CPU 1", "CPU 2"};

typedef struct {
    int rank;
    int suit;
} Card;

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

void printCard(Card c) {
    if (c.rank == -1) printf("[Empty]");
    else printf("[%s of %s]", RANKS[c.rank], SUITS[c.suit]);
}

void printHand(Card hand[]) {
    for (int i = 0; i < HAND_SIZE; i++) {
        if (hand[i].rank != -1) {
            printf("%d:", i + 1); printCard(hand[i]); printf("  ");
        }
    }
}

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// NEW: Reads exactly one keystroke instantly. Only accepts numbers.
int readSingleDigit() {
    while (1) {
        int ch = _getch();
        if (ch >= '0' && ch <= '9') {
            int val = ch - '0';
            printf("%d\n", val); // Print the number so the user sees what they typed
            return val;
        }
    }
}

// NEW: Snappy pause
void waitForAnyKey() {
    printf("\nPress any key to continue...");
    _getch();
    printf("\n");
}

void displayRules() {
    clearScreen();
    printf("====================================================\n");
    printf("                  HOW TO PLAY                       \n");
    printf("====================================================\n\n");
    printf("THE GOAL:\n");
    printf("Collect points! There are 8 total points in the deck:\n");
    printf("All four 10s (1 pt each) and all four Aces (1 pt each).\n\n");

    printf("THE RULES:\n");
    printf("1. The leader plays a card. The rank of this card becomes\n");
    printf("   the 'target' for the trick.\n");
    printf("2. The other players must play a card.\n");
    printf("3. If someone plays a card matching the target rank, OR\n");
    printf("   plays a 7, they 'cut' the trick and are currently winning.\n");
    printf("4. If the trick is cut, the leader gets a chance to 'reply'\n");
    printf("   by playing another matching card or a 7. If they do, the\n");
    printf("   trick continues and the opponents MUST play another card.\n");
    printf("5. Whoever last cut the trick wins all the points inside it!\n");
    printf("6. Players draw back up to 4 cards, and the winner leads next.\n\n");

    waitForAnyKey();
}

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

int main() {
    srand(time(NULL));

    // --- MAIN MENU LOOP ---
    int choice = 0;
    while (choice != 1) {
        clearScreen();
        printf("====================================================\n");
        printf("               S E P T I C A                        \n");
        printf("====================================================\n\n");
        printf("  1. Start Game\n");
        printf("  2. How to Play\n");
        printf("  3. Quit\n\n");
        printf("Choose an option: ");

        choice = readSingleDigit();

        if (choice == 2) {
            displayRules();
        } else if (choice == 3) {
            printf("Thanks for playing!\n");
            return 0;
        } else if (choice != 1) {
            printf("Invalid choice.");
            waitForAnyKey();
        }
    }

    // --- INITIALIZE GAME ---
    Card deck[DECK_SIZE];
    int top = 0;

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 4; j++) { deck[top++] = (Card){i, j}; }
    }
    shuffle(deck);
    top = 0;

    Card hands[PLAYERS][HAND_SIZE];
    int points[PLAYERS] = {0, 0, 0};

    for (int i = 0; i < HAND_SIZE; i++) {
        for (int p = 0; p < PLAYERS; p++) hands[p][i] = deck[top++];
    }

    int turn = 0;
    int gameDone = 0;

    while (!gameDone) {
        clearScreen();
        printf("====================================================\n");
        printf("   SCORE -> You: %d | CPU 1: %d | CPU 2: %d\n", points[0], points[1], points[2]);
        printf("   CARDS IN DECK: %d\n", DECK_SIZE - top);
        printf("====================================================\n");

        int trickPoints = 0;
        int originalRank = -1;
        int leader = turn;
        int currentWinner = leader;
        int trickActive = 1;
        int isFirstPass = 1;

        while (trickActive) {
            int cardsPlayedThisPass = 0;

            // --- 1. THE LEADER'S TURN ---
            int leaderHasCards = 0;
            for(int i = 0; i < HAND_SIZE; i++) if(hands[leader][i].rank != -1) leaderHasCards = 1;

            if (!leaderHasCards) break;

            if (isFirstPass) {
                if (leader == 0) {
                    printf("\n%s's turn to lead.\nYour hand: ", NAMES[leader]);
                    printHand(hands[0]);
                    while(1) {
                        printf("\nChoose a card to lead (1-4): ");
                        int cardChoice = readSingleDigit();
                        if (cardChoice >= 1 && cardChoice <= 4 && hands[0][cardChoice-1].rank != -1) {
                            Card playedCard = hands[0][cardChoice-1];
                            hands[0][cardChoice-1].rank = -1;
                            printf("You played: "); printCard(playedCard); printf("\n");
                            trickPoints += getPoints(playedCard);
                            originalRank = playedCard.rank;
                            currentWinner = leader;
                            cardsPlayedThisPass++;
                            break;
                        }
                        printf("Invalid choice or empty slot. Try again.\n");
                    }
                } else {
                    int cardChoice = chooseCardAI(hands[leader], -1, trickPoints, 1);
                    Card playedCard = hands[leader][cardChoice];
                    hands[leader][cardChoice].rank = -1;
                    printf("\n%s leads with: ", NAMES[leader]); printCard(playedCard); printf("\n");
                    trickPoints += getPoints(playedCard);
                    originalRank = playedCard.rank;
                    currentWinner = leader;
                    cardsPlayedThisPass++;
                }
            } else {
                if (leader == 0) {
                    printf("\n>>> %s cut the trick! <<<\n", NAMES[currentWinner]);
                    printf("Your hand: "); printHand(hands[0]);
                    printf("\nChoose a card to reply and continue (1-4) or 0 to give up: ");
                    while(1) {
                        int replyChoice = readSingleDigit();
                        if (replyChoice == 0) {
                            trickActive = 0;
                            break;
                        } else if (replyChoice >= 1 && replyChoice <= 4 && hands[0][replyChoice-1].rank != -1) {
                            Card replyCard = hands[0][replyChoice-1];
                            if(isCut(replyCard, originalRank)) {
                                hands[0][replyChoice-1].rank = -1;
                                printf("You replied with: "); printCard(replyCard); printf("\n");
                                trickPoints += getPoints(replyCard);
                                currentWinner = leader;
                                cardsPlayedThisPass++;
                                break;
                            } else {
                                printf("That card doesn't cut! Try a different card or 0 to give up:\n");
                            }
                        } else {
                            printf("Invalid choice or empty slot. Try again:\n");
                        }
                    }
                } else {
                    int replyChoice = -1;
                    for (int i = 0; i < HAND_SIZE; i++) {
                        if (hands[leader][i].rank != -1 && isCut(hands[leader][i], originalRank)) {
                            if (hands[leader][i].rank == 0 && trickPoints == 0) continue;
                            replyChoice = i; break;
                        }
                    }
                    if (replyChoice != -1) {
                        Card replyCard = hands[leader][replyChoice];
                        hands[leader][replyChoice].rank = -1;
                        printf("\n%s REPLIES with: ", NAMES[leader]); printCard(replyCard); printf("\n");
                        trickPoints += getPoints(replyCard);
                        currentWinner = leader;
                        cardsPlayedThisPass++;
                    } else {
                        printf("\n%s gives up the trick.\n", NAMES[leader]);
                        trickActive = 0;
                    }
                }
            }

            if (!trickActive) break;

            // --- 2. THE OPPONENTS FOLLOW ---
            for (int offset = 1; offset < PLAYERS; offset++) {
                int p = (leader + offset) % PLAYERS;

                int hasCards = 0;
                for (int i = 0; i < HAND_SIZE; i++) if (hands[p][i].rank != -1) hasCards = 1;
                if (!hasCards) continue;

                if (p == 0) {
                    printf("\nYour turn to follow.\nYour hand: ");
                    printHand(hands[0]);
                    while(1) {
                        printf("\nChoose a card to play (1-4): ");
                        int cardChoice = readSingleDigit();
                        if (cardChoice >= 1 && cardChoice <= 4 && hands[0][cardChoice-1].rank != -1) {
                            Card playedCard = hands[0][cardChoice-1];
                            hands[0][cardChoice-1].rank = -1;
                            printf("You played: "); printCard(playedCard); printf("\n");
                            trickPoints += getPoints(playedCard);
                            if (isCut(playedCard, originalRank)) currentWinner = p;
                            cardsPlayedThisPass++;
                            break;
                        }
                        printf("Invalid choice or empty slot. Try again.\n");
                    }
                } else {
                    int cardChoice = chooseCardAI(hands[p], originalRank, trickPoints, 0);
                    Card playedCard = hands[p][cardChoice];
                    hands[p][cardChoice].rank = -1;
                    printf("%s played: ", NAMES[p]); printCard(playedCard); printf("\n");
                    trickPoints += getPoints(playedCard);
                    if (isCut(playedCard, originalRank)) currentWinner = p;
                    cardsPlayedThisPass++;
                }
            }

            if (currentWinner == leader) {
                trickActive = 0;
            }

            isFirstPass = 0;
        }

        printf("\n====================================================\n");
        printf("*** %s wins the trick and takes %d points! ***\n", NAMES[currentWinner], trickPoints);
        printf("====================================================\n");
        points[currentWinner] += trickPoints;

        for (int offset = 0; offset < PLAYERS; offset++) {
            int p = (currentWinner + offset) % PLAYERS;
            for (int i = 0; i < HAND_SIZE; i++) {
                if (hands[p][i].rank == -1 && top < DECK_SIZE) {
                    hands[p][i] = deck[top++];
                }
            }
        }

        turn = currentWinner;

        gameDone = 1;
        for(int p = 0; p < PLAYERS; p++) {
            for(int i = 0; i < HAND_SIZE; i++) {
                if(hands[p][i].rank != -1) gameDone = 0;
            }
        }

        if (!gameDone) waitForAnyKey();
    }

    clearScreen();
    printf("\n====================================================\n");
    printf("                  === GAME OVER ===                 \n");
    printf("====================================================\n");
    printf("Final Scores -> You: %d | CPU 1: %d | CPU 2: %d\n", points[0], points[1], points[2]);

    // --- ACCURATE ENDGAME LOGIC ---
    int maxScore = points[0];
    if (points[1] > maxScore) maxScore = points[1];
    if (points[2] > maxScore) maxScore = points[2];

    int winnersCount = 0;
    for (int i = 0; i < PLAYERS; i++) {
        if (points[i] == maxScore) winnersCount++;
    }

    if (winnersCount > 1) {
        printf("\nIt's a tie for first place with %d points!\n", maxScore);
    } else if (points[0] == maxScore) {
        printf("\nCongratulations, you won!\n");
    } else if (points[1] == maxScore) {
        printf("\nCPU 1 outsmarted you this time.\n");
    } else {
        printf("\nCPU 2 outsmarted you this time.\n");
    }

    return 0;
}
