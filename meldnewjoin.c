#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h> 

#include "macros.h"
#include "engine.h"
#include "drawdiscard.h"
#include "meldnewjoin.h"

/* Removes Card of rank 2 from given Group, adds whatever Card is missing for a meld */
/* NOTE: Only works if Group only has one Card of rank 2 and is, indeed, a meld */
void replaceTwo(Group *g)
{
	int i, hastwo = false;
	Group twos = NULL_GROUP; 
	Card c = NULL_CARD;

	for (i = 0; i < g->size; i++) {
		if (g->cards[i].rank == Two)
			hastwo = true;
	}
	if (!hastwo)
		return;
	separateTwos(&twos, g);
	sortGroup(g);
	for (i = 0; i < g->size-1; i++) {
		if (g->cards[i].rank == g->cards[i+1].rank+2) {
			c.suit = g->cards[i].suit;
			c.rank = g->cards[i+1].rank+1;
			addCardToGroup(g, c);
			break;
		}
	}
}

/* Scans a Group, returns json of cards that form some meld, or NULL if no meld found */
/* NOTE: Caller must free pointer if not NULL */
char* findMeldNew(Group *g)
{
	int i;
	char *p;
	Group meld = NULL_GROUP;

	for (i = 0; i < g->size; i++) {
		p = findMeldWithCard(g->cards[i], g);
		if (p != NULL) {
			addCardToGroup(&meld, g->cards[i]);
			addJsonToGroup(&meld, p);
			free(p);
			p = groupToJson(&meld);
			return p;
		}
	}
	return NULL;
}

/* Scans a Group and a Set, returns a string of the index of a Set's Group followed by
 * a json of one Card from given Group that fits inside the Set's Group, or NULL if
 * given Set is empty or if no Cards fit in any melds */
/* NOTE: Caller must free pointer if not NULL */
char* findMeldJoin(Set *st, Group *g)
{
	int i, k, bigs, bigk, hastwo;
	char *p, *str;
	Group g2 = NULL_GROUP, twos = NULL_GROUP;
	Group tmpg = NULL_GROUP;

	groupCopy(&g2, g);
	separateTwos(&twos, &g2);
	if (st->size <= 0) /* Set of melds is empty */
		return NULL;
	str = NULL;
	/* Find melds for non-twos */
	for (i = 0; i < g2.size; i++) {
		for (k = 0; k < st->size; k++) {
			if (st->grps[k].size >= 7)
				continue;
			if (groupHasCard(g2.cards[i], &tmpg))
				continue;
			addCardToGroup(&tmpg, g2.cards[i]);
			p = groupToJson(&(st->grps[k]));
			addJsonToGroup(&tmpg, p);
			free(p);
			replaceTwo(&tmpg);
			if (groupIsMeld(&tmpg)) {
				str = realloc(str, sizeof(char) * 15);
				sprintf(str, "%d ", k);
				p = cardToJson(g2.cards[i]);
				strcat(str, p);
				free(p);
			}
			clearGroup(&tmpg);
		}
	}
	if (str != NULL || twos.size == 0) /* Meld was found or no twos to test */
		return str;
	bigk = -1;
	bigs = -1;
	/* Find same suit melds for twos */
	for (k = 0; k < st->size; k++) {
		if (st->grps[k].size >= 7 && g2.size > 0)
			continue;
		hastwo = false;
		for (i = 0; i < st->grps[k].size; i++) {
			if (st->grps[k].cards[i].rank == Two)
				hastwo = true;
		}
		if (hastwo)
			continue;
		if (st->grps[k].size > bigs) {
			/* Saving meld without two */
			bigs = st->grps[k].size;
			bigk = k;
		}
		for (i = 0; i < twos.size; i++) {
			if (st->grps[k].cards[0].rank == twos.cards[i].rank &&
			    st->grps[k].cards[1].rank == twos.cards[i].rank) {
				/* Same suit meld found for some two */
				str = realloc(str, sizeof(char) * 15);
				sprintf(str, "%d ", k);
				p = cardToJson(twos.cards[i]);
				strcat(str, p);
				free(p);
				return str;
			}
		}
	}
	if (bigs == -1) /* No same suit melds nor melds without twos were found */
		return NULL;
	/* No melds found but melds without twos exist */
	str = realloc(str, sizeof(char) * 15);
	sprintf(str, "%d ", bigk);
	p = cardToJson(twos.cards[0]);
	strcat(str, p);
	free(p);
	return str;
}
