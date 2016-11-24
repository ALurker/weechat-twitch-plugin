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

#include <stdlib.h>

#include "weechat-plugin.h"
#include "twitch.h"
#include "twitch-stack.h"

twitch_stack_element *twitch_stack_push_base(twitch_stack *stack) {
	twitch_stack_element *e = calloc(1, sizeof(twitch_stack_element));
	e->pNext = stack->pHead;
	stack->pHead = e;
	return e;
}

int twitch_stack_push_char(char *string, twitch_stack *stack) {
	twitch_stack_element *e = twitch_stack_push_base(stack);
	e->type = STRING;
	e->data.pString = string;
	return 1;
}

int twitch_stack_push_split(char **string_split, twitch_stack *stack) {
	twitch_stack_element *e = twitch_stack_push_base(stack);
	e->type = STRING_SPLIT;
	e->data.pStringSplit = string_split;
	return 1;
}

int twitch_stack_push_int(int *i, twitch_stack *stack) {
	twitch_stack_element *e = twitch_stack_push_base(stack);
	e->type = INTEGER;
	e->data.pInteger = i;
	return 1;
}

int twitch_stack_push_hashtable(struct t_hashtable *hashtable, twitch_stack *stack) {
	twitch_stack_element *e = twitch_stack_push_base(stack);
	e->type = HASHTABLE;
	e->data.pHashtable = hashtable;
	return 1;
}

void twitch_stack_pop(twitch_stack *stack) {
	twitch_stack_element *e = stack->pHead;
	stack->pHead = e->pNext;
	switch(e->type) {
		case STRING:
			free(e->data.pString);
			break;
		case HASHTABLE:
			weechat_hashtable_free(e->data.pHashtable);
			break;
		case INTEGER:
			free(e->data.pInteger);
			break;
		case STRING_SPLIT:
			weechat_string_free_split(e->data.pStringSplit);
			break;
	}
	free(e);
	return;
}

int twitch_stack_free(twitch_stack *stack) {
	while (stack->pHead != NULL) {
		twitch_stack_pop(stack);
	}
	return 1;
}
