/* bombs & projcetiles */
#include "raylib.h"
#include <stdlib.h>

extern int GRID_SIZE;

#include "events/bombs.h"

extern int playerX, playerY;
extern bool gameOver;

Bomb *bombs = NULL;
int bombCount = 0;

// bomb projectiles
Projectile *projectiles = NULL;
int projectileCount = 0;

// ===================== BOMBS =====================
// Dynamically spawn bombs
void SpawnBombs(int amount) {
  int cols = GetScreenWidth()/GRID_SIZE;
  int rows = GetScreenHeight()/GRID_SIZE;

  bombs = realloc(bombs, sizeof(Bomb) * (bombCount + amount));
  for (int i = 0; i < amount; i++) {
    Bomb* b = &bombs[bombCount + i];
    b->x = (rand() % cols) * GRID_SIZE;          // random X position
    b->y = (rand() % rows) * GRID_SIZE;          // random Y position
    b->timer = 0.5f + ((float)rand()/RAND_MAX)*0.8f;  // countdown to explosion
    b->exploded = false;                         // bomb hasn't exploded yet
    b->explosionTimer = 0;                       // timer for explosion effect
    b->active = true;                            // bomb is active
  }
  bombCount += amount;
}

// Update bomb timers and trigger explosions
void UpdateBombs(float dt) {
  float speed = (GRID_SIZE * 0.5f);             // projectile speed
  //int cols = GetScreenWidth()/GRID_SIZE;
  //int rows = GetScreenHeight()/GRID_SIZE;

  for (int i = 0; i < bombCount; i++) {
    if (!bombs[i].active) continue;

    if (!bombs[i].exploded) {
      bombs[i].timer -= dt;                      // countdown to explosion
      if (bombs[i].timer <= 0) {
        bombs[i].exploded = true;               // bomb explodes
        bombs[i].explosionTimer = 0.5f;         // set explosion duration

        // --- Check player in explosion area ---
        for(int dx=-1; dx<=1; dx++)
          for(int dy=-1; dy<=1; dy++) {
            Rectangle expRect = {bombs[i].x + dx*GRID_SIZE, bombs[i].y + dy*GRID_SIZE, GRID_SIZE, GRID_SIZE};
            Rectangle playerRect = {playerX, playerY, GRID_SIZE, GRID_SIZE};
            if(CheckCollisionRecs(playerRect, expRect)) gameOver = true;
          }

        // spawn projectiles in 4 directions
        int dirs[4][2]={{1,0},{-1,0},{0,1},{0,-1}};
        for (int d = 0; d < 4; d++) {
          for (int p = 0; p < projectileCount; p++) {
            if (!projectiles[p].active) {
              projectiles[p].active = true;
              projectiles[p].x = bombs[i].x;
              projectiles[p].y = bombs[i].y;
              projectiles[p].vx = dirs[d][0] * speed * 60;  // X velocity
              projectiles[p].vy = dirs[d][1] * speed * 60;  // Y velocity
              break;
            }
          }
        }
      }
    } else {
      bombs[i].explosionTimer -= dt;             // countdown for explosion effect
      if (bombs[i].explosionTimer <= 0) bombs[i].active = false; // deactivate bomb
    }
  }
}

// Draw bombs and explosion effect
void DrawBombs() {
  for (int i = 0; i < bombCount; i++) {
    if (!bombs[i].active) continue;

    if (!bombs[i].exploded) DrawRectangle(bombs[i].x,bombs[i].y,GRID_SIZE,GRID_SIZE,ORANGE);
    else if (bombs[i].explosionTimer > 0) {
      // Draw 3x3 explosion area
      for(int dx=-1; dx<=1; dx++)
        for(int dy=-1; dy<=1; dy++)
          DrawRectangle(bombs[i].x+dx*GRID_SIZE,bombs[i].y+dy*GRID_SIZE,GRID_SIZE,GRID_SIZE,Fade(RED,0.7f));
    }
  }
}

// ===================== PROJECTILES =====================
// Dynamically allocate projectiles
void SpawnProjectiles(int amount) {
  projectiles = realloc(projectiles, sizeof(Projectile) * (projectileCount + amount));
  for (int i = 0; i < amount; i++) {
    projectiles[projectileCount + i].active = false; // initialize as inactive
  }
  projectileCount += amount;
}

// Update projectile positions and collisions
void UpdateProjectiles() {
  int screenW = GetScreenWidth();
  int screenH = GetScreenHeight();

  for (int i = 0; i < projectileCount; i++) {
    if (!projectiles[i].active) continue;

    projectiles[i].x += projectiles[i].vx * GetFrameTime();  // move projectile
    projectiles[i].y += projectiles[i].vy * GetFrameTime();

    // Deactivate if out of screen
    if (projectiles[i].x < 0 || projectiles[i].y < 0 || 
        projectiles[i].x >= screenW || projectiles[i].y >= screenH) {
      projectiles[i].active = false;
      continue;
    }

    // --- Check collision with player ---
    Rectangle playerRect = {playerX, playerY, GRID_SIZE, GRID_SIZE};
    Rectangle projRect = {projectiles[i].x, projectiles[i].y, GRID_SIZE, GRID_SIZE};
    if(CheckCollisionRecs(playerRect, projRect)) gameOver = true;
  }
}

// Draw active projectiles
void DrawProjectiles() {
  for (int i = 0; i < projectileCount; i++)
    if (projectiles[i].active)
      DrawRectangle((int)projectiles[i].x,(int)projectiles[i].y,GRID_SIZE,GRID_SIZE,YELLOW);
}
