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

WEECHAT_PLUGIN_NAME("twitch");
WEECHAT_PLUGIN_DESCRIPTION("Twitch plugin for WeeChat");
WEECHAT_PLUGIN_AUTHOR("ALurker <ALurker@outlook.com>");
WEECHAT_PLUGIN_VERSION("0.1");
WEECHAT_PLUGIN_LICENSE("GPL3");

struct t_weechat_plugin *weechat_plugin = NULL;
struct t_hook *hook_usernotice = NULL;
struct t_hook *hook_clearchat = NULL;

struct t_hashtable *hashtable_get_message_parse(const char *string) {
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

	return hashtable_message_parse;
}

char* cb_modifier_clearchat(const void *pointer,
                            void *data,
                            const char *modifier,
                            const char *modifier_data,
                            const char *string) {

	struct t_hashtable *hashtable_message_parse = hashtable_get_message_parse(string);

	if (!hashtable_message_parse) {
		return NULL;
	}

	/* Server Name should be in modifier_data */
	int length_server = strlen(modifier_data);
	/* Increment by 1 to include \0 */
	char *string_server = weechat_strndup(modifier_data, length_server + 1);

	if (!weechat_hashtable_has_key(hashtable_message_parse, "channel")) {
		return NULL;
	}

	weechat_printf(NULL, "Server: %s", string_server);

	int length_channel = strlen(weechat_hashtable_get(hashtable_message_parse, "channel"));
	char *string_channel = weechat_strndup(weechat_hashtable_get(hashtable_message_parse, "channel"), length_channel + 1);

	weechat_printf(NULL, "Channel: %s", string_channel);

	/* These messages have the user clearing the chat stored in text */
	if (!weechat_hashtable_has_key(hashtable_message_parse, "text")) {
		return NULL;
	}

	int length_user = strlen(weechat_hashtable_get(hashtable_message_parse, "text"));
	char *string_user = weechat_strndup(weechat_hashtable_get(hashtable_message_parse, "text"), length_user + 1);

	weechat_printf(NULL, "User: %s", string_user);

	//FIXME
	//weechat_printf(NULL, "hashtable_message_parse allocated successfully"

	int count_tags;
	char **tags;
	if (weechat_hashtable_has_key(hashtable_message_parse, "tags")) {
		/* Tags should be seperated by ; */
		tags = weechat_string_split(weechat_hashtable_get(hashtable_message_parse, "tags"),
		                            ";",
		                            0,
		                            0,
		                            &count_tags
		);
	}

	/* Stuff to Return
	 * For some reason returning an empty string gets rid of the command not found message
	 */

	int length_return = 2;
	char *result = malloc(length_return);
	snprintf(result, length_return, "%s", "");
	return result;
}

