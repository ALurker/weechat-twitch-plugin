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

WEECHAT_PLUGIN_NAME("twitch");
WEECHAT_PLUGIN_DESCRIPTION("Twitch plugin for WeeChat");
WEECHAT_PLUGIN_AUTHOR("ALurker <ALurker@outlook.com>");
WEECHAT_PLUGIN_VERSION("0.1");
WEECHAT_PLUGIN_LICENSE("GPL3");

struct t_weechat_plugin *weechat_plugin = NULL;
struct t_hook *hook_usernotice = NULL;
struct t_hook *hook_clearchat = NULL;
struct t_hook *hook_roomstate = NULL;

char* cb_modifier_roomstate(const void *pointer,
                            void *data,
                            const char *modifier,
                            const char *modifier_data,
                            const char *string) {
	/* Example Message:
	 * 15:23:44 --> ZNC-Tw+| @badges=;color=;display-name=TheRealLurker;emote-sets=0;mod=0;subscriber=0;user-type= :tmi.twitch.tv USERSTATE #day9tv
	 * 15:23:44 --> ZNC-Tw+| @broadcaster-lang=;emote-only=0;r9k=0;slow=0;subs-only=0 :tmi.twitch.tv ROOMSTATE #day9tv                             
	 */

	struct t_hashtable *hashtable_message_parse = twitch_get_message(string);

	if (!weechat_hashtable_has_key(hashtable_message_parse, "tags")) {
		weechat_printf(NULL, "No Tags in hashtable_message_parse");
		return NULL;
	}

	/* Get the Channel Buffer */
	char *string_buffer_plugin = "irc";

	int length_server = strlen(modifier_data);
	char *string_server = calloc(length_server + 1, sizeof(char));
	snprintf(string_server, length_server + 1, "%s", modifier_data);

	char *string_channel = twitch_get_channel(hashtable_message_parse);
	int length_channel = strlen(string_channel);

	/* Include the +1 below for the period between server and channel */
	int length_buffer_channel = length_server + length_channel + 1;
	char *string_buffer_channel = calloc(length_buffer_channel + 1, sizeof(char));
	snprintf(string_buffer_channel, length_buffer_channel + 1, "%s.%s", string_server, string_channel);
	struct t_gui_buffer *buffer_channel = weechat_buffer_search(string_buffer_plugin, string_buffer_channel);

	/* Potential Tags
	 * 15:23:44 --> ZNC-Tw+| @broadcaster-lang=;emote-only=0;r9k=0;slow=0;subs-only=0 :tmi.twitch.tv ROOMSTATE #day9tv
	 * broadcaster-lang
	 * emote-only
	 * r9k
	 * slow
	 * subs-only
	 */

	int count_tags = 0;
	char **tags = weechat_string_split(weechat_hashtable_get(hashtable_message_parse, "tags"), ";", 0, 0, &count_tags);

	if (!tags) {
		weechat_printf(NULL, "Unable to split hashtable_message_parse into tags");
		return NULL;
	}

	char *string_language = NULL;
	char *string_emote = NULL;
	char *string_r9k = NULL;
	char *string_slow = NULL;
	char *string_subs = NULL;
	for (int i = 0; i < count_tags; i++) {
		if (weechat_string_match(tags[i], "broadcaster-lang=*", 1)) {
			/* Assign Language to string_language
			 * "" => en
			 */
			string_language = twitch_parse_tag(tags[i]);
		} else if (weechat_string_match(tags[i], "emote-only=*", 1)) {
			/* Assign Emote Only to string_emote
			 * Valid should be 0 => False, 1 => True
			 */
			string_emote = twitch_parse_tag(tags[i]);
		} else if (weechat_string_match(tags[i], "r9k=*", 1)) {
			/* Assign r9k value to string_r9k
			 * TODO research what this value actually means
			 */
			string_r9k = twitch_parse_tag(tags[i]);
		} else if (weechat_string_match(tags[i], "slow=*", 1)) {
			/* Assign slow value to string_slow
			 * I'm given to understanding that different values
			 * mean different levels of slow.
			 * TODO research on the possible values here
			 */
			string_slow = twitch_parse_tag(tags[i]);
		} else if (weechat_string_match(tags[i], "subs-only=*", 1)) {
			/* Assign subs-only value to string_subs
			 * 0 => False, 1 => True
			 */
			string_subs = twitch_parse_tag(tags[i]);
		}
	}

	/* We only care if the variable has changed for that buffer */
	if(string_language) {
		twitch_buffer_update_local(buffer_channel, "lang", string_language);
	}

	if(string_emote) {
		twitch_buffer_update_local(buffer_channel, "emote", string_emote);
	}
	if(string_r9k) {
		twitch_buffer_update_local(buffer_channel, "r9k", string_r9k);
	}
	if(string_slow) {
		twitch_buffer_update_local(buffer_channel, "slow", string_slow);
	}
	if(string_subs) {
		twitch_buffer_update_local(buffer_channel, "subs", string_subs);
	}

	/* Stuff to Free */
	if(string_language) {
		free(string_language);
	}
	if(string_emote) {
		free(string_emote);
	}
	if(string_r9k) {
		free(string_r9k);
	}
	if(string_slow) {
		free(string_slow);
	}
	if(string_subs) {
		free(string_subs);
	}
	free(string_server);
	free(string_channel);
	free(string_buffer_channel);
	weechat_string_free_split(tags);
	weechat_hashtable_free(hashtable_message_parse);

	/* Stuff to Return
	 * For some reason returning an empty string gets rid of the command not found message
	 */
	int length_return = 2;
	char *result = malloc(length_return);
	snprintf(result, length_return, "%s", "");
	return result;
}

