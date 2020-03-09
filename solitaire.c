/* Solitaire (Klondike) */

/* ==========================================================
 *
 * The basic logic of this implementation of solitaire is as follows:
 *
 * 1. Shuffle a deck of 52 cards.

 * 2. Dealing from the top of the deck, make seven piles (tableaus) of cards.
 * The pile number (1, 2, 3, 4, 5, 6, or 7) must contain a number of cards
 * equal to the pile number (1 card in pile 1, 2 cards in pile 2, etc.). The
 * top card of each tableau pile faces up.
 * 
 * 3. Reserve four empty piles (foundations).
 * 
 * 4. Reserve another pile (waste) into which the remaining cards are to be
 * dealt.
 * 
 * 5. The remaining cards are called the stock, the top card of which is face
 * down like the rest.
 * 
 * 6. Get input from the user (game begins here).
 * 
 * 7. Input takes the form of <label><label>. The piles are labeled in the
 * output as w (waste); f1, f2, f3, f4 (foundations); t1, t2, t3, t4, t5, t6,
 * t7 (tableaus).
 * 
 * 8. The following are valid input, where angle brackets contain necessary
 * input, square brackets contain optional input, the bar symbol separates
 * exclusive possible inputs, and the pound sign is a literal number:
 * 
 * a) [#]<w|f#|t#><t#> if either (the rank of the source is one less than the
 * rank of the destination) and (either (the suit of the source is a spade or
 * club if the destination is a heart or diamond) or (the suit of the source is
 * a heart or diamond if the destination is a spade or club)) or the source is
 * a king and the destination is empty. The initial, optional number indicates
 * how many cards you would like to move at once and that the bottom card of
 * that specified block is the source card against which the above conditions
 * are tested. No number specified defaults to one.
 * 
 * b) <w|f#|t#><f#> if the source is of the same suit as the destination and the
 * source's rank is one higher than that of the destination's rank. Only one
 * card at a time can be moved to a foundation pile.
 * 
 * c) <d> deals a single card from the stock to the waste.
 *
 * d) <u#> turns up a face down card on a tableau pile.
 *
 * 11. The game is won if each of the four foundations contains 13 cards (which
 * means that the stock, waste, and tableaus will be empty).
 * 
 * ===========================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <curses.h>
#include <ctype.h>
#include <stdbool.h>

struct card_type {
	char rank, suit;
	bool faceup;
};

struct card_type deck[52] = {{'2','H'},{'2','D'},{'2','C'},{'2','S'},
					 		 {'3','H'},{'3','D'},{'3','C'},{'3','S'},
					 		 {'4','H'},{'4','D'},{'4','C'},{'4','S'},
					 		 {'5','H'},{'5','D'},{'5','C'},{'5','S'},
					 		 {'6','H'},{'6','D'},{'6','C'},{'6','S'},
					 		 {'7','H'},{'7','D'},{'7','C'},{'7','S'},
					 		 {'8','H'},{'8','D'},{'8','C'},{'8','S'},
					 		 {'9','H'},{'9','D'},{'9','C'},{'9','S'},
					 		 {'0','H'},{'0','D'},{'0','C'},{'0','S'},
					 		 {'J','H'},{'J','D'},{'J','C'},{'J','S'},
					 		 {'Q','H'},{'Q','D'},{'Q','C'},{'Q','S'},
					 		 {'K','H'},{'K','D'},{'K','C'},{'K','S'},
					 		 {'A','H'},{'A','D'},{'A','C'},{'A','S'}};

struct pile_type {
	struct card_type pile[52];
	int pointer;
};

struct pile_type stock, waste, tab[7], fnd[4], temppile;
struct pile_type *srcp, *destp;

char rank_value[] = {'A','2','3','4','5','6','7','8','9','0','J','Q','K'};
char suit_value[] = {'D','C','H','S'};
char inputstr[80];
char destc;

struct pile_type total_cards;
int total_cards_count;

void shuffle();
void init_deal();
int get_input(char s[]);
int exec_input(char s[]);
int verify_input(char s[]);
int move_card(struct pile_type *srcp, struct pile_type *destp, char destc);
void init_curses();
void push(struct card_type card, struct pile_type *stock);
struct card_type pop(struct pile_type *stock);
void print_info();
int get_rank_value(struct card_type card);
int get_suit_value (struct card_type card);

int main()
{
	int n;
	int h, w;

	init_curses();
	getmaxyx(stdscr, h, w);
	shuffle();
	init_deal();
	print_info();
	while (get_input(inputstr)) {
		exec_input(inputstr);
		print_info();
		if (fnd[0].pointer == 13 && fnd[1].pointer == 13 && \
	fnd[2].pointer == 13 && fnd[3].pointer == 13)
			break;
	}
	endwin();
}

/* Move a card to a destination, either a tableau, foundation, or the waste. */
int move_card(struct pile_type *srcp, struct pile_type *destp, char destc)
{
	int i;
	struct card_type card_src, card_dest;

	/* Copy the appropriate cards to temporary structs for comparison. */
	card_src = srcp->pile[(srcp->pointer)-1];
	card_dest = destp->pile[(destp->pointer)-1];

	/* Do not permit moving a face-down card, unless it's from the stock. */
	if (srcp != &stock && !card_src.faceup)
		return 0;

	/* If moving to a tableau: */
	if (destc == 't') {
		/* Test:
		 * 1) if the source rank is one less than the destination rank;
		 * 2) and if the suits are opposite colors (see suit_value[]).
		 * 3) Or, if destination is empty, test if the source is a king.*/ 
		if ((((get_rank_value(card_src) == get_rank_value(card_dest) - 1) && \
			 ((get_suit_value(card_src) + get_suit_value(card_dest)) % 2 != 0))) || \
			  (get_rank_value(card_src) == 12 && destp->pointer == 0)) {
			push(pop(srcp), destp);
			return 1;
		}
	}
	/* If moving to a foundation: */
	else if (destc == 'f') {
		/* Move if the source is an ace and the foundation is empty. */
		if (get_rank_value(card_src) == 0 && destp->pointer == 0) {
			push(pop(srcp), destp);
			return 1;
		}
		/* Or move if the source is one rank higher than the destination. */
		if ((get_rank_value(card_src) == get_rank_value(card_dest) + 1) && \
	(get_suit_value(card_src) == get_suit_value(card_dest))) {
			push(pop(srcp), destp);
			return 1;
		}
	}
	/* If moving to the waste (only possible when dealing from the stock), move
	 * in any case and turn the new top waste card up and the previous one down. */
	else if (destc == 'w') {
		push(pop(srcp), destp);
		destp->pile[(destp->pointer)-2].faceup = false;
		destp->pile[(destp->pointer)-1].faceup = true;
		return 1;
	}
	return 0;
}

