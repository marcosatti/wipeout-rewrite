#ifndef PAUSE_MENU_H
#define PAUSE_MENU_H

#include "wipeout/menu.h"

#ifdef __cplusplus
extern "C" {
#endif

void ingame_menus_load(void);

menu_t *pause_menu_init(void);
menu_t *game_over_menu_init(void);
menu_t *race_stats_menu_init(void);
menu_t *text_scroll_menu_init(char * const *lines, int len);

#ifdef __cplusplus
}
#endif

#endif