char* cb_modifier_usernotice(const void *pointer,
                             void *data,
                             const char *modifier,
                             const char *modifier_data,
                             const char *string) {

	struct t_hashtable *hashtable_message_parse = hashtable_get_message_parse(string);

	if (!hashtable_message_parse) {
		return NULL;
	}

	/* Server Name should be in modifier_data */
	int length_server = strlen(modifier_data);
	/* Increment by 1 to include \0 */
	char *string_server_name = weechat_strndup(modifier_data, length_server + 1);

	int length_channel = strlen(weechat_hashtable_get(hashtable_message_parse, "channel"));
	char *string_channel_name = weechat_strndup(
		weechat_hashtable_get(hashtable_message_parse, "channel"),
		length_channel + 1
	);

	/* Buffer Name: znc-twitch.#day9tv\0
	 * Adding of 2 to allow for period and endline
	 */

	int length_buffer = 2 + length_server + length_channel;
	char *string_buffer = calloc(length_buffer, sizeof(char));
	snprintf(string_buffer, length_buffer, "%s.%s", string_server_name, string_channel_name);

	char *string_buffer_plugin = "irc";
	struct t_gui_buffer *buffer_channel = weechat_buffer_search(string_buffer_plugin, string_buffer);

	if (!buffer_channel) {
		return NULL;
	}

	char *string_server_prefix = "server.";
	int length_server_prefix = strlen(string_server_prefix);

	int length_server_full = length_server_prefix + length_server + 1;
	char *string_server_full = calloc(length_server_full, sizeof(char));

	/* Primary Server Buffer: irc.server.ZNC-Twitch */
	snprintf(string_server_full, length_server_full, "%s%s", string_server_prefix, string_server_name);
	struct t_gui_buffer *buffer_server = weechat_buffer_search(string_buffer_plugin, string_server_full);

	if (!buffer_channel) {
		return NULL;
	}

	if (weechat_hashtable_has_key(hashtable_message_parse, "tags")) {
		/* Tags should be seperated by ; */
		int count_tags;
		char **tags;
		tags = weechat_string_split(weechat_hashtable_get(hashtable_message_parse, "tags"),
		                            ";",
		                            0,
		                            0,
		                            &count_tags
		);

		if (!tags) {
			return NULL;
		}

		int count_system_message;
		char **system_message_array;
		for (int i = 0; i < count_tags; i++) {
			if (weechat_string_match(tags[i], "system-msg=*", 1)) {
				system_message_array = weechat_string_split(tags[i], "=", 0, 2, &count_system_message);
				/* Once we find system-msg, no need to continue for loop */
				i = count_tags;
			}
		}

		if (!system_message_array) {
			return NULL;
		}

		/* If above worked correctly, system_message_array[1] should have the message
		 * however, twitch is odd in that spaces are replaced with \\s
		 */

		char *system_message = weechat_string_replace(system_message_array[1], "\\s", " ");

		/* Done unless there is the optional text
		 * In that case, append
		 */

		char *string_comment = NULL;
		int is_key_text = weechat_hashtable_has_key(hashtable_message_parse, "text");
		int is_key_pos = weechat_hashtable_has_key(hashtable_message_parse, "pos_text");
		if ((is_key_text != 0) && (is_key_pos != 0)) {
			char *pos_value = weechat_hashtable_get(hashtable_message_parse, "pos_text");
			if (weechat_strcasecmp(pos_value, "-1") != 0) {
				char *string_comment_prefix = " [Comment: ";
				int length_comment_prefix = strlen(string_comment_prefix);

				int length_text = strlen(weechat_hashtable_get(hashtable_message_parse, "text"));
				char *string_comment_text = weechat_strndup(weechat_hashtable_get(hashtable_message_parse, "text"), length_text + 1);

				char *string_comment_suffix = "]";
				int length_comment_suffix = strlen(string_comment_suffix);

				int length_comment = length_comment_prefix + length_text + length_comment_suffix + 1;

				string_comment = calloc(length_comment, sizeof(char));
				snprintf(string_comment, length_comment, "%s%s%s", string_comment_prefix, string_comment_text, string_comment_suffix);

				weechat_printf(buffer_server,
				               "%s%s%s",
				               weechat_prefix("network"),
				               system_message,
				               string_comment
				);
				weechat_printf(buffer_channel,
				               "%s%s%s",
				               weechat_prefix("network"),
				               system_message,
				               string_comment
				);

				/* Stuff to Free only if text */
				free(string_comment_text);
			} else {
				weechat_printf(buffer_server,
				               "%s%s",
				               weechat_prefix("network"),
				               system_message
				);
				weechat_printf(buffer_channel,
				               "%s%s",
				               weechat_prefix("network"),
				               system_message
				);
			}
		} else {
			weechat_printf(buffer_server,
				       "%s%s",
				       weechat_prefix("network"),
				       system_message
			);
			weechat_printf(buffer_channel,
				       "%s%s",
				       weechat_prefix("network"),
				       system_message
			);
		}

		/* Stuff to Free only if tags */
		free(system_message);
		weechat_string_free_split(system_message_array);
		weechat_string_free_split(tags);

	}

	/* Stuff to Free */
	free(string_buffer);
	free(string_server_name);
	free(string_channel_name);
	weechat_hashtable_free(hashtable_message_parse);

	/* Stuff to Return
	 * For some reason returning an empty string gets rid of the command not found message
	 */

	int length_return = 2;
	char *result = malloc(length_return);
	snprintf(result, length_return, "%s", "");
	return result;
}

int weechat_plugin_init(struct t_weechat_plugin *plugin,
	                int argc,
	                char *argv[]) {

	weechat_plugin = plugin;

	hook_usernotice = weechat_hook_modifier("irc_in_USERNOTICE",
	                                        &cb_modifier_usernotice,
	                                        NULL,
	                                        NULL);

	hook_clearchat = weechat_hook_modifier("irc_in_CLEARCHAT",
	                                       &cb_modifier_clearchat,
	                                       NULL,
	                                       NULL);

	return WEECHAT_RC_OK;
}

int weechat_plugin_end (struct t_weechat_plugin *plugin) {
	/* make C compiler happy */
	(void) plugin;

	if(hook_usernotice) {
		weechat_unhook(hook_usernotice);
		hook_usernotice = NULL;
	}

	return WEECHAT_RC_OK;
}
