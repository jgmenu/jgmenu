#ifndef JGMENU_SCHEMA_H
#define JGMENU_SCHEMA_H

struct key_value_pair {
	char *key;
	char *value;
};

/* clang-format off */
static struct key_value_pair schema_builtin[] = {
	{ "Name", "Accessories" },
	{ "Name[sv]", "Tillbehör" },
	{ "Icon", "applications-accessories" },
	{ "Categories", "Accessibility;Core;Utility;" },

	{ "Name", "Development" },
	{ "Name[sv]", "Utveckling" },
	{ "Icon", "applications-development" },
	{ "Categories", "Development;" },

	{ "Name", "Education" },
	{ "Name[sv]", "Utbildning" },
	{ "Icon", "applications-science" },
	{ "Categories", "Education;" },

	{ "Name", "Games" },
	{ "Name[sv]", "Spel" },
	{ "Icon", "applications-games" },
	{ "Categories", "Game;" },

	{ "Name", "Graphics" },
	{ "Name[sv]", "Grafik" },
	{ "Icon", "applications-graphics" },
	{ "Categories", "Graphics;" },

	{ "Name", "Multimedia" },
	{ "Name[sv]", "Multimedia" },
	{ "Icon", "applications-multimedia" },
	{ "Categories", "Audio;Video;AudioVideo;" },

	{ "Name", "Internet" },
	{ "Name[sv]", "Internet" },
	{ "Icon", "applications-internet" },
	{ "Categories", "Network;" },

	{ "Name", "Office" },
	{ "Name[sv]", "Kontorsprogram" },
	{ "Icon", "applications-office" },
	{ "Categories", "Office;" },

	{ "Name", "Other" },
	{ "Name[sv]", "Övrigt" },
	{ "Icon", "applications-other" },

	{ "Name", "Settings" },
	{ "Name[sv]", "Inställningar" },
	{ "Icon", "preferences-desktop" },
	{ "Categories", "Settings;Screensaver;" },

	{ "Name", "System" },
	{ "Name[sv]", "System" },
	{ "Icon", "applications-system" },
	{ "Categories", "Emulator;System;" },

	{ NULL, NULL }
};

/* clang-format on */

#endif /* JGMENU_SCHEMA_H */
