/*
 * License: GPLv3 (General Public License Version 3)
 *
 * main() should be self-documenting enough for general understanding.
 * And so should be the functions above it.
 * macros.h has all the macros for the project.
 * engine.c and engine.h hold all the core functions, structs and enums.
 * drawdiscard.c and drawdiscard.h hold all the draw and discard logic funcs.
 * meldnewjoin.c and meldnewjoin.h hold all the meld new and join logic funcs.
 * They are a terrible hot mess. Approach with caution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h> 

#include "macros.h"
#include "engine.h"
#include "drawdiscard.h"
#include "meldnewjoin.h"

/* Sends GET_STOCK to buraco.js */
void doDrawFromDeck()
{
	printf("GET_STOCK\n");
}

/* Sends GET_DISCARD to buraco.js and updates all related variables */
void doDrawFromPile(char *json, Group *hand, Group *pile, Set *board)
{
	printf("GET_DISCARD %s\n", json);
	rmJsonFromGroup(hand, json);

	addJsonToGroup(&(board->grps[board->size]), json);
	free(json);
	json = cardToJson(pile->cards[pile->size-1]);
	addJsonToGroup(&(board->grps[board->size]), json);
	board->size++;
	free(json);

	json = groupToJson(pile);
	fprintf(stderr, "EMPTYING PILE %s\n", json);
	clearGroup(pile);
	free(json);
}

/* Sends MELD_JOIN to buraco.js and updates all related variables */
void doJoinSomeMeld(char *json, Group *hand, Set *board)
{
	int index;
	char jcard[MAX_LINE];

	printf("MELD_JOIN %s\n", json);
	sscanf(json, "%d %*c %s %*c", &index, jcard);
	free(json);
	addJsonToGroup(&(board->grps[index]), jcard);
	rmJsonFromGroup(hand, jcard);
}

/* Sends MELD_NEW to buraco.js and updates all related variables */
void doLowerNewMeld(char *json, Group *hand, Set *board)
{
	printf("MELD_NEW %s\n", json);
	addJsonToGroup(&(board->grps[board->size]), json);
	board->size++;
	rmJsonFromGroup(hand, json);
	free(json);
}

/* Sends DISCARD to buraco.js and updates all related variables */
void doDiscardCard(char *json, Group *hand, Group *pile)
{
	printf("DISCARD %s\n", json);
	rmJsonFromGroup(hand, json);
	addJsonToGroup(pile, json);
	free(json);
}

int main(void)
{
	int running = true;
	char LOVE[] = "[ 2♥ 3♥ 4♥ 5♥ 6♥ 7♥ 8♥ 9♥ 10♥ J♥ Q♥ K♥ A♥ ]";
	char *pline;
	char line[MAX_LINE];
	char myId[ID_SIZE];
	char oppId[ID_SIZE];
	Group myHand = NULL_GROUP;
	Group oppHand = NULL_GROUP;
	Group pile = NULL_GROUP;
	Set myMelds = NULL_SET;
	Set oppMelds = NULL_SET; /* Not actually used in any strategy (yet?) */

	setbuf(stdin, NULL);
	setbuf(stdout, NULL);
	setbuf(stderr, NULL);

	readline(line);                /* Reads both identifiers */
	sscanf(line, "%*s %s", oppId);
	readline(line);                /* Reads my identifier */
	strncpy(myId, line, ID_SIZE);
	readline(line);                /* Reads my initial hand */
	addJsonToGroup(&myHand, line);
	readline(line);                /* Reads top of discard pile */
	addJsonToGroup(&pile, line);

	while (running) {
		do {
			readline(line);
			if (!strncmp(line, "DISCARD", 7)) {
				/* Update pile and opponent's hand */
				addJsonToGroup(&pile, line+8);
				rmJsonFromGroup(&oppHand, line+8);
			}
			if (!strncmp(line, "GET_DISCARD", 11)) {
				/* Update pile and opponent's hand */
				rmJsonFromGroup(&oppHand, line+12);
				rmCardFromGroup(&pile, pile.cards[pile.size-1]);
				pline = groupToJson(&pile);
				addJsonToGroup(&oppHand, pline);
				free(pline);
				clearGroup(&pile);
			}
		} while (strcmp(line, myId));

		pline = NULL;
		if (pile.size > 0)
			pline = findMeldWithCard(pile.cards[pile.size-1], &myHand);
		if (pline != NULL)
			doDrawFromPile(pline, &myHand, &pile, &myMelds);
		else
			doDrawFromDeck();
		readline(line); /* Gets card or pile to add to hand */
		addJsonToGroup(&myHand, line);
		sortGroup(&myHand);

		pline = NULL;
		if (myHand.size > 0)
			pline = findMeldJoin(&myMelds, &myHand);
		while (pline != NULL) {
			doJoinSomeMeld(pline, &myHand, &myMelds);
			pline = findMeldJoin(&myMelds, &myHand);
		}

		pline = NULL;
		if (myHand.size > 0)
			pline = findMeldNew(&myHand);
		while (pline != NULL) {
			doLowerNewMeld(pline, &myHand, &myMelds);
			pline = findMeldNew(&myHand);
		}
		
		pline = NULL;
		if (myHand.size > 0) {
			pline = findBestDiscard(&myHand, &oppHand);
			doDiscardCard(pline, &myHand, &pile);
		}
	}
	return 0;
}
