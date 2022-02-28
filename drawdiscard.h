#ifndef DRAWDISCARD_H_
#define DRAWDISCARD_H_

void findPotentialMelds(Set *st, Group *g);
char* findMeldWithCard(Card c, Group *g);
char* findBestDiscard(Group *g, Group *g2);

#endif
