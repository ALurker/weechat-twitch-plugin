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

#ifndef TWITCH_PLUGIN_H
#define TWITCH_PLUGIN_H 1

extern char *twitch_parse_tag(char *tag);
extern char *twitch_get_channel(struct t_hashtable *hashtable);

extern struct t_hashtable *twitch_get_message(const char *string);
extern char *twitch_hashtable_get_string(struct t_hashtable *hashtable, const char *key);

extern int twitch_buffer_var_empty(struct t_gui_buffer *buffer, const char *localvar);
extern void twitch_buffer_update_local(struct t_gui_buffer *buffer, const char *var, const char *val);

extern char *twitch_build_privmsg_extended(const char *user, const char *host, const char *channel, const char *message);
extern char *twitch_build_privmsg(const char *host, const char *channel, const char *message);

#endif /* TWITCH_PLUGIN_H */