/* Make sure the input command is valid and return a number that corresponds
 * to the type of move being made (interpreted by exec_input). */
int verify_input(char s[])
{
	int i, n;
	char c;

	i = 0;
	/* Leading digits indicate how many cards to move at a time
	 * (none is fine too, this defaults to one card). */
	while (isdigit(c = s[i]))
		i++;
	/* If the first letter is a 'd' (deal), then point to the stock and
	 * waste as respective source and destination. Nothing should follow. */
	if ((c = s[i++]) == 'd' && s[i] == '\0') {
		srcp = &stock;
		destp = &waste;
		destc = 'w';
		return 1;
	}
	/* If the first letter is a 'w' (waste), then point to the waste as
	 * source. */
	if (c == 'w') {
		srcp = &waste;
		/* There can't be any number after 'w', since there is only one
		 * waste, so immediately check for either an 'f' (foundation) or
		 * 't' (tableau), since these are the only possible places to move
		 * a waste card to. */
		if ((destc = s[i++]) == 'f' || destc == 't') {
			/* Look for the digit that indicates which foundation or
	 		 * tableau the player wants to move to. */
			if (isdigit(s[i])) {
				n = (s[i++] - '0') - 1;
				/* Point to either the foundation or tableau and make
				 * sure the command has no further stuff in it. */
				destp = (destc == 'f' ? &fnd[n] : &tab[n]);
				return (s[i] == '\0') ? 2 : 0;
			}
		}
	}
	/* If the first letter is either an 'f' (foundation) or 't' (tableau): */
	if (c == 'f' || c == 't') {
		/* Check for the digit that indicates which foundation or tableau. */
		if (isdigit(s[i])) {
			n = (s[i++] - '0') - 1;
			/* Point to the appropriate source pile. */
			srcp = (c == 'f' ? &fnd[n] : &tab[n]);
			/* In this case, a card can be moved to either a foundation
			 * or tableau. */
			if ((destc = s[i++]) == 'f' || c == 't') {
				/* Get the digit indicating which one. */
				if (isdigit(s[i])) {
					n = (s[i++] - '0') - 1;
					/* Point to the appropriate destination pile and make
					 *  sure the command ends after this. */
					destp = (destc == 'f' ? &fnd[n] : &tab[n]);
					return (s[i] == '\0') ? 3 : 0;
				}
			}
		}
	}
	/* If the first letter is a 'u' (upturn): */
	if (c == 'u')
		/* Make sure there are no preceding digits. */
		if (i > 1)
			return 0;
	/* The next letter has to be a 't' for tableau since you can't turn
	 * over cards in other piles (except when a stock card turns over onto
	 * the waste pile, which happens automatically). */
	if ((c = s[i++]) == 't')
		/* Digit indicating which tableau. */
		if(isdigit(s[i++]))
			/* Make sure there are no further elements in the command. */
			return(s[i] == '\0') ? 4 : 0;
	return 0;
}

