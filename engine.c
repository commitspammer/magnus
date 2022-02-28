#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "macros.h" 
#include "engine.h" 

static const struct pair RankMatch[RANK_COUNT] = {
        {"1",    One},
        {"2",    Two},   {"3", Three},
        {"4",   Four},   {"5",  Five},
        {"6",    Six},   {"7", Seven},
        {"8",  Eight},   {"9",  Nine},
        {"10",   Ten},   {"J",  Jack},
        {"Q",  Queen},   {"K",  King},
        {"A",    Ace}
};

static const struct pair SuitMatch[SUIT_COUNT] = {
	{"♥",   Hearts},
	{"♣",    Clubs},
	{"♠",   Spades},
	{"♦", Diamonds}
};

/* Reads a line from stdin, stores it inside given char array */
void readline(char *line)
{
	fgets(line, MAX_LINE, stdin);
	int l = strlen(line) - 1;
	if(line[l] == '\n') {
  		line[l] = '\0';
	}
}

/* Scans a rank in string format, returns matching enum, or -1 if error */
int strToRank(char *str)
{
	for (int i = 0; i < RANK_COUNT; i++) {
		if (!strcmp(RankMatch[i].s, str))
			return RankMatch[i].v;
	}
	EPUT("ERROR: strToRank()");
	return -1;
}

/* Scans a rank in enum format, returns matching string, or NULL if error */
/* NOTE: Caller must free pointer if not NULL */
char* rankToStr(Card c)
{
	char *str = malloc(sizeof(char) * 3);

	for (int i = 0; i < RANK_COUNT; i++) {
		if (RankMatch[i].v == c.rank) {
			strcpy(str, RankMatch[i].s);
			return str;
		}
	}
	EPUT("ERROR: rankToStr()");
	return NULL;
}

/* Scans a suit in string format, returns matching enum, or -1 if error */
int strToSuit(char *str)
{
	for (int i = 0; i < SUIT_COUNT; i++) {
		if (!strcmp(SuitMatch[i].s, str))
			return SuitMatch[i].v;
	}
	EPUT("ERROR: strToSuit()");
	return -1;
}

/* Scans a suit in enum format, returns matching string, or NULL if error */
/* NOTE: Caller must free pointer if not NULL */
char* suitToStr(Card c)
{
	char *str = malloc(sizeof(char) * 5);

	for (int i = 0; i < SUIT_COUNT; i++) {
		if (SuitMatch[i].v == c.suit) {
			strcpy(str, SuitMatch[i].s);
			return str;
		}
	}
	EPUT("ERROR: suitToStr()");
	return NULL;
}

/* Scans a Card, returns correspondent string */
/* NOTE: Caller must free pointer */
char* cardToStr(Card c)
{
	char *r, *s, *rs;	

	r = rankToStr(c);
	s = suitToStr(c);
	rs = malloc(sizeof(char) * 7);
	sprintf(rs, "%s%s", r, s);
	free(r);
	free(s);
	return rs;
}

/* Prints a Group in stderr for debugging */
void printGroup(Group *g)
{
	int i;
	char *rs;

	for (i = 0; i < g->size; i++) {
		rs = cardToStr(g->cards[i]);
		fprintf(stderr, "[%d] %s\n", i+1, rs);
		free(rs);
	}
}

/* Scans a Card, returns correspondent json */
/* NOTE: Caller must free pointer */
char* cardToJson(Card c)
{
	char *j, *rs;

	j = malloc(sizeof(char) * 11);
	strcpy(j, "[ ");
	rs = cardToStr(c);
	strcat(j, rs);
	strcat(j, " ]");
	free(rs);
	return j;
}

/* Scans a Group, returns correspondent json */
/* NOTE: Caller must free pointer */
char* groupToJson(Group *g)
{
	int i;
	char *j, *rs;

	j = malloc(sizeof(char) * 3);
	strcpy(j, "[ ");
	for (i = 0; i < g->size; i++) {
		j = realloc(j, sizeof(char) * 8*(i+1));
		rs = cardToStr(g->cards[i]);
		strcat(j, rs);
		strcat(j, " ");
		free(rs);
	}
	j = realloc(j, sizeof(char) * (8*i)+2);
	strcat(j, "]");
	return j;
}

/* Compares two Cards, returns whether they're equal or not */
int cardCmp(Card c1, Card c2)
{
	if (c1.rank == c2.rank) {
		if (c1.suit == c2.suit)
			return true;
	}
	return false;
}

/* Empties a Group */
void clearGroup(Group *g)
{
	g->size = 0;
	for (int i = 0; i < MAX_GROUP; i++) {
		g->cards[i].rank = 0;
		g->cards[i].suit = 0;
	}
}

/* Scans a card in json format, assigns values to given Card */
/* NOTE: Potentially unstable, for unknown reasons */
void assignCard(Card *c, char *json)
{
	int k;
	char *tok, j[MAX_LINE];

	strcpy(j, json);
	tok = strtok(j, "[ ]\n");
	if (tok == NULL)
		return;
	k = (j[1] == '0') ? 2 : 1;
	c->suit = strToSuit(j+k);
	j[k] = '\0';
	c->rank = strToRank(j);
}

