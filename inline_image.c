/*
 * purple
 *
 * Purple is the legal property of its developers, whose names are too numerous
 * to list here.  Please refer to the COPYRIGHT file distributed with this
 * source distribution.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 */

#ifndef PURPLE_PLUGINS
#define PURPLE_PLUGINS 1
#endif

#include <string.h>
 
#include "conversation.h"
#include "plugin.h"
#include "notify.h"
#include "util.h"

PurplePlugin *plugin_handle = NULL;

static gboolean
writing_msg_cb(PurpleAccount *account, const char *who, char **message,
					PurpleConversation *conv, PurpleMessageFlags flags, gpointer null)
{
	PurpleConnectionFlags features;
	xmlnode *root, *img;	
	gboolean changed_message = FALSE;
	gchar *htmlmessage;
	
	features = purple_conversation_get_features(conv);
	
	if (!(features & PURPLE_CONNECTION_HTML)) {
		return FALSE;
	}
	
	htmlmessage = g_strconcat("<html>", *message, "</html>", NULL);
	root = xmlnode_from_str(htmlmessage, -1);
	
	for(img = xmlnode_get_child(root, "img");
		img;
		img = xmlnode_get_next_twin(img)) {
		
		const gchar *src = xmlnode_get_attrib(img, "src");
		if (g_str_has_prefix(src, "data:image/")) {
			const gchar *data_start;
			guchar *data;
			gsize len;
			gint id;
			
			src = purple_url_decode(src);
			data_start = strchr(src, ';');
			
			if (!data_start) {
				continue;
			}
			data_start++;
			
			if ((g_strstr_len(src, src - data_start, "base64,")) != NULL) {
				data = purple_base64_decode(data_start, &len);
			} else {
				data = (guchar *)g_strdup(data_start);
				len = -1;
			}
			
			id = purple_imgstore_add_with_id(data, len, NULL);
			
			if (id) {
				gchar *id_str = g_strdup_printf("%d", id);
				xmlnode_remove_attrib(img, "src");
				xmlnode_set_attrib(img, "data", id_str);
				g_free(id_str);
				
				changed_message = TRUE;
			}
			
			g_free(data);
		}
	}
	
	if (changed_message) {
		g_free(*message);
		*message = xmlnode_to_str(root, NULL);
	}
	xmlnode_free(root);
	g_free(htmlmessage);
	
	return FALSE;
}

static gboolean
plugin_load(PurplePlugin *plugin)
{
     void *handle = purple_conversations_get_handle();
     plugin_handle = plugin;
	 
     purple_signal_connect(handle, "writing-im-msg", plugin,
                PURPLE_CALLBACK(writing_msg_cb), NULL);
     purple_signal_connect(handle, "writing-chat-msg", plugin,
                PURPLE_CALLBACK(writing_msg_cb), NULL);

     return TRUE;
}


static PurplePluginInfo info =
{
     PURPLE_PLUGIN_MAGIC,
     2,
     10,
     PURPLE_PLUGIN_STANDARD,
     NULL,
     0,
     NULL,
     PURPLE_PRIORITY_DEFAULT,
     "inlineimages",
     "Inline Images",
     "1.0",
     "Displays inline images",
     "Changes inline images (e.g. images sent with data: uri's) to be visible in Pidgin.",
     "Eion Robb <eionrobb@gmail.com>",
     "http://eion.robbmob.com/blog",
     plugin_load,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
	 /* padding */
     NULL,
     NULL,
     NULL,
     NULL
};

 static void
 init_plugin(PurplePlugin *plugin)
 {
 }

PURPLE_INIT_PLUGIN(inlineimages, init_plugin, info)
