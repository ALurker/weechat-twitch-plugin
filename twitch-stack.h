/**
  * This program is free software; you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation; either version 3 of the License, or
  * (at your option) any later version.
  * 
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  * 
  * You should have received a copy of the GNU General Public License
  * along with this program.  If not, see <http://www.gnu.org/licenses/>.
  * 
  * This code is heavily based of of the work by mumixam located at
  * https://github.com/mumixam/weechat-twitch
  *
  **/

#ifndef TWITCH_STACK_H
#define TWITCH_STACK_H 1

#define twitch_stack_push(X,Y) _Generic((X),              \
	char *: twitch_stack_push_char,                   \
	char **: twitch_stack_push_split,                 \
	int *: twitch_stack_push_int,                     \
	struct t_hashtable *: twitch_stack_push_hashtable \
	)(X,Y)


typedef struct _twitch_stack twitch_stack;
typedef struct _twitch_stack_element twitch_stack_element;

struct _twitch_stack {
	struct _twitch_stack_element *pHead;
};

struct _twitch_stack_element {
	union {
		char *pString;
		struct t_hashtable *pHashtable;
		int *pInteger;
		char **pStringSplit;
	} data;
	enum {
		STRING,
		HASHTABLE,
		INTEGER,
		STRING_SPLIT
	} type;
	twitch_stack_element *pNext;
};

twitch_stack *twitch_stack_create();
twitch_stack_element *twitch_stack_push_base(twitch_stack *stack);
int twitch_stack_push_char(char *string, twitch_stack *stack);
int twitch_stack_push_split(char **string_split, twitch_stack *stack);
int twitch_stack_push_int(int *i, twitch_stack *stack);
int twitch_stack_push_hashtable(struct t_hashtable *hashtable, twitch_stack *stack);

twitch_stack_element *twitch_stack_pop(twitch_stack *stack);
void twitch_stack_free_element(twitch_stack_element *e);
int twitch_stack_free(twitch_stack *stack);

#endif /* TWITCH_STACK_H */