int exec_input(char s[])
{
	int n;
	int i;
	int mult;
	char multstr[2];

	/* Verifying the input string returns a number between 0 and 4,
	 * indicating various kinds of commands. */
	n = verify_input(s);

	/* Collect the initial digits of the string indicating how many
	 * cards to move. */
	i = 0;
	while (isdigit(s[i])) {
		multstr[i] = s[i];
		i++;
	}
	multstr[i] = '\0';

	/* 0 returned from verify_input means the input is invalid. */
	if (n == 0) {
		printw("Invalid input!");
		getch();
		return 0;
	}
	/* 1 returned means deal. */
	else if (n == 1) {
		/* If the stock is empty, then the waste becomes the new stock. */
		if (stock.pointer == 0) {
			waste.pile[waste.pointer - 1].faceup = false;
			for (i = waste.pointer - 1; i >= 0; i--) {
				push(waste.pile[i], &stock);
			}
			waste.pointer = 0;
		}
		/* Move card from the stock to the waste. */
		move_card(srcp, destp, destc);
	}
	/* 2 returned means move from waste to tableau or foundation. */
	else if (n == 2) {
		if (move_card(srcp, destp, destc))
			waste.pile[waste.pointer-1].faceup = true;
	}
	/* 3 returned means move from tableau or foundation. */
	else if (n == 3) {
		mult = atoi(multstr);
		if (mult == 0)
			mult++;
		/* Move mult cards from the source pile to temppile. */
		for (i = 0; i < mult; i++)
			push(pop(srcp), &temppile);
		/* Move the temppile to the destination pile, but undo the process
		 * if one of the cards is not permitted to move. */
		for (i = 0; i < mult; i++) {
			if (!move_card(&temppile, destp, destc)) {
				srcp->pointer += mult;
				destp->pointer -= i;
				break;
			}
		}
	}
	/* 4 returns means turn up a tableau card. */
	else if (n == 4) {
		/* Get the tableau digit from the string (which should look like
		 * 'ut3'), convert it to a number, reduce by one for index. */
		i = (s[2] - '0') - 1;
		tab[i].pile[tab[i].pointer - 1].faceup = true;
	}
}

/* Use curses library to get input string. Return false if 'q' is
 * entered. */
int get_input(char s[])
{
	int h, w;

	getmaxyx(stdscr, h, w);
	mvprintw(h-1, 0, ":");
	getstr(s);
	return (s[0] != 'q');
}

void push(struct card_type card, struct pile_type *stock)
{
	int i;

	stock->pile[stock->pointer++] = card;
}

struct card_type pop(struct pile_type *stock)
{
	return stock->pile[(stock->pointer--)-1];
}

