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
  * 
  * This script checks stream status of any channel on any servers
  * listed in the "plugins.var.python.twitch.servers" setting. When you
  * switch to a buffer it will display updated infomation about the stream
  * in the title bar. Typing '/twitch' in buffer will also fetch updated
  * infomation. '/whois nick' will lookup user info and display it in current
  *
  * This code is heavily based of of the work by mumixam located at
  * https://github.com/mumixam/weechat-twitch
  *
  **/

#include <stdlib.h>

#include <stdio.h>
#include <string.h>
#include "weechat-plugin.h"

WEECHAT_PLUGIN_NAME("Twitch");
WEECHAT_PLUGIN_DESCRIPTION("Twitch plugin for WeeChat");
WEECHAT_PLUGIN_AUTHOR("ALurker <ALurker@outlook.com>");
WEECHAT_PLUGIN_VERSION("0.1");
WEECHAT_PLUGIN_LICENSE("GPL3");

struct t_weechat_plugin *weechat_plugin = NULL;
struct t_hook *usernotice_hook = NULL;

char* usernotice_modifier_cb(const void *pointer,
                             void *data,
                             const char *modifier,
                             const char *modifier_data,
                             const char *string) {

	weechat_printf(NULL, "Entered USERNOTICE");

	if (!string) {
		return NULL;
	}

	weechat_printf(NULL, "Entered USERNOTICE; String != NULL");

	//char *color_prefix_network = weechat_color("chat_prefix_network");
	//char *color_chat = weechat_color("chat");

	struct t_hashtable *hashtable_message_in;
	struct t_hashtable *hashtable_message_parse;

	weechat_printf(NULL, "Creating weechat_hashtable_new");

	hashtable_message_in = weechat_hashtable_new(8,
	                                             WEECHAT_HASHTABLE_STRING,
	                                             WEECHAT_HASHTABLE_STRING,
	                                             NULL,
	                                             NULL);

	if (!hashtable_message_in) {
		return NULL;
	}

	weechat_printf(NULL, "Created weechat_hashtable_new");

	weechat_hashtable_set(
		hashtable_message_in,
		"message",
		string
	);
	hashtable_message_parse = weechat_info_get_hashtable(
		"irc_message_parse",
		hashtable_message_in
	);

	/* irc_in_USERNOTICE will have servername in modifier_data */
	int length_server_name = strlen(modifier_data);

	char *channel_name = weechat_hashtable_get(
		hashtable_message_parse,
		"channel"
	);
	int length_channel_name = strlen(channel_name);
	/* Final string will be "server.channel\0" */
	int length_string = 2 + length_channel_name + length_server_name;
	char *string_buffer = calloc(length_string, sizeof(char));
	snprintf(string_buffer,
	         length_string,
	         "%s.%s",
	         modifier_data,
	         channel_name
	);
	struct t_gui_buffer *buffer_irc = weechat_buffer_search("irc", string_buffer);
	free(string_buffer);

	if (weechat_hashtable_has_key(hashtable_message_parse, "tags")) {
		weechat_printf(NULL, "tags exists");
		int count_tags;
		char **tags;
		tags = weechat_string_split(
			weechat_hashtable_get(
				hashtable_message_parse,
				"tags"
			),
			";",
			0,
			0,
			&count_tags
		);
		char **sys_msg;
		for (int i = 0; i < count_tags; i++) {
			int is_system_msg = weechat_string_match(tags[i], "system-msg*", 1);
			if (is_system_msg == 1) {
				int count_msg;
				sys_msg = weechat_string_split(
					tags[i],
					"=",
					0,
					2,
					&count_msg
				);
				/* at this point, no need to continue search */
				i = count_tags;
			}
		}
		weechat_printf(NULL, "Checking sys_msg");
		if (!sys_msg) {
			return NULL;
		}
		weechat_printf(NULL, "sys_msg exists");
		char *sys_msg_readable = weechat_string_replace(sys_msg[1], "\\s", " ");
		weechat_string_free_split(sys_msg);
		weechat_string_free_split(tags);

		char *text_buffer = NULL;
		if (weechat_hashtable_has_key(hashtable_message_parse, "text")) {
			char *comment = " [Comment] ";
			int length_comment = strlen(comment);
			char *text = weechat_hashtable_get(
				hashtable_message_parse,
				"text"
			);
			int length_text = strlen(text);
			int length_full = length_comment + length_text + strlen(sys_msg_readable) + 1;
			text_buffer = calloc(length_full, sizeof(char));
			snprintf(text_buffer,
				length_full,
				"%s%s%s",
				sys_msg_readable,
				comment,
				text
			);
		} else {
			weechat_printf(NULL, "No \"text\" in message_parse");
		}
		weechat_printf(NULL, "checking text_buffer");
		if (!text_buffer) {
			return NULL;
		}
		weechat_printf(NULL, "text_buffer exists");

		int length_cpn = strlen(weechat_color("chat_prefix_network"));
		int length_chat = strlen(weechat_color("chat"));
		int length_text = strlen(text_buffer);

		int length_final = length_cpn + length_chat + length_text + 1;
		char *text_final = calloc(length_final, sizeof(char));
		snprintf(text_final,
		         length_final,
		         "%s--%s %s",
		         weechat_color("chat_prefix_network"),
		         weechat_color("chat"),
		         text_buffer
		);

		weechat_printf(
			buffer_irc,
			"%s",
			text_final
		);

		/* After Use */
		free(sys_msg_readable);
		free(text_buffer);

		//return text_final;
		return "";
	}
	return NULL;
}

