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
#include <stdio.h>
#include <string.h>

#include "weechat-plugin.h"
#include "twitch.h"
#include "twitch-plugin.h"

/* Pulls out the contents of a string after the first "="
 * Input:
 * 	char *tag: The full input tag
 * Return: free() after use
 * 	char *string_contents
 */
char *twitch_parse_tag(char *tag) {
	int count_tag = 0;
	char **array_string_tag = weechat_string_split(tag, "=", 0, 2, &count_tag);

	int length_contents;
	char *string_contents;
	if (count_tag > 1) {
		length_contents = strlen(array_string_tag[1]);
		string_contents = weechat_strndup(array_string_tag[1], length_contents);
	} else {
		length_contents = 0;
		string_contents = calloc(length_contents + 1, sizeof(char));
		snprintf(string_contents, length_contents + 1, "%s", "");
	}

	weechat_string_free_split(array_string_tag);
	return string_contents;
}

/* Grabs the name of the channel from the message
 * Input:
 * 	hashtable: Contains the message in a hashtable
 * Return: free() after use
 * 	char *: The name of the channel
 */
char *twitch_get_channel(struct t_hashtable *hashtable) {
	if (!weechat_hashtable_has_key(hashtable, "channel")) {
		return NULL;
	}

	int length_channel = strlen(weechat_hashtable_get(hashtable, "channel"));
	return weechat_strndup(weechat_hashtable_get(hashtable, "channel"), length_channel + 1);
}

/* Turns a message string into a hashtable format
 * Input:
 * 	char *string: The message string
 * Return: weechat_hashtable_free() after use
 * 	hashtable *message_parse: The parsable hashtable message
 */
struct t_hashtable *twitch_get_message(const char *string) {
	if (!string) {
		return NULL;
	}

	struct t_hashtable *hashtable_message_in;
	struct t_hashtable *hashtable_message_parse;

	hashtable_message_in = weechat_hashtable_new(8,
	                                             WEECHAT_HASHTABLE_STRING,
	                                             WEECHAT_HASHTABLE_STRING,
	                                             NULL,
	                                             NULL);

	if (!hashtable_message_in) {
		return NULL;
	}

	weechat_hashtable_set(
		hashtable_message_in,
		"message",
		string
	);

	if (!weechat_hashtable_has_key(hashtable_message_in, "message")) {
		return NULL;
	}

	hashtable_message_parse = weechat_info_get_hashtable(
		"irc_message_parse",
		hashtable_message_in
	);

	if (!hashtable_message_parse) {
		return NULL;
	}

	weechat_hashtable_free(hashtable_message_in);

	return hashtable_message_parse;
}

/* Checks if a localvar is set within a buffer
 * Input:
 * 	t_gui_buffer *buffer: The buffer to check
 * 	char *localvar: The name of the variable to check for
 * Output:
 * 	int ret:
 * 		0 => localvar is not set
 * 		1 => localvar is set
 */
int twitch_buffer_var_empty(struct t_gui_buffer *buffer, const char *localvar) {
	int ret = 0;
	if(!weechat_buffer_get_string(buffer, localvar)) {
		ret = 1;
	}
	return ret;
}

/* Updates localvar in buffer, adds if it doesn't exist
 * Input:
 * 	t_gui_buffer *buffer: The buffer to update
 * 	char *var: The variable to update
 * 	char *val: The value to update the variable to
 */ 	
void twitch_buffer_update_local(struct t_gui_buffer *buffer, const char *var, const char *val) {
	char *local = "localvar_";
	char *local_set = "localvar_set_";

	int length_local = strlen(local);
	int length_local_set = strlen(local_set);
	int length_var = strlen(var);

	char *string_local = calloc(length_local + length_var + 1, sizeof(char));
	snprintf(string_local, length_local + length_var + 1, "%s%s", local, var);

	char *string_local_set = calloc(length_local_set + length_var + 1, sizeof(char));
	snprintf(string_local_set, length_local_set + length_var + 1, "%s%s", local_set, var);

	if(twitch_buffer_var_empty(buffer, string_local)) {
		/* var is not defined */
		weechat_buffer_set(buffer, string_local_set, val);
	} else if(weechat_strcasecmp(weechat_buffer_get_string(buffer, string_local), val) != 0) {
		/* var and val are different, update var */
		weechat_buffer_set(buffer, string_local_set, val);
	}

	free(string_local);
	free(string_local_set);
	return;
}
