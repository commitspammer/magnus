#ifndef ENGINE_H_
#define ENGINE_H_

enum Rank {
	One, Two, Three, Four, Five,
	Six, Seven, Eight, Nine, Ten,
	Jack, Queen, King, Ace
};

enum Suit {
	Hearts, Clubs, Spades, Diamonds
};

typedef struct card {
	int rank;
	int suit;
} Card;

typedef struct group {
	Card cards[MAX_GROUP];
	int size;
} Group;

typedef struct set {
	Group grps[MAX_GROUP];
	int size;
} Set;

struct pair {
	char s[5];
	int v;
};

void readline(char *line);
int strToRank(char *str);
char* rankToStr(Card c);
int strToSuit(char *str);
char* suitToStr(Card c);
char* cardToStr(Card c);
void printGroup(Group *g);
char* cardToJson(Card c);
char* groupToJson(Group *g);
int cardCmp(Card c1, Card c2);
void clearGroup(Group *g);
void assignCard(Card *c, char *json);
void addJsonToGroup(Group *g, char *json);
void addCardToGroup(Group *g, Card c);
void rmJsonFromGroup(Group *g, char *json);
void rmCardFromGroup(Group *g, Card c);
void groupCopy(Group *dest, Group *src);
void swapCards(Card *c1, Card *c2);
void sortGroup(Group *g);
void rmGroupDups(Group *g);
void addOnePerAce(Group *g);
void separateTwos(Group *dest, Group *src);
int groupIsMeld(Group *g);
int groupHasCard(Card c, Group *g);

#endif
