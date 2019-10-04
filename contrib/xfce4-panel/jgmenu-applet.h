/*
* Copyright © 2019
*
* Distributed under terms of the GPL2 license.
*/

#ifndef SRC_HEADERS_JGMENU_H_
#define SRC_HEADERS_JGMENU_H_

G_BEGIN_DECLS

#define DEFAULT_ICON_NAME "jgmenu"
#define DEFAULT_TOOLTIP_MESSAGE "Applications Menu"
#define DEFAULT_TITLE "jgmenu"
#define DEFAULT_RUN_COMMAND "jgmenu_run"
#define DEFAULT_RUN_COMMAND_AT_POINTER "jgmenu --at-pointer"
#define XFCE_PLUGIN_VERSION "0.1"

typedef struct _JgmenuPlugin {
    XfcePanelPlugin *plugin;

    GtkWidget       *button;
    GtkWidget       *icon;

    gchar           *icon_name;
}
JgmenuPlugin;


G_END_DECLS


#endif /* SRC_HEADERS_JGMENU_H_ */
