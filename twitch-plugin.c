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
	return twitch_hashtable_get_string(hashtable, "channel");
}

/* Grabs the hostname from the message
 * Input:
 * 	hashtable: Contains the message in a hashtable
 * Return: free() after use
 * 	char *: the hostname
 */
char *twitch_get_hostname(struct t_hashtable *hashtable) {
	return twitch_hashtable_get_string(hashtable, "host");
}

/* Returns the channel buffer
 * Input:
 * 	char *channel: The channel
 * 	char *server: The server name
 * Return:
 * 	t_gui_buffer *buffer: The buffer for the channel
 */
struct t_gui_buffer *twitch_get_channel_buffer(const char *channel, const char *server) {
	int length_channel = strlen(channel);
	int length_server = strlen(server);

	/* format: server.channel */
	int length_buffer = length_server + 1 + length_channel;
	char *search_buffer = calloc(length_buffer + 1, sizeof(char));
	snprintf(search_buffer, length_buffer + 1, "%s.%s", server, channel);
	struct t_gui_buffer *buffer = weechat_buffer_search("irc", search_buffer);

	free(search_buffer);
	return buffer;
}

/* Returns the server buffer
 * Input:
 * 	char *server: The Server
 * Return: free() after use
 * 	t_gui_buffer *buffer: The buffer for the server
 */
struct t_gui_buffer *twitch_get_server_buffer(const char *server) {
	int length_server = strlen(server);

	char *prefix = "server";
	int length_prefix = strlen(prefix);

	int length_buffer = length_prefix + 1 + length_server;
	char *search_buffer = calloc(length_buffer + 1, sizeof(char));
	snprintf(search_buffer, length_buffer + 1, "%s.%s", prefix, server);
	struct t_gui_buffer *buffer = weechat_buffer_search("irc", search_buffer);

	free(search_buffer);
	return buffer;
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

/* Allocates space and returns a string containing the data
 * in a hashtable.
 * Input:
 * 	t_hashtable *hashtable: The hashtable to get the value from
 * 	char *key: The key to grab the value of
 * Return: free() after use, NULL on error
 * 	char *val: The value in the key
 */
char *twitch_hashtable_get_string(struct t_hashtable *hashtable, const char *key) {
	if(!weechat_hashtable_has_key(hashtable, key)) {
		return NULL;
	}
	int *plength_val = malloc(sizeof(int));
	*plength_val = strlen(weechat_hashtable_get(hashtable, key));

	/* API documentation doesn't include a +1 for the \0 char,
	 * assuming not necessary
	 */
	char *pval = weechat_strndup(weechat_hashtable_get(hashtable, key), *plength_val);
	free(plength_val);
	return pval;
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

/* Builds a custom privmsg message when there isn't a given user
 * Input:
 * 	char *user: The user to masquarade as
 * 	char *host: The full host the message comes from
 * 	char *channel: The channel to receive the message on
 * 	char *message: The message to display
 * Return:
 * 	char *full_message: The entire IRC PRIVMSG
 */
char *twitch_build_privmsg_extended(const char *user, const char *host, const char *channel, const char *message) {
	int length_user = strlen(user);
	int length_host = strlen(host);

	char *exclamation = "!";
	int length_exclamation = strlen(exclamation);
	char *at = "@";
	int length_at = strlen(at);

	int length_new_host = length_user + length_exclamation;
	length_new_host += length_user;
	length_new_host += length_at;
	length_new_host += length_host;

	char *new_host = calloc(length_new_host + 1, sizeof(char));
	snprintf(new_host,
	         length_new_host + 1,
	         "%s%s%s%s%s",
	         user,
	         exclamation,
	         user,
	         at,
	         host);

	char *full_message = twitch_build_privmsg(new_host, channel, message);
	free(new_host);
	return full_message;
}

/* Builds a custom privmsg message
 * Input:
 * 	char *host: The full host the message comes from
 * 	char *channel: The channel to receive the message on
 * 	char *message: The message to display
 * Return:
 * 	char *full_message: The entire IRC PRIVMSG
 */
char *twitch_build_privmsg(const char *host, const char *channel, const char *message) {
	int length_host = strlen(host);
	int length_channel = strlen(channel);
	int length_message = strlen(message);

	char *colon = ":";
	int length_colon = strlen(colon);
	char *space = " ";
	int length_space = strlen(space);

	char *priv = " PRIVMSG ";
	int length_priv = strlen(priv);

	int length_full_message = length_colon + length_host;
	length_full_message += length_priv;
	length_full_message += length_channel;
	length_full_message += length_space;
	length_full_message += length_colon;
	length_full_message += length_message;

	char *full_message = calloc(length_full_message + 1, sizeof(char));
	snprintf(full_message,
	         length_full_message + 1,
	         "%s%s%s%s%s%s%s",
	         colon,
	         host,
	         priv,
	         channel,
	         space,
	         colon,
	         message);
	return full_message;
}


char *twitch_build_notice_channel(const char *host, const char *channel, const char *message) {
	int length_channel = strlen(channel);
	int length_message = strlen(message);
	int length_host = strlen(host);

	char *colon = ":";
	int length_colon = strlen(colon);
	char *space = " ";
	int length_space = strlen(space);

	char *notice = " NOTICE ";
	int length_notice = strlen(notice);

	int length_full_message = length_colon;
	length_full_message += length_host;
	length_full_message += length_notice;
	length_full_message += length_channel;
	length_full_message += length_space;
	length_full_message += length_colon;
	length_full_message += length_message;

	char *full_message = calloc(length_full_message + 1, sizeof(char));
	snprintf(full_message,
	         length_full_message + 1,
	         "%s%s%s%s%s%s%s",
	         colon,
		 host,
	         notice,
	         channel,
		 space,
	         colon,
	         message);
	return full_message;
}
