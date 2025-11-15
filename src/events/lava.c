#include <stdlib.h>
#include "raylib.h"

#include "events/lava.h"

// ===================== LAVA =====================

LavaTile *lava = NULL;
int lavaCount = 0;

extern int playerX, playerY;
extern int score;
extern float scoreTimer;
extern bool gameOver;

extern int GRID_SIZE;

// Dynamically spawn lava tiles
void SpawnLava(int amount) {
  int cols = GetScreenWidth()/GRID_SIZE;
  int rows = GetScreenHeight()/GRID_SIZE;

  lava = realloc(lava, sizeof(LavaTile)*(lavaCount + amount));
  for (int i = 0; i < amount; i++) {
    int x, y;
    bool validPos;
  
    do {
      validPos = true;
      x = (rand() % cols) * GRID_SIZE;
      y = (rand() % rows) * GRID_SIZE;

      // Check if it isnt 3x3 around player
      for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
          int gx = playerX + dx * GRID_SIZE;
          int gy = playerY + dy * GRID_SIZE;
          if (x == gx && y == gy) {
            validPos = false;
          }
        }
      }
    } while (!validPos);

    LavaTile* l = &lava[lavaCount + i];
    l->x = x;                                // random X position
    l->y = y;                                // random Y position
    l->active = true;                      // tile is active
    l->appearTimer = i*0.1f;               // gradual appearance timing
    l->fullTimer = 2.0f;                   // full active duration
  }
  lavaCount += amount;                     // update total lava count
}

// Update lava timers and check player collision
void UpdateLava() {
  int screenW = GetScreenWidth();
  int screenH = GetScreenHeight();

  for (int i = 0; i < lavaCount; i++) {
    if (!lava[i].active) continue;

    // Fade-in phase: tile gradually appears
    if (lava[i].appearTimer > 0) {
      lava[i].appearTimer -= GetFrameTime();
      if (lava[i].appearTimer < 0) lava[i].appearTimer = 0;
    } else {
      // Fully active phase: countdown for dangerous lava
      lava[i].fullTimer -= GetFrameTime();
      if (lava[i].fullTimer < -1) lava[i].fullTimer = -1; // finished
    }

    // Check collision with player only during active phase
    if (lava[i].fullTimer >= 0 &&
        playerX >= 0 && playerX < screenW &&
        playerY >= 0 && playerY < screenH &&
        playerX == lava[i].x && playerY == lava[i].y)
      gameOver = true;
  }
}

// Draw lava tiles on screen
void DrawLava() {
  for (int i = 0; i < lavaCount; i++) {
    if (!lava[i].active) continue;

    // Partially appeared lava (transparent)
    if (lava[i].appearTimer > 0)
      DrawRectangle(lava[i].x, lava[i].y, GRID_SIZE, GRID_SIZE, Fade(RED, 0.3f));
    // Fully active lava (solid red)
    else if (lava[i].fullTimer > 0)
      DrawRectangle(lava[i].x, lava[i].y, GRID_SIZE, GRID_SIZE, RED);
  }
}
