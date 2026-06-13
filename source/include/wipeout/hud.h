#ifndef HUD_H
#define HUD_H

#include "wipeout/ship.h"

#ifdef __cplusplus
extern "C" {
#endif

void hud_load(void);
void hud_draw(ship_t *ship);

#ifdef __cplusplus
}
#endif

#endif
