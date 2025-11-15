#ifndef BOMBS_H
#define BOMBS_H
#include <stdbool.h>

/* Bomb structure */
typedef struct {
  int x, y;                  // coordinates
  float timer;               // time to explode
  bool exploded;             // true if exploded
  float explosionTimer;      // from explosion to disappearing
  bool active;               // is bomb active
} Bomb;

/* Bomb projectiles structure */
typedef struct {
  float x, y;                // coordinates
  float vx, vy;              // velocity
  bool active;               // is projectile active
} Projectile;

// Functions
void SpawnBombs(int amount);
void UpdateBombs(float dt); 
void DrawBombs(void);

void SpawnProjectiles(int amount);
void UpdateProjectiles(void);
void DrawProjectiles(void);

#endif // BOMBS_H
