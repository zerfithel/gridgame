/* grid game */
#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <limits.h>

#include "events/lava.h"
#include "events/bombs.h"

#ifndef PATH_MAX
  #define PATH_MAX 4096
#endif

// Default grid size
int GRID_SIZE = 30; // global variable

// Global events
extern Bomb *bombs;
extern int bombCount;

extern Projectile *projectiles;
extern int projectileCount;

extern LavaTile *lava;
extern int lavaCount;

// Player
int playerX, playerY;
int score = 0;
float scoreTimer = 0;
bool gameOver = false;

// Event timers
float eventTimer = 0;
float eventInterval = 2.0f;

// ESC double-tap handling
float lastEscTime = 0.0f;
const float doubleTapThreshold = 0.3f; // max 0.3s between presses
bool escPrompt = false;
float escPromptTimer = 0.0f;           // timer for ESC prompt
const float escPromptDuration = 2.0f;  // show prompt for 2 seconds

/* SETTINGS */
// Load GRID_SIZE from config file
void LoadGridSize() {
  char path[PATH_MAX];

#if defined(_WIN32) || defined(_WIN64)
  snprintf(path, sizeof(path), "settings.txt");
#else
  const char *home = getenv("HOME");
  if (home == NULL) home = ".";
  snprintf(path, sizeof(path), "%s/.config/gridgame/settings.txt", home);
#endif

  FILE *file = fopen(path, "r");
  if (file == NULL) return; // file not found, use default

  char line[256];
  while (fgets(line, sizeof(line), file)) {
    // remove newline characters
    line[strcspn(line, "\r\n")] = 0;

    // check for GRID_SIZE line
    if (strncmp(line, "GRID_SIZE:", 10) == 0) {
      char *value = line + 10;
      while (*value == ' ' || *value == '\t') value++; // skip spaces

      int tmp = atoi(value);
      if (tmp > 0) GRID_SIZE = tmp; // only positive integers
      break;
    }
  }

  fclose(file);
}

/* TEXTURES */

// Load player texture from file or fallback to MINT color
Texture2D LoadPlayerTexture() {
  char path[PATH_MAX];

#if defined(_WIN32) || defined(_WIN64)
  snprintf(path, sizeof(path), "resources/player.png");
#else
  const char *home = getenv("HOME");
  if (home == NULL) home = ".";
  snprintf(path, sizeof(path), "%s/.config/gridgame/resources/player.png", home);
#endif

  Texture2D tex = LoadTexture(path);

  if (tex.id == 0) {
    fprintf(stderr, "Failed to load texture from %s, using MINT color instead.\n", path);

    // fallback: create 1x1 MINT texture
    Image img = GenImageColor(1, 1, (Color){189, 252, 201, 255}); // MINT color
    tex = LoadTextureFromImage(img);
    UnloadImage(img);
  }

  return tex;
}


// Load background texture from file
Texture2D LoadBackgroundTexture() {
  char path[PATH_MAX];

#if defined(_WIN32) || defined(_WIN64)
  snprintf(path, sizeof(path), "resources/background.png");
#else
  const char *home = getenv("HOME");
  if (home == NULL) home = ".";
  snprintf(path, sizeof(path), "%s/.config/gridgame/resources/background.png", home);
#endif

  Texture2D tex = LoadTexture(path);

  if (tex.id == 0) {
#if defined(_WIN32) || defined(_WIN64)
    fprintf(stderr, "Failed to load background texture from %s\n", path);
#else
    fprintf(stderr, "Failed to load background texture from %s, trying ./resources/background.png\n", path);
    tex = LoadTexture("resources/background.png");
#endif
  }

  return tex;
}

/* Reset all events and map */
void ResetMap(int cols, int rows) {
  free(bombs);
  bombs = NULL;
  bombCount = 0;

  free(projectiles);
  projectiles = NULL;
  projectileCount = 0;

  free(lava);
  lava = NULL;
  lavaCount = 0;
}

/* Check if player can move to the new position */
bool CanMoveTo(int newX, int newY, int screenWidth, int screenHeight) {
  if (newX < 0 || newY < 0) return false;

  int maxX = (screenWidth / GRID_SIZE - 1) * GRID_SIZE;
  int maxY = (screenHeight / GRID_SIZE - 1) * GRID_SIZE;

  if (newX > maxX) return false;
  if (newY > maxY) return false;

  return true; // movement allowed
}

