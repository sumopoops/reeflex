#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#define GRID_WIDTH 6
#define GRID_HEIGHT 5
#define ENEMY_TYPE_COUNT 4

//---------------------------------------------------------------------------------------- STRUCTS

typedef struct Sprite {
	Rectangle rec;
	Vector2 loc;
	bool anim;
	int frames;
	Vector2 frameAjustment;
	bool repeatAnim;
	int id;
	bool exists;
} Sprite;



//---------------------------------------------------------------------------------------- ENUMS

enum gameModes {GAMEMODE_TITLE, GAMEMODE_GAME, GAMEMODE_HELP};



//---------------------------------------------------------------------------------------- GLOBALS

const int screenWidth = 60;
const int screenHeight = 60;
int grid[GRID_HEIGHT][GRID_WIDTH];
Texture2D textures[10];
int types[ENEMY_TYPE_COUNT] = {0, 0, 0, 0};
int remainingTargets = 0;
int currentLevel = 6;



//---------------------------------------------------------------------------------------- FUNCTIONS

void PopulateGrid() {
	int type, x, y;
	while (remainingTargets < currentLevel) {
		y = GetRandomValue(0, GRID_HEIGHT-1);
		x = GetRandomValue(0, GRID_WIDTH-1);
		if (grid[y][x]) continue;
		type = GetRandomValue(1, ENEMY_TYPE_COUNT);
		grid[y][x] = type;
		types[type-1]++;
		remainingTargets++;
	}
}

void AddSprite(Sprite newSprite, Sprite spriteArray[]) {
	
}

void ClearGrid() {
	for (int y=0; y<GRID_HEIGHT; y++)
	for (int x=0; x<GRID_WIDTH; x++)
	grid[y][x] = 0;
	remainingTargets = 0;
	for (int i=0; i<ENEMY_TYPE_COUNT; i++) types[i] = 0;
}

void PrintGrid() {
	for (int y=0; y<GRID_HEIGHT; y++) {
		printf("\n");
		for (int x=0; x<GRID_WIDTH; x++) {
			printf("%d ", grid[y][x]);
		}
	}
	printf("\n\nTYPES: \33[31m");
	for (int i=0; i<ENEMY_TYPE_COUNT; i++) printf("%d ", types[i]);
	printf("\33[0m\n");
}

void DrawIconGrid(Texture2D sheet, int frame) {
	for (int y=0; y<GRID_HEIGHT; y++) {
		for (int x=0; x<GRID_WIDTH; x++) {
			if (grid[y][x])
			DrawTextureRec(sheet, (Rectangle){(grid[y][x]-1)*10, frame*10, 10, 10}, (Vector2){x*10, y*10}, WHITE);
		}
	}
}

Sprite BlankSprite() {
	Sprite newBlankSprite = {0};
	return newBlankSprite;
}

void InitSpriteArray(Sprite spriteArray[]) {
	for (int i=0; i<(sizeof(*spriteArray)/sizeof(Sprite)); i++) {
		spriteArray[i] = BlankSprite();
	}
}



//---------------------------------------------------------------------------------------- MAIN

