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

static const char jgmenu_plugin_copyright[] =
"Copyright \xc2\xa9 2019 Miloš Pavlović (plugin)\n"
"Copyright \xc2\xa9 Johan Malm (jgmenu)\n";

static void jgmenu_about(XfcePanelPlugin *plugin)
{
	const gchar * auth[] = { "Miloš Pavlović", NULL };
	GdkPixbuf *icon;

	icon = xfce_panel_pixbuf_from_source("jgmenu", NULL, 32);
	gtk_show_about_dialog(NULL,
			      "logo", icon,
			      "license", xfce_get_license_text(XFCE_LICENSE_TEXT_GPL),
			      "version", XFCE_PLUGIN_VERSION,
			      "program-name", "jgmenu-applet",
			      "comments", _("Starts Jgmenu"),
			      "website", "https://github.com/johanmalm/jgmenu",
			      "copyright", _(jgmenu_plugin_copyright),
			      "authors", auth,
			      NULL);
	if (icon)
		g_object_unref(G_OBJECT(icon));
}

static char **extend_env(int coordinate, char *variable, char **envp)
{
	envp = g_environ_setenv(envp, variable,
				g_strdup_printf("%" G_GINT16_FORMAT, coordinate),
				FALSE);
	return envp;
}

static void button_clicked(GtkWidget *button, XfcePanelPlugin *plugin)
{
	gint  x, y, panel_w, panel_h, button_width, button_height;
	gint panel_x1 = 0, panel_x2 = 0, panel_y1 = 0, panel_y2 = 0,
	     button_x1 = 0, button_x2 = 0, button_y1 = 0, button_y2 = 0;
	GdkRectangle    extents;
	GdkWindow       *window;
	GtkAllocation allocation;
	GdkScreen *screen;
	gint screen_width, screen_height;
	gchar **envp;

	screen = gdk_screen_get_default();
	if (!screen) {
		fprintf(stderr, "xfce4-plugin: gdk_screen_get_default() failed");
		return;
	}
	screen_width = gdk_screen_get_width(screen);
	screen_height = gdk_screen_get_height(screen);

	screen_width = screen_width / 2;
	screen_height = screen_height / 2;

	/* get parent window */
	window = gtk_widget_get_parent_window(button);

	/* parent's top-left screen coordinates */
	gdk_window_get_root_origin(window, &x, &y);

	/* parent's width and height */
	panel_w = gdk_window_get_width(window);
	panel_h = gdk_window_get_height(window);

	/* parent's extents (including decorations) */
	gdk_window_get_frame_extents(window, &extents);

	gtk_widget_get_allocation(button, &allocation);

	/* calculating x (assuming: left border size == right border size) */
	x = x + (extents.width - panel_w) / 2 + allocation.x;

	/* calculating y (assuming: left border size == right border size == bottom border size) */
	y = y + (extents.height - panel_h) - (extents.width - panel_w) / 2 + allocation.y;

	button_width = allocation.width;
	button_height = allocation.height;

	/* Panel Position */
	XfceScreenPosition position;

	position = xfce_panel_plugin_get_screen_position(plugin);
	if (xfce_screen_position_is_horizontal(position)) {
		/* horizontal */
		if (xfce_screen_position_is_top(position)) {
			/* top panel position */
			panel_x1 = extents.x;
			panel_x2 = extents.x + panel_w;
			panel_y1 = extents.y + panel_h;
			panel_y2 = panel_y1;
			button_x1 = x;
			button_x2 = x + button_width;
			button_y1 = y + button_height;
			button_y2 = y + button_height;
		}
		if (xfce_screen_position_is_bottom(position)) {
			/* bottom */
			panel_x1 = extents.x;
			panel_x2 = extents.x + panel_w;
			panel_y1 = extents.y;
			panel_y2 = extents.y;
			button_x1 = x;
			button_x2 = x + button_width;
			button_y1 = y;
			button_y2 = y;
		}
		if (xfce_screen_position_is_floating(position)) {
			/* floating */
			if (extents.y > screen_height) {
				/* bottom */
				panel_x1 = extents.x;
				panel_x2 = extents.x + panel_w;
				panel_y1 = extents.y;
				panel_y2 = extents.y;
				button_x1 = x;
				button_x2 = x + button_width;
				button_y1 = y;
				button_y2 = y;
			} else {
				/* top */
				panel_x1 = extents.x;
				panel_x2 = extents.x + panel_w;
				panel_y1 = extents.y + panel_h;
				panel_y2 = panel_y1;
				button_x1 = x;
				button_x2 = x + button_width;
				button_y1 = y + button_height;
				button_y2 = y + button_height;
			}
		}
	} else {
		/* vertical */
		if (xfce_screen_position_is_left(position)) {
			/* left */
			panel_x1 = extents.x + panel_w;
			panel_x2 = extents.x + panel_w;
			panel_y1 = extents.y;
			panel_y2 = extents.y + panel_h;
			button_x1 = x + button_width;
			button_x2 = button_x1;
			button_y1 = y;
			button_y2 = y + button_height;
		}
		if (xfce_screen_position_is_right(position)) {
			/* right */
			panel_x1 = extents.x;
			panel_x2 = extents.x;
			panel_y1 = extents.y;
			panel_y2 = extents.y + panel_h;
			button_x1 = x;
			button_x2 = x;
			button_y1 = y;
			button_y2 = y + button_height;
		}
		if (xfce_screen_position_is_floating(position)) {
			/* floating */
			if (extents.x > screen_width) {
				/* right */
				panel_x1 = extents.x;
				panel_x2 = extents.x;
				panel_y1 = extents.y;
				panel_y2 = extents.y + panel_h;
				button_x1 = x;
				button_x2 = x;
				button_y1 = y;
				button_y2 = y + button_height;
			} else {
				/* left */
				panel_x1 = extents.x + panel_w;
				panel_x2 = extents.x + panel_w;
				panel_y1 = extents.y;
				panel_y2 = extents.y + panel_h;
				button_x1 = x + button_width;
				button_x2 = button_x1;
				button_y1 = y;
				button_y2 = y + button_height;
			}
		}
	}

	envp = g_get_environ();

	envp = g_environ_unsetenv(envp, "TINT2_BUTTON_ALIGNED_X1");
	envp = g_environ_unsetenv(envp, "TINT2_BUTTON_ALIGNED_X2");
	envp = g_environ_unsetenv(envp, "TINT2_BUTTON_ALIGNED_Y1");
	envp = g_environ_unsetenv(envp, "TINT2_BUTTON_ALIGNED_Y2");
	envp = g_environ_unsetenv(envp, "TINT2_BUTTON_PANEL_X1");
	envp = g_environ_unsetenv(envp, "TINT2_BUTTON_PANEL_X2");
	envp = g_environ_unsetenv(envp, "TINT2_BUTTON_PANEL_Y1");
	envp = g_environ_unsetenv(envp, "TINT2_BUTTON_PANEL_Y2");

	if (button_x1 >= 0) {
		envp = extend_env(button_x1, "TINT2_BUTTON_ALIGNED_X1", envp);
		envp = extend_env(button_x2, "TINT2_BUTTON_ALIGNED_X2", envp);
		envp = extend_env(button_y1, "TINT2_BUTTON_ALIGNED_Y1", envp);
		envp = extend_env(button_y2, "TINT2_BUTTON_ALIGNED_Y2", envp);
		envp = extend_env(panel_x1, "TINT2_BUTTON_PANEL_X1", envp);
		envp = extend_env(panel_x2, "TINT2_BUTTON_PANEL_X2", envp);
		envp = extend_env(panel_y1, "TINT2_BUTTON_PANEL_Y1", envp);
		envp = extend_env(panel_y2, "TINT2_BUTTON_PANEL_Y2", envp);
	}

	/*
	 * Set IPC mode to ensure jgmenu reads the above environment variables
	 * The jgmenurc config file will only be updated if not already in IPC
	 * mode
	 */
	gchar * command_setup[] = { "jgmenu_run", "config", "-s",
				    "~/.config/jgmenu/jgmenurc", "-k",
				    "position_mode", "-v", "ipc", NULL };
	gchar * command[] = { DEFAULT_RUN_COMMAND, NULL };
	GError *error = NULL;

	g_spawn_sync(".", command_setup, envp, G_SPAWN_SEARCH_PATH,
		      NULL, NULL, NULL, NULL, NULL, &error);
	if (error)
		g_warning("unable to launch: %s", error->message);
	g_spawn_async(".", command, envp, G_SPAWN_SEARCH_PATH,
		      NULL, NULL, NULL, &error);
	if (error)
		g_warning("unable to launch: %s", error->message);
	g_strfreev(envp);
}

