#ifndef LAVA_H
#define LAVA_H
#include <stdbool.h>

/* Single lava tile structure */
typedef struct {
  int x, y;                // coordinates
  bool active;             // is active
  float appearTimer;       // time from appearing on screen
  float fullTimer;         // time from lava activating
} LavaTile;

// Functions
void SpawnLava(int amount);      // only amount, cols/rows dynamic
void UpdateLava(void);           // update timers and check collision
void DrawLava(void);             // draw lava tiles

#endif // LAVA_H
