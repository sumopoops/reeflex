#include "raylib.h"
#include <stdio.h>

//---------------------------------------------------------------------------------------- STRUCTS

typedef struct Sprite {
	Texture2D* tx;
	Rectangle rec;
	Vector2 loc;
} Sprite;



//---------------------------------------------------------------------------------------- ENUMS

enum gameModes {GAMEMODE_TITLE, GAMEMODE_GAME};



//---------------------------------------------------------------------------------------- GLOBALS

const int screenWidth = 60;
const int screenHeight = 60;
int grid[6][5];
Texture2D textures[10];
int types[6] = {0, 0, 0, 0, 0};
int remainingTargets = 0;
int currentLevel = 10;



//---------------------------------------------------------------------------------------- FUNCTIONS

void PopulateGrid() {
	int type, x, y;
	while (remainingTargets < currentLevel) {
		x = GetRandomValue(0, 6);
		y = GetRandomValue(0, 5);
		if (grid[y][x]) {
			continue;
		}
		type = GetRandomValue(1, 5);
		grid[x][y] = type;
		types[type-1]++;
		remainingTargets++;
	}
}

void ClearGrid() {
	for (int y=0; y<5; y++)
	for (int x=0; x<6; x++)
	grid[x][y] = 0;
	remainingTargets = 0;
}

void PrintGrid() {
	for (int y=0; y<5; y++) {
		printf("\n");
		for (int x=0; x<6; x++) {
			printf("%d, ", grid[y][x]);
		}
	}
	printf("\n\n");
}

void DrawIconGrid(Texture2D sheet, int frame) {
	for (int y=0; y<6; y++) {
		for (int x=0; x<5; x++) {
			if (grid[y][x])
			DrawTextureRec(sheet, (Rectangle){grid[y][x]*10, frame*10, 10, 10}, (Vector2){y*10, x*10}, WHITE);
		}
	}
}



//---------------------------------------------------------------------------------------- MAIN

int main() {

	// Variables
	float animTick = 0;
	const float animRate = 0.04;
	int animFrame = 0;
	unsigned char gameMode = GAMEMODE_GAME;
	float timeLeft = 56;

	// Init Window Stuff
	const char windowed = 0; // Make 0 for Fullscreen
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
	Texture2D TX_logo = LoadTexture("img/logo.png");

	// Sprites
	Sprite SP_hud = {&TX_sprites, {0, 20, 60, 10}, {0, 50}};
	Sprite SP_dot1 = {&TX_sprites, {0, 30, 1, 2}, {0, 0}};
	Sprite SP_timeDot = {&TX_sprites, {2, 30, 1, 1}, {0, 0}};
	Sprite SP_letter_A = {&TX_sprites, {5, 30, 3, 5}, {0, 0}};
	Sprite SP_letter_S = {&TX_sprites, {10, 30, 3, 5}, {0, 0}};
	Sprite SP_letter_D = {&TX_sprites, {15, 30, 3, 5}, {0, 0}};
	Sprite SP_letter_J = {&TX_sprites, {15, 30, 3, 5}, {0, 0}};
	Sprite SP_letter_K = {&TX_sprites, {15, 30, 3, 5}, {0, 0}};
	Sprite SP_letter_L = {&TX_sprites, {15, 30, 3, 5}, {0, 0}};

	RenderTexture2D target = LoadRenderTexture(screenWidth, screenHeight);
    SetTargetFPS(60);

	// Init
	ClearGrid();
	PopulateGrid();
	PrintGrid();

    while (!WindowShouldClose()) {

        // UPDATE
		if (gameMode == GAMEMODE_GAME) {

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
				case KEY_D:
					printf("D PRESSED\n");
					break;
				case KEY_J:
					printf("J PRESSED\n");
					break;
				case KEY_K:
					printf("K PRESSED\n");
					break;
				case KEY_L:
					printf("L PRESSED\n");
					break;
			}

			timeLeft -= 0.06;
			printf("TYPES: %d, %d, %d, %d, %d, %d\n", types[0], types[1], types[2], types[3], types[4], types[5]); //TEMP

		}
		

		// TEXTURE DRAW
		BeginTextureMode(target);
		
			// DRAW EVERYTHING HERE
			ClearBackground((Color){33, 33, 33, 255});

			if (gameMode == GAMEMODE_TITLE) {

				DrawTexture(TX_logo, 0, 0, WHITE);

			} else if (gameMode == GAMEMODE_GAME) {

				DrawIconGrid(TX_sprites, animFrame);
				DrawTextureRec(*SP_hud.tx, SP_hud.rec, SP_hud.loc, WHITE);

				// Draw target tracker
				for (int i=0; i<remainingTargets; i++)
				DrawTextureRec(*SP_dot1.tx, SP_dot1.rec, (Vector2){1+(i*2), 53}, WHITE);

				// Temp draw letters
				DrawTextureRec(*SP_letter_A.tx, SP_letter_A.rec, (Vector2){0, 0}, WHITE);

				// Draw time bar
				for (int i=0; i<timeLeft; i++)
				DrawTextureRec(*SP_timeDot.tx, SP_timeDot.rec, (Vector2){2+i, 57}, WHITE);

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