/* Scans a json of cards, adds them to given Group */
void addJsonToGroup(Group *g, char *json)
{
	int i, k;
	char *tok, j[MAX_LINE];

	strcpy(j, json);
	tok = strtok(j, "[ ]\n");
	while (tok != NULL) {
		k = (tok[1] == '0') ? 2 : 1;
		i = g->size;
		g->cards[i].suit = strToSuit(tok+k);
		tok[k] = '\0';
		g->cards[i].rank = strToRank(tok);
		g->size++;
		tok = strtok(NULL, "[ ]\n");
	}
}

/* Scans a Card, adds it to given Group */
void addCardToGroup(Group *g, Card c)
{
	char *rs;

	rs = cardToJson(c);
	addJsonToGroup(g, rs);
	free(rs);
}

/* Scans a json of cards, removes them from given Group */
void rmJsonFromGroup(Group *g, char *json)
{
	int i, k;
	char j[MAX_LINE];
	Group bad = NULL_GROUP, fine = NULL_GROUP;

	strcpy(j, json);
	addJsonToGroup(&bad, j);
	for (i = 0; i < g->size; i++) {
		for (k = 0; k < bad.size; k++) {
			if (cardCmp(bad.cards[k], g->cards[i]))
				break;
		}
		if (k < bad.size) {
			assignCard(&(bad.cards[k]), "1♥"); /* Dont touch. It works */
			continue;
		}
		addCardToGroup(&fine, g->cards[i]);
	}
	g->size = fine.size;
	for (i = 0; i < MAX_GROUP; i++)
		g->cards[i] = fine.cards[i];
}

/* Scans a Card, removes it from given Group */
void rmCardFromGroup(Group *g, Card c)
{
	char *rs;

	rs = cardToJson(c);
	rmJsonFromGroup(g, rs);
	free(rs);
}

/* Copies second Group into first Group */
void groupCopy(Group *dest, Group *src)
{
	char *rs;
	
	rs = groupToJson(src);
	clearGroup(dest);
	addJsonToGroup(dest, rs);
	free(rs);
}

/* Swaps memory address of two Cards */
void swapCards(Card *c1, Card *c2)
{
	Card tmp;

	tmp = *c1;
	*c1 = *c2;
	*c2 = tmp;
}

/* Sorts a given Group in blocks of suits, each block in descending rank value */
void sortGroup(Group *g)
{
	int i, j;

	for (i = 0; i < g->size; i++) {
		for (j = 0; j < g->size-1-i; j++) {
			if (g->cards[j].rank < g->cards[j+1].rank)
				swapCards(&(g->cards[j]), &(g->cards[j+1]));
			if (g->cards[j].suit < g->cards[j+1].suit)
				swapCards(&(g->cards[j]), &(g->cards[j+1]));
		}
	}
}

/* Removes duplicated Cards from given Group */
void rmGroupDups(Group *g)
{
	int i = 0;

	sortGroup(g);
	while (i < g->size-1) {
		if (cardCmp(g->cards[i], g->cards[i+1])) {
			rmCardFromGroup(g, g->cards[i+1]);
			continue;
		}
		i++;
	}
}

/* Adds Cards of rank 1 inside Group for each Card of rank Ace */
void addOnePerAce(Group *g)
{
	int i;
	char *rs;

	for (i = 0; i < g->size; i++) { 
		rs = cardToJson(g->cards[i]);
		if (g->cards[i].rank == Ace) {
			rs[2] = '1';
			addJsonToGroup(g, rs);
		}
		free(rs);
	}
}

/* Removes Cards of rank 2 from second Group, adds them to first Group */
void separateTwos(Group *dest, Group *src)
{
	int i = 0;
	char *rs;

	while (i < src->size) {
		rs = cardToJson(src->cards[i]);
		if (src->cards[i].rank == Two) {
			rmJsonFromGroup(src, rs);
			addJsonToGroup(dest, rs);
		} else {
			i++;
		}
		free(rs);
	}
}

/* Scans a Group, returns whether it's a meld or not */
/* NOTE: Doesn't consider Cards of rank 2 as jokers (a fatal regret) */
int groupIsMeld(Group *g)
{
	int i, badace = 0;
	Group g2 = NULL_GROUP;

	if (g->size < 3)
		return false;
	groupCopy(&g2, g);
	addOnePerAce(&g2);
	sortGroup(&g2);
	for (i = 0; i < g2.size-1; i++) {
		if (g2.cards[i].suit == g2.cards[i+1].suit &&
		    g2.cards[i].rank == g2.cards[i+1].rank+1)
			continue;
		if (g2.cards[i].rank == Ace || g2.cards[i+1].rank == Ace)
			badace++;
		else if (g2.cards[i].rank == One || g2.cards[i+1].rank == One)
			badace++;
		else
			return false;
		if (badace > 1)
			return false;
	}
	return true;
}

/* Scans a Group, returns whether given Card exists or not */
int groupHasCard(Card c, Group *g)
{
	int i;

	for (i = 0; i < g->size; i++) {
		if (cardCmp(g->cards[i], c))
			return true;
	}
	return false;
}
