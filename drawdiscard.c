#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h> 

#include "macros.h"
#include "engine.h"
#include "drawdiscard.h"

/* Scans a Group, stores copies of all Cards of rank 2 inside given Set's first Group
 * and stores melds as well as potential melds inside Set's second Group and forward */
/* NOTE: Doesn't use recurssion, so it only finds a few potential melds */
void findPotentialMelds(Set *st, Group *g)
{
	int i, k, sequence = false;
	Group g2 = NULL_GROUP;

	groupCopy(&g2, g);
	addOnePerAce(&g2);
	rmGroupDups(&g2);
	separateTwos(&(st->grps[0]), &g2);
	sortGroup(&g2);
	st->size = 1;
	for (i = 0; i < g2.size-1; i++) {
		k = st->size;
		if (g2.cards[i].suit == g2.cards[i+1].suit &&
		    g2.cards[i].rank == g2.cards[i+1].rank+1) {
			sequence = true;
			addCardToGroup(&(st->grps[k]), g2.cards[i]);
			addCardToGroup(&(st->grps[k]), g2.cards[i+1]);
		} else if (sequence) {
			rmGroupDups(&(st->grps[k]));
			st->size++;
			sequence = false;
		}
	}
	if (sequence) {
		rmGroupDups(&(st->grps[k]));
		st->size++;
	}
	for (i = 0; i < g2.size-1; i++) {
		k = st->size;
		if (g2.cards[i].suit == g2.cards[i+1].suit &&
	            g2.cards[i].rank == g2.cards[i+1].rank+2) {
			if (g2.cards[i+1].rank == One)
				g2.cards[i+1].rank = Ace;
			addCardToGroup(&(st->grps[k]), g2.cards[i]);
			addCardToGroup(&(st->grps[k]), g2.cards[i+1]);
			st->size++;
		}
	}
}

/* Scans a Group, checks if given Card makes any melds, returns json of cards
 * necessary for the meld (except given Card itself), or NULL if no meld found */
/* NOTE: Caller must free pointer if not NULL */
char* findMeldWithCard(Card c, Group *g)
{
	int i;
	char *str;
	Set melds = NULL_SET;
	Group tmpg = NULL_GROUP, twos = NULL_GROUP;
	Card tmpc = NULL_CARD;

	findPotentialMelds(&melds, g);
	if (melds.grps[1].size == 0) /* Set has no non-two cards */
		return NULL;
	if (c.rank == Two) { /* Card is a two */
		str = groupToJson(&(melds.grps[1]));
		return str;
	}
	/* Find clean meld for card*/
	for (i = 1; i < melds.size; i++) {
		rmCardFromGroup(&(melds.grps[i]), c); /* Removes possible copy of card */
		addCardToGroup(&(melds.grps[i]), c);
		if (groupIsMeld(&(melds.grps[i]))) {
			/* Clean meld for card found */
			rmCardFromGroup(&(melds.grps[i]), c);
			str = groupToJson(&(melds.grps[i]));
			return str;
		}
		rmCardFromGroup(&(melds.grps[i]), c);
	}
	if (melds.grps[0].size == 0) /* Clean meld not found and no twos in hand */
		return NULL;
	tmpc.suit = c.suit;
	/* Find dirty meld for card */
	for (i = 0; i < 2; i++) {
		if (i == 0)
			tmpc.rank = (c.rank == Ace) ? Ace : c.rank+1;
		else
			tmpc.rank = (c.rank == Three) ? Three : c.rank-1;
		groupCopy(&tmpg, g);
		separateTwos(&twos, &tmpg);
		str = findMeldWithCard(tmpc, &tmpg);
		if (str == NULL)
			continue;
		clearGroup(&tmpg);
		addJsonToGroup(&tmpg, str);
		free(str);
		if (groupHasCard(c, &tmpg))
			continue;
		/* Dirty meld for card found */
		addCardToGroup(&tmpg, twos.cards[0]);
		str = groupToJson(&tmpg);
		return str;
	}
	/* No dirty melds for card found */
	return NULL;
}

/* Scans my hand and opponent's hand, returns json of the Card that is best to be
 * discarded, or NULL if my hand is empty */
/* NOTE: Caller must free pointer is not NULL */
char* findBestDiscard(Group *g, Group *g2)
{
	int i, j = 0, k = 0, n = -1, m = 0;
	char *str;
	Group hand = NULL_GROUP;
	Group safe = NULL_GROUP, twos = NULL_GROUP;

	if (g->size == 0) /* Hand is empty */
		return NULL;
	groupCopy(&hand, g);
	sortGroup(&hand);
	separateTwos(&twos, &hand);
	if (hand.size == 0) { /* Hand only has twos */
		str = cardToJson(twos.cards[0]);
		return str;
	} else if (hand.size == 1) { /* Hand only has one non-two card */
		str = cardToJson(hand.cards[0]);
		return str;
	}
	/* Find safe cards opponent can't pickup */
	for (i = 0; i < hand.size; i++) {
		str = findMeldWithCard(hand.cards[i], g2);
		if (str == NULL) {
			addCardToGroup(&safe, hand.cards[i]);
		} else {
			free(str);
			str = cardToStr(hand.cards[i]);
		}
		free(str);
	}
	/* Find safe duplicated, safe isolated and farthest away cards */
	str = NULL;
	for (i = 0; i < 4; i++) {
		k = 0;
		while (hand.cards[j].suit == hand.cards[j+1].suit && j < hand.size-1) {
			if (hand.cards[j].rank == hand.cards[j+1].rank &&
			    groupHasCard(hand.cards[i], &safe))  {
				/* Safe duplicate found */
				str = cardToJson(hand.cards[j]);
				return str;
			}
			if (hand.cards[j].rank - hand.cards[j+1].rank > m) {
				/* Current farthest away card found */
				m = hand.cards[j].rank - hand.cards[j+1].rank;
				n = j;
				if (str != NULL)
					free(str);
				str = cardToStr(hand.cards[j]);
			}
			k++;
			j++;
		}
		j++;
		if (k == 0 && groupHasCard(hand.cards[j-1], &safe)) {
			/* Safe isolated suit found */
			str = cardToJson(hand.cards[j-1]);
			return str;
		}
		if (j >= hand.size-1) /* No safe duplicated or isolated cards found */
			break;
	}
	/* Asure farthest away card */
	if (hand.cards[n+1].suit != hand.cards[n+2].suit || n+2 < hand.size)
		str = cardToJson(hand.cards[n+1]);
	else if (hand.cards[n].suit != hand.cards[n-1].suit || n-1 >= 0)
		str = cardToJson(hand.cards[n]);
	if (str != NULL)
		return str;
	if (hand.cards[n+1].rank - hand.cards[n+2].rank <
	    hand.cards[n-1].rank - hand.cards[n].rank)
		str = cardToJson(hand.cards[n]);
	else if (hand.size < n+1)
		str = cardToJson(hand.cards[n]);
	else
		str = cardToJson(hand.cards[n+1]);
	return str;
}