int get_suit_value (struct card_type card)
{
	int i;

	for (i = 0; i < 4; i++)
		if (card.suit == suit_value[i])
			return i;
}

int get_rank_value (struct card_type card)
{
	int i;

	for (i = 0; i <= 13; i++)
		if (card.rank == rank_value[i])
			return i;
}

/* Randomize the order of the deck. */
void shuffle()
{
	int i, r;
	struct card_type t;

	srand(time(0));
	/* Go through the deck, switching each successive card with a
	 * random one. */
	for (i = 0; i < 52; i++) {
		r = rand() % 52;
		t = deck[r];
		deck[r] = deck[i];
		deck[i] = t;
	}
}

/* Initialize the piles of cards necessary to gameplay: the seven
 * tableaus (those piles on which one builds up successive ranks of
 * alternating suits), the stock (the general resource from which
 * cards are drawn), the waste (the pile onto which one card at a time
 * is drawn from the stock, the topmost being playable), the
 * foundations (the four piles which it is one's goal to fill with
 * like-suited successively ranked cards), and finally a temporary
 * pile which is necessary for the underlying logic but does not
 * appear in gameplay. */
void init_deal()
{
	int i, j;
	int deckp;

	/* Make 7 tableaus, each of which has as many cards as its index
	 * number, so that the first one gets one card, the second two,
	 * etc. Take these cards from the deck. Only the topmost card's
	 * face-up value should be true. */
	deckp = 51;
	for (i = 0; i < 7; i++) {
		for (j = 0; j <= i; j++) {
			tab[i].pile[j] = deck[deckp--];
			if (i == j)
				tab[i].pile[j].faceup = true;
			else
				tab[i].pile[j].faceup = false;
		}
		tab[i].pointer = i + 1;
	}

	/* Fill the stock with the remaining cards from the deck. None
	 * should be face-up. */
	i = 0;
	while (deckp >= 0) {
		stock.pile[i++] = deck[deckp--];
		stock.pile[i-1].faceup = false;
	}

	/* Set up four empty foundations, one empty waste, and one empty
	 * temppile. */
	stock.pointer = i;
	for (i = 0; i < 4; i++)
		fnd[i].pointer = 0;
	waste.pointer = 0;
	temppile.pointer = 0;
}

/* Print out an informational representation of the current state of a
 * pile, consisting of a row of square-bracket-enclosed two-character
 * card representations, the first character being the rank and the
 * second the suit. */
void print_pile(struct pile_type *stock)
{
	int i;
	/* Iterate as far as the pointer points in the stock. Print full
	 * information for face-up cards, and empty brackets for non-face-up
	 * cards. */
	for (i = 0; i < stock->pointer; i++) {
		total_cards.pile[total_cards_count++] = stock->pile[i];
		if (stock->pile[i].faceup == false)
			printw("[  ]");
		else {
			if (stock->pile[i].suit == 'H' || stock->pile[i].suit == 'D') {
				attron(COLOR_PAIR(1));
				printw("[%c%c]", stock->pile[i].rank, stock->pile[i].suit);
				attroff(COLOR_PAIR(1));
			} else {
				printw("[%c%c]", stock->pile[i].rank, stock->pile[i].suit);
			}
		}
	}
}

/* Print out a label for each pile and call the function to print the
 * pile itself. */
void print_info()
{
	int i, j;

	total_cards_count = 0;

	clear();
	print_pile(&stock);
	printw("\nSTOCK\n\n");
	print_pile(&waste);
	printw("\nWASTE\n\n");
	for (i = 0; i < 7; i++) {
		print_pile(&tab[i]);
		printw("\nTABLEAU %d\n\n", i+1);
	}
	for (i = 0; i < 4; i++) {
		print_pile(&fnd[i]);
		printw("\nFOUNDATION %d\n\n", i+1);
	}

	refresh();
}

/* Initialize curses. */
void init_curses()
{
	initscr();
	start_color();
	init_pair(1, COLOR_RED, COLOR_BLACK);
	cbreak();
	nonl();
	intrflush(stdscr, FALSE);
	keypad(stdscr, TRUE);
	curs_set(1);
}