int usernotice_signal_cb(const void *pointer,
                         void *data,
                         const char *signal,
                         const char *type_data,
                         void *signal_data) {

	weechat_printf(NULL, "Entered USERNOTICE");

	if (!signal_data) {
		weechat_printf(NULL, "Error: No signal_data");
		return WEECHAT_RC_ERROR;
	}

	weechat_printf(NULL, "Entered USERNOTICE; String == %s", signal_data);

	struct t_hashtable *hashtable_message_in;
	struct t_hashtable *hashtable_message_parse;

	weechat_printf(NULL, "Creating hashtable_message_in");

	hashtable_message_in = weechat_hashtable_new(8,
	                                             WEECHAT_HASHTABLE_STRING,
	                                             WEECHAT_HASHTABLE_STRING,
	                                             NULL,
	                                             NULL);

	if (!hashtable_message_in) {
		weechat_printf(NULL, "Error: No hashtable_message_in");
		return WEECHAT_RC_ERROR;
	}

	weechat_printf(NULL, "Created hashtable_message_in");

	weechat_hashtable_set(
		hashtable_message_in,
		"message",
		signal_data
	);

	if (weechat_hashtable_has_key(hashtable_message_in, "message")) {
		weechat_printf(NULL, "Message in hashtable_message_in");
	}

	weechat_printf(NULL, "Creating hashtable_message_parse");

	hashtable_message_parse = weechat_info_get_hashtable(
		"irc_message_parse",
		hashtable_message_in
	);
	
	if (!hashtable_message_parse) {
		weechat_printf(NULL, "Error: No hashtable_message_parse");
		return WEECHAT_RC_ERROR;
	}

	weechat_printf(NULL, "Created hashtable_message_parse");

	weechat_hashtable_free(hashtable_message_in);
	weechat_hashtable_free(hashtable_message_parse);

	weechat_printf(NULL, "Freed the hashtables");

	return WEECHAT_RC_OK;
}

int weechat_plugin_init(struct t_weechat_plugin *plugin,
	                int argc,
	                char *argv[]) {

	weechat_plugin = plugin;

	//usernotice_hook = weechat_hook_modifier("irc_in_USERNOTICE",
	//                      &usernotice_modifier_cb,
	//		      NULL,
	//		      NULL);

	usernotice_hook = weechat_hook_signal("*,irc_in_USERNOTICE",
	                      &usernotice_signal_cb,
			      NULL,
			      NULL);

	/**weechat_hook_command ("double",
	*                    "Display two times a message "
	*                    "or execute two times a command",
	*                    "message | command",
	*                    "message: message to display two times\n"
	*                    "command: command to execute two times",
	*                    NULL,
	*                    &command_double_cb, NULL, NULL);
	**/

	return WEECHAT_RC_OK;
}

int weechat_plugin_end (struct t_weechat_plugin *plugin) {
	/* make C compiler happy */
	(void) plugin;

	if(usernotice_hook) {
		weechat_unhook(usernotice_hook);
		usernotice_hook = NULL;
	}

	return WEECHAT_RC_OK;
}