char* cb_modifier_clearchat(const void *pointer,
                            void *data,
                            const char *modifier,
                            const char *modifier_data,
                            const char *string) {

	struct t_hashtable *hashtable_message_parse = twitch_get_message(string);

	if (!hashtable_message_parse) {
		return NULL;
	}

	/* Server Name should be in modifier_data */
	int length_server = strlen(modifier_data);
	
	/* Increment by 1 to include \0
	 * API refernce documentation doesn't inclue the +1 incrementation
	 * Assuming not necessary
	 */

	char *string_server = weechat_strndup(modifier_data, length_server);

	if (!weechat_hashtable_has_key(hashtable_message_parse, "channel")) {
		return NULL;
	}

	//weechat_printf(NULL, "");
	//weechat_printf(NULL, "Server: %s", string_server);

	int length_channel = strlen(weechat_hashtable_get(hashtable_message_parse, "channel"));
	char *string_channel = weechat_strndup(weechat_hashtable_get(hashtable_message_parse, "channel"), length_channel + 1);

	//weechat_printf(NULL, "Channel: %s", string_channel);

	/* These messages have the user whose message is cleared in text
	 * No user present implies everybodies chat was cleared
	 */

	if (!weechat_hashtable_has_key(hashtable_message_parse, "text")) {
		return NULL;
	}

	int length_user = strlen(weechat_hashtable_get(hashtable_message_parse, "text"));
	char *string_user = weechat_strndup(weechat_hashtable_get(hashtable_message_parse, "text"), length_user + 1);

	//weechat_printf(NULL, "Length User: %d", length_user);
	//weechat_printf(NULL, "User: %s", string_user);

	/* Get the Channel Buffer */
	char *string_buffer_plugin = "irc";
	int length_buffer_channel = length_channel + length_server + 2;
	char *string_buffer_channel = calloc(length_buffer_channel, sizeof(char));
	snprintf(string_buffer_channel, length_buffer_channel, "%s.%s", string_server, string_channel);
	struct t_gui_buffer *buffer_channel = weechat_buffer_search(string_buffer_plugin, string_buffer_channel);

	if (!buffer_channel) {
		return NULL;
	}

	/* Get the Server Buffer */
	char *string_server_prefix = "server.";
	int length_server_prefix = strlen(string_server_prefix);
	int length_buffer_server = length_server + length_server_prefix;
	char *string_buffer_server = calloc(length_buffer_server + 1, sizeof(char));
	snprintf(string_buffer_server, length_buffer_server + 1, "%s%s", string_server_prefix, string_server);
	//weechat_printf(NULL, "String Buffer Server: %s", string_buffer_server);
	struct t_gui_buffer *buffer_server = weechat_buffer_search(string_buffer_plugin, string_buffer_server);

	if (!buffer_server) {
		return NULL;
	}

	int count_tags = 0;
	char **tags;
	if (!weechat_hashtable_has_key(hashtable_message_parse, "tags")) {
		return NULL;
	}
	/* Tags should be seperated by ; */
	tags = weechat_string_split(weechat_hashtable_get(hashtable_message_parse, "tags"),
				    ";",
				    0,
				    0,
				    &count_tags
	);

	int count_ban_reason = 0;
	char **ban_reason_array;
	int count_ban_duration = 0;
	char **ban_duration_array;
	for (int i = 0; i < count_tags; i++) {
		if (weechat_string_match(tags[i], "ban-reason=*", 1)) {
			ban_reason_array = weechat_string_split(tags[i], "=", 0, 2, &count_ban_reason);
		} else if (weechat_string_match(tags[i], "ban-duration=*", 1)) {
			ban_duration_array = weechat_string_split(tags[i], "=", 0, 2, &count_ban_duration);
		}
	}

	char *ban_reason;
	int length_reason = 0;
	char *ban_duration;
	int length_duration = 0;
	/* if tag present, but empty string, count will be one */
	if (count_ban_reason > 1) {
		ban_reason = weechat_string_replace(ban_reason_array[1], "\\s", " ");
		length_reason = strlen(ban_reason);
		//weechat_printf(NULL, "Ban Reason: %s", ban_reason);
	}
	if (count_ban_duration > 1) {
		ban_duration = weechat_string_replace(ban_duration_array[1], "\\s", " ");
		length_duration = strlen(ban_duration);
		//weechat_printf(NULL, "Ban Duration: %s seconds", ban_duration);
	}

	/* ban_reason but no ban_duration => perm ban
	 * ban_reason and ban_duration => timeout
	 * not ban_reason but ban_duration => timeout
	 * user but no ban_duration and no ban_reason => clear user chat
	 * no_user => chat clear
	 */

	char *string_output;
	int length_output = 0;
	int length_prefix = strlen(weechat_prefix("network"));
	char *string_prefix = weechat_strndup(weechat_prefix("network"), length_prefix);

	if (length_user <= 0) {
		/* No User => Clear Chat */
		char *string_primary = ": Entire Chat Cleared by Moderator";
		int length_primary = strlen(string_primary);

		length_output = length_prefix;
		length_output += length_channel;
		length_output += length_primary;

		string_output = calloc(length_output + 1, sizeof(char));
		snprintf(string_output, length_output + 1, "%s%s%s", string_prefix, string_channel, string_primary);
	} else {
		if (count_ban_reason > 1) {
			if (count_ban_duration > 1) {
				/* Timeout with Reason */
				char *string_primary = ": ";
				int length_primary = strlen(string_primary);
				char *string_secondary = " has been timed out for ";
				int length_secondary = strlen(string_secondary);
				char *string_tertiary = " seconds. [Reason: ";
				int length_tertiary = strlen(string_tertiary);
				char *string_quaternary = "]";
				int length_quaternary = strlen(string_quaternary);

				length_output = length_prefix;
				length_output += length_channel;
				length_output += length_primary;
				length_output += length_user;
				length_output += length_secondary;
				length_output += length_duration;
				length_output += length_tertiary;
				length_output += length_reason;
				length_output += length_quaternary;

				string_output = calloc(length_output + 1, sizeof(char));
				snprintf(string_output,
				         length_output + 1,
				         "%s%s%s%s%s%s%s%s%s",
				         string_prefix,
					 string_channel,
					 string_primary,
				         string_user,
				         string_secondary,
				         ban_duration,
				         string_tertiary,
				         ban_reason,
				         string_quaternary
				);
			} else {
				/* Perm Ban */
				char *string_primary = ": ";
				int length_primary = strlen(string_primary);
				char *string_secondary = " has been banned. [Reason: ";
				int length_secondary = strlen(string_secondary);
				char *string_tertiary = "]";
				int length_tertiary = strlen(string_tertiary);

				length_output = length_prefix;
				length_output += length_channel;
				length_output += length_primary;
				length_output += length_user;
				length_output += length_secondary;
				length_output += length_reason;
				length_output += length_tertiary;

				string_output = calloc(length_output + 1, sizeof(char));
				snprintf(string_output,
				         length_output + 1,
				         "%s%s%s%s%s%s%s",
				         string_prefix,
					 string_channel,
					 string_primary,
				         string_user,
				         string_secondary,
				         ban_reason,
				         string_tertiary
				);
			}
		} else {
			if (count_ban_duration > 1) {
				/* Timeout without Reason */
				char *string_primary = ": ";
				int length_primary = strlen(string_primary);
				char *string_secondary = " has been timed out for ";
				int length_secondary = strlen(string_secondary);
				char *string_tertiary = " seconds.";
				int length_tertiary = strlen(string_tertiary);

				length_output = length_prefix;
				length_output += length_channel;
				length_output += length_primary;
				length_output += length_user;
				length_output += length_secondary;
				length_output += length_duration;
				length_output += length_tertiary;

				string_output = calloc(length_output + 1, sizeof(char));
				snprintf(string_output,
				         length_output + 1,
				         "%s%s%s%s%s%s%s",
				         string_prefix,
					 string_channel,
					 string_primary,
				         string_user,
				         string_secondary,
				         ban_duration,
				         string_tertiary
				);
			} else {
				/* Cleared User Chat */
				char *string_primary = ": ";
				int length_primary = strlen(string_primary);
				char *string_secondary = "'s Chat Cleared by Moderator";
				int length_secondary = strlen(string_secondary);

				length_output = length_prefix;
				length_output += length_channel;
				length_output += length_primary;
				length_output += length_user;
				length_output += length_secondary;

				string_output = calloc(length_output + 1, sizeof(char));
				snprintf(string_output,
				         length_output + 1,
				         "%s%s%s%s%s",
				         string_prefix,
					 string_channel,
					 string_primary,
				         string_user,
				         string_secondary
				);
			}
		}
	}

	if (!string_output) {
		return NULL;
	}

	weechat_printf(buffer_server, "%s", string_output);
	weechat_printf(buffer_channel, "%s", string_output);
	
	/* Stuff to Free */
	if (count_ban_reason > 1) {
		free(ban_reason);
	}
	if (count_ban_duration > 1) {
		free(ban_duration);
	}
	free(string_buffer_channel);
	free(string_output);
	free(string_prefix);
	weechat_hashtable_free(hashtable_message_parse);

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

	struct t_hashtable *hashtable_message_parse = twitch_get_message(string);

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

	if (!buffer_server) {
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

	hook_roomstate = weechat_hook_modifier("irc_in_ROOMSTATE",
	                                       &cb_modifier_roomstate,
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
	
	if(hook_clearchat) {
		weechat_unhook(hook_clearchat);
		hook_clearchat = NULL;
	}

	if(hook_roomstate) {
		weechat_unhook(hook_roomstate);
		hook_roomstate = NULL;
	}

	return WEECHAT_RC_OK;
}