/* ENTRY POINT */
int main() {
  srand(time(NULL));

  InitWindow(0, 0, "Grid Game");
  ToggleFullscreen();
  SetTargetFPS(60);
  SetExitKey(KEY_NULL); // disable default ESC exit

  // Calculate number of columns and rows based on grid
  int cols = GetScreenWidth() / GRID_SIZE;
  int rows = GetScreenHeight() / GRID_SIZE;

  // Initialize player position in the center
  playerX = (cols / 2) * GRID_SIZE;
  playerY = (rows / 2) * GRID_SIZE;

  // Load textures
  Texture2D playerTex = LoadPlayerTexture();
  Texture2D bgTex = LoadBackgroundTexture();

  LoadGridSize(); // Set GRID_SIZE from settings

  SpawnProjectiles(300); // allocate projectile slots

  enum {MENU, PLAYING} screen = MENU;

  // Buttons for menu
  Rectangle btnStart = {GetScreenWidth()/2 - 150, GetScreenHeight()/2 - 100, 300, 100};
  Rectangle btnExit  = {GetScreenWidth()/2 - 150, GetScreenHeight()/2 + 20, 300, 100};

  while (!WindowShouldClose()) {
    float dt = GetFrameTime();
    BeginDrawing();
    ClearBackground(BLACK);

    // Draw background first
    DrawTexturePro(bgTex,
                   (Rectangle){0, 0, bgTex.width, bgTex.height},
                   (Rectangle){0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()},
                   (Vector2){0,0}, 0, WHITE);

    if (screen == MENU) {
      DrawText("GRID GAME", GetScreenWidth()/2 - 180, 50, 60, WHITE);

      if (GuiButton(btnStart, "PLAY")) {
        screen = PLAYING;
        score = 0; scoreTimer = 0; eventTimer = 0; eventInterval = 2.0f; gameOver = false;
        playerX = (cols / 2) * GRID_SIZE;
        playerY = (rows / 2) * GRID_SIZE;

        ResetMap(cols, rows); // reset events
        SpawnProjectiles(300);
      }

      if (GuiButton(btnExit, "EXIT")) {
        break; // exit game
      }

    } else if (screen == PLAYING) {
      if (gameOver) {
        DrawText("GAME OVER", GetScreenWidth()/2 - 180, 200, 60, RED);
        DrawText(TextFormat("SCORE: %d", score), GetScreenWidth()/2 - 100, 300, 40, WHITE);
        DrawText("ESC - menu", GetScreenWidth()/2 - 100, 350, 30, GRAY);

        if (IsKeyPressed(KEY_ESCAPE)) screen = MENU;
        EndDrawing();
        continue;
      }

      // Escape handling
      if (IsKeyPressed(KEY_ESCAPE)) {
        float currentTime = GetTime();
        if (currentTime - lastEscTime <= doubleTapThreshold) {
          screen = MENU;      // double-tap ESC = go to menu
          escPrompt = false;  // reset prompt
        } else {
          escPrompt = true;   // single press = show prompt
          escPromptTimer = escPromptDuration; // reset timer
        }
        lastEscTime = currentTime;
      }

      // Update ESC prompt timer
      if (escPrompt) {
        escPromptTimer -= dt;
        if (escPromptTimer <= 0) escPrompt = false;
      }

      // Player movement
      int moveAmount = GRID_SIZE;

      // Up
      if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) {
        int newY = playerY - moveAmount;
        if (CanMoveTo(playerX, newY, GetScreenWidth(), GetScreenHeight())) {
          playerY = newY;
        }
      }

      // Down
      if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) {
        int newY = playerY + moveAmount;
        if (CanMoveTo(playerX, newY, GetScreenWidth(), GetScreenHeight())) {
          playerY = newY;
        }
      }

      // Left
      if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) {
        int newX = playerX - moveAmount;
        if (CanMoveTo(newX, playerY, GetScreenWidth(), GetScreenHeight())) {
          playerX = newX;
        }
      }

      // Right
      if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) {
        int newX = playerX + moveAmount;
        if (CanMoveTo(newX, playerY, GetScreenWidth(), GetScreenHeight())) {
          playerX = newX;
        }
      }

      // Ensure player stays inside screen bounds
      if (playerX < 0) playerX = 0;
      if (playerY < 0) playerY = 0;
      if (playerX > GetScreenWidth() - GRID_SIZE) playerX = GetScreenWidth() - GRID_SIZE;
      if (playerY > GetScreenHeight() - GRID_SIZE) playerY = GetScreenHeight() - GRID_SIZE;

      // Update score
      scoreTimer += dt;
      if (scoreTimer >= 1.0f) {
        score += 50;
        scoreTimer = 0;
        if (score % 250 == 0 && eventInterval > 0.2f) eventInterval -= 0.1f;
      }

      // Event spawning
      eventTimer += dt;
      if (eventTimer >= eventInterval) {
        if (rand() % 2 == 0) SpawnBombs(9);
        else SpawnLava(20);
        eventTimer = 0;
      }

      // Update all events
      UpdateBombs(dt);
      UpdateProjectiles();
      UpdateLava();

      // Draw all events
      DrawBombs();
      DrawProjectiles();
      DrawLava();

      // Draw player
      DrawTexturePro(playerTex,
                     (Rectangle){0,0,playerTex.width,playerTex.height},
                     (Rectangle){playerX,playerY,GRID_SIZE,GRID_SIZE},
                     (Vector2){0,0},0,WHITE);

      // GUI
      GuiLabel((Rectangle){20,20,200,30}, TextFormat("SCORE: %d", score));
      GuiLabel((Rectangle){20,50,300,30}, TextFormat("EVENT INTERVAL: %.1f", eventInterval));

      // ESC prompt
      if (escPrompt) {
        DrawText("Are you sure to exit? If yes, press ESCAPE", 100, GetScreenHeight()-50, 20, RED);
      }
    }

    EndDrawing();
  }

  // Cleanup before exit
  ResetMap(cols, rows);
  UnloadTexture(playerTex);
  UnloadTexture(bgTex);
  CloseWindow();
  return 0;
}