static JgmenuPlugin *jgmenu_init(XfcePanelPlugin *plugin)
{
	JgmenuPlugin *jgmenu = g_slice_new0(JgmenuPlugin);

	jgmenu->plugin = plugin;
	jgmenu->icon_name = g_strdup(DEFAULT_ICON_NAME);
	jgmenu->button = xfce_panel_create_button();
	gtk_widget_show(jgmenu->button);
	g_signal_connect(G_OBJECT(jgmenu->button), "clicked",
			 G_CALLBACK(button_clicked), plugin);
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

static gboolean jgmenu_size_changed(XfcePanelPlugin *plugin, gint size,
				    JgmenuPlugin *jgmenu)
{
	size = size / xfce_panel_plugin_get_nrows(plugin);
	gtk_widget_set_size_request(GTK_WIDGET(plugin), size, size);
	return TRUE;
}

static gboolean jgmenu_remote(XfcePanelPlugin *plugin, gchar *name,
			      GValue *value, JgmenuPlugin *jgmenu)
{
	g_return_val_if_fail(!value || G_IS_VALUE(value), FALSE);
	if (strcmp(name, "popup") == 0) {
		if (value && G_VALUE_HOLDS_BOOLEAN(value) &&
		    g_value_get_boolean(value) &&
		    gtk_widget_get_visible(GTK_WIDGET(plugin))) {
			/* popup here at mouse pointer */
			GError *error = NULL;

			g_spawn_command_line_async(DEFAULT_RUN_COMMAND_AT_POINTER, &error);
			if (error)
				g_warning("unable to launch: %s", error->message);
		} else {
			/* popup here, where X is an internal id
			* xfce4-panel --plugin-event=jgmenu-applet-X:popup:bool:false
			*/
			button_clicked(jgmenu->button, plugin);
		}
		return TRUE;
	}
	return FALSE;
}

static void jgmenu_construct(XfcePanelPlugin *plugin)
{
	JgmenuPlugin *jgmenu;

	jgmenu = jgmenu_init(plugin);
	gtk_container_add(GTK_CONTAINER(plugin), jgmenu->button);
	xfce_panel_plugin_add_action_widget(plugin, jgmenu->button);
	xfce_panel_plugin_menu_show_about(plugin);
	g_signal_connect(G_OBJECT(plugin), "free-data", G_CALLBACK(jgmenu_free), jgmenu);
	g_signal_connect(G_OBJECT(plugin), "size-changed", G_CALLBACK(jgmenu_size_changed), jgmenu);
	g_signal_connect(G_OBJECT(plugin), "remote-event", G_CALLBACK(jgmenu_remote), jgmenu);
	g_signal_connect(G_OBJECT(plugin), "about", G_CALLBACK(jgmenu_about), jgmenu);
}

XFCE_PANEL_PLUGIN_REGISTER(jgmenu_construct);