int main() {

	// Variables
	const float animRate = 0.04;
	unsigned char gameMode = GAMEMODE_TITLE;
	float animTick = 0;
	int animFrame = 0;
	float timeLeft = 56;
	Sprite sprites[100];
	InitSpriteArray(sprites); //TEMP

	// Init Window Stuff
	const char windowed = 15; // Make 0 for Fullscreen
	float scale, playAreaX;
	SetConfigFlags(FLAG_VSYNC_HINT);
	if (windowed) {
		InitWindow(screenWidth*windowed, screenHeight*windowed, "REEFLEX");
		scale = (float)windowed;
		playAreaX = 0;
	} else {
		InitWindow(GetScreenWidth(), GetScreenHeight(), "REEFLEX");
		scale = (float)GetScreenHeight()/screenHeight;
		playAreaX = (float)(GetScreenWidth()-(screenWidth*scale))*0.5;
		ToggleFullscreen();
		HideCursor();
	}

	// Load assets
	Texture2D TX_sprites = LoadTexture("img/sprites.png");

	// Sprites
	Sprite SP_types = {{0, 0, 40, 10}, {10, 18}};
	Sprite SP_logo = {{0, 40, 58, 16}, {1, 16}};
	Sprite SP_hud = {{0, 20, 60, 10}, {0, 50}};
	Sprite SP_targetRem = {{0, 30, 1, 2}, {0, 0}};
	Sprite SP_timeDot = {{1, 30, 1, 1}, {0, 0}};
	Sprite SP_letter_A = {{3, 35, 3, 5}, {0, 0}};
	Sprite SP_letter_S = {{7, 35, 3, 5}, {0, 0}};
	Sprite SP_letter_K = {{19, 35, 3, 5}, {0, 0}};
	Sprite SP_letter_L = {{23, 35, 3, 5}, {0, 0}};

	RenderTexture2D target = LoadRenderTexture(screenWidth, screenHeight);
    SetTargetFPS(60);
	SetWindowPosition(500, 300);

	// Init
	ClearGrid();
	PopulateGrid();
	PrintGrid();

    while (!WindowShouldClose()) {

        // UPDATE
		if (gameMode == GAMEMODE_TITLE) {

			switch (GetKeyPressed()) {
				case KEY_ENTER: case KEY_A: case KEY_S: case KEY_K: case KEY_L:
					gameMode = GAMEMODE_HELP;
			}

		} else if (gameMode == GAMEMODE_GAME) {

			// Animation tick
			animTick += animRate;
			if (animTick > 1) {
				animTick = 0;
				animFrame = !animFrame;
			}

			// Keyboard input
			switch (GetKeyPressed()) {
				case KEY_A:
					printf("A PRESSED\n");
					ClearGrid();
					PopulateGrid();
					break;
				case KEY_S:
					printf("S PRESSED\n");
					break;
				case KEY_K:
					printf("K PRESSED\n");
					break;
				case KEY_L:
					printf("L PRESSED\n");
					break;
			}

			timeLeft -= 0.06;

		} else if (gameMode == GAMEMODE_HELP) {

			switch (GetKeyPressed()) {
				case KEY_ENTER: case KEY_A: case KEY_S: case KEY_K: case KEY_L:
					gameMode = GAMEMODE_GAME;
			}

		}
		

		// TEXTURE DRAW
		BeginTextureMode(target);
		
			// DRAW EVERYTHING HERE
			ClearBackground((Color){33, 33, 33, 255});

			if (gameMode == GAMEMODE_TITLE) {

				DrawTextureRec(TX_sprites, SP_logo.rec, SP_logo.loc, WHITE);

			} else if (gameMode == GAMEMODE_HELP) {

				// Draw enemy types
				DrawTextureRec(TX_sprites, SP_types.rec, SP_types.loc, WHITE);

				// Temp draw letters
				DrawTextureRec(TX_sprites, SP_letter_A.rec, (Vector2){14, 30}, WHITE);
				DrawTextureRec(TX_sprites, SP_letter_S.rec, (Vector2){24, 30}, WHITE);
				DrawTextureRec(TX_sprites, SP_letter_K.rec, (Vector2){34, 30}, WHITE);
				DrawTextureRec(TX_sprites, SP_letter_L.rec, (Vector2){44, 30}, WHITE);

			} else if (gameMode == GAMEMODE_GAME) {

				DrawIconGrid(TX_sprites, animFrame);
				DrawTextureRec(TX_sprites, SP_hud.rec, SP_hud.loc, WHITE);

				// Draw target tracker
				for (int i=0; i<remainingTargets; i++)
				DrawTextureRec(TX_sprites, SP_targetRem.rec, (Vector2){1+(i*2), 53}, WHITE);

				// Draw time bar
				for (int i=0; i<timeLeft; i++)
				DrawTextureRec(TX_sprites, SP_timeDot.rec, (Vector2){2+i, 57}, WHITE);

			}
			
		
		EndTextureMode();

		// DRAW
        BeginDrawing();
		ClearBackground(BLACK);
		DrawTexturePro(target.texture, (Rectangle){0, 0, screenWidth, -screenHeight}, (Rectangle){playAreaX, 0, screenWidth*scale, screenHeight*scale}, (Vector2){0, 0}, 0, WHITE);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}