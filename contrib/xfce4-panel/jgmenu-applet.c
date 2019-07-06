/*
* Copyright © 2019 misko_2083
*
* Distributed under terms of the GPL2 license.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif


#include <gtk/gtk.h>
#include <libxfce4util/libxfce4util.h>
#include <libxfce4panel/xfce-panel-plugin.h>
#include "jgmenu-applet.h"


#ifdef LIBXFCE4PANEL_CHECK_VERSION
#if LIBXFCE4PANEL_CHECK_VERSION(4, 12, 1)
#define HAS_PANEL_412
#endif
#endif


static void button_clicked(GtkWidget *button, JgmenuPlugin *jgmenu)
{
    /*
    * --------- TODO: ---------
    * Before starting jgmenu here need to:
    * - get button widget's aboslute position
    * gint wx, wy;
    * gdk_window_get_origin(gtk_widget_get_window(jgmenu->button), &wx, &wy);
    * - get button size (allocation->width and allocation->widthheight)
    * - get panel's screen position https://wiki.xfce.org/dev/howto/panel_plugins
    *
    * -------- Option I -------
    * - parse the /.config/jgmenu/jgmenurc config file and get menu height
    * + Menu width would also be useful for the panel(s) on the right;
    * - calculate where the jgmenu is supposed to be
    * - write corected menu_margin_x and menu_margin_x to the config file
    *
    * -------- Option II -------
    * - Implement new conf files to write the button positions/sizes and
    * the panel orientation.
    * Jgmenu would check those files before determing where to position
    *
    * -------- Option III ------
    * - For heaven's sake! Recalibrate and set some Tint2 env variables that
    * jgmenu checks for and let jgmenu do the heavy lifting and position itself
    *
    * ----------- End ---------
    * Useful links:
    *    https://git.xfce.org/panel-plugins/xfce4-sample-plugin/
    *    https://ecc-comp.blogspot.com/2015/02/a-simple-tutorial-for-xfce-panel-plugin.html?m=1
    *
    * For now simply run jgmenu_run
    */
    GError *error = NULL;
    g_spawn_command_line_async(DEFAULT_RUN_COMMAND, &error);
    if (error != NULL) {
        g_warning("unable to launch: %s", error->message);
    }
}


static JgmenuPlugin *
jgmenu_init(XfcePanelPlugin *plugin)
{
    JgmenuPlugin *jgmenu = g_slice_new0(JgmenuPlugin);
    jgmenu->plugin = plugin;
    jgmenu->icon_name = g_strdup(DEFAULT_ICON_NAME);
    jgmenu->button = xfce_panel_create_button();
    gtk_widget_show(jgmenu->button);
    g_signal_connect(G_OBJECT(jgmenu->button),
                     "clicked", G_CALLBACK(button_clicked), plugin);
    gtk_widget_set_tooltip_text(GTK_WIDGET(jgmenu->button),
                                DEFAULT_TOOLTIP_MESSAGE);
    jgmenu->icon = xfce_panel_image_new_from_source(jgmenu->icon_name);
    gtk_widget_show(jgmenu->icon);
    gtk_container_add(GTK_CONTAINER(jgmenu->button), jgmenu->icon);
    return jgmenu;
}



static void jgmenu_free(XfcePanelPlugin *plugin, JgmenuPlugin *jgmenu)
{
    gtk_widget_destroy(jgmenu->button);
    gtk_widget_destroy(jgmenu->icon);
    g_slice_free(JgmenuPlugin, jgmenu);
}



static gboolean jgmenu_size_changed(XfcePanelPlugin *plugin,
                                    gint size, JgmenuPlugin *jgmenu)
{
#ifdef HAS_PANEL_412
    size /= xfce_panel_plugin_get_nrows(plugin);
#endif
    gtk_widget_set_size_request(GTK_WIDGET(jgmenu->button), size, size);
    return TRUE;
}



static void jgmenu_construct(XfcePanelPlugin *plugin)
{
    JgmenuPlugin *jgmenu;
    jgmenu = jgmenu_init(plugin);
    gtk_container_add(GTK_CONTAINER(plugin), jgmenu->button);
    xfce_panel_plugin_add_action_widget(plugin, jgmenu->button);
    g_signal_connect(G_OBJECT(plugin),
                     "free-data", G_CALLBACK(jgmenu_free), jgmenu);
    g_signal_connect(G_OBJECT(plugin),
                     "size-changed", G_CALLBACK(jgmenu_size_changed), jgmenu);
}

XFCE_PANEL_PLUGIN_REGISTER(jgmenu_construct);
