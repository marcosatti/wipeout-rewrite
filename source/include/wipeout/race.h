#ifndef RACE_H
#define RACE_H

#ifdef __cplusplus
extern "C" {
#endif

void race_init(void);
void race_update(void);
void race_start(void);
void race_restart(void);
void race_pause(void);
void race_unpause(void);
void race_end(void);
void race_next(void);
void race_release_control(void);

#ifdef __cplusplus
}
#endif

#endif
