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
int types[5] = {0, 0, 0, 0, 0};
int remainingTargets = 0;
int currentLevel = 5;



//---------------------------------------------------------------------------------------- FUNCTIONS

void PopulateGrid() {
	int type, x, y;
	while (remainingTargets < currentLevel) {
		x = GetRandomValue(0, 6);
		y = GetRandomValue(0, 5);
		if (grid[x][y]) continue;
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
	for (int a=0; a<6; a++) {
		for (int b=0; b<5; b++)
		DrawTextureRec(sheet, (Rectangle){grid[a][b]*10, frame*10, 10, 10}, (Vector2){a*10, b*10}, WHITE);
	}
}



//---------------------------------------------------------------------------------------- MAIN

int main() {

	// Variables
	float animTick = 0;
	const float animRate = 0.04;
	int animFrame = 0;
	//int targets = 6;
	unsigned char gameMode = GAMEMODE_GAME;
	float timeLeft = 56;

	// Levels
	//float levels[] = {0.06, 0.09, 0.12, 0.15};

	// Init Window Stuff
	const char windowed = 13; // Make 0 for Fullscreen
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
	//Sprite SP_dot2 = {&TX_sprites, {2, 30, 1, 2}, {0, 0}};
	Sprite SP_timeDot = {&TX_sprites, {2, 30, 1, 1}, {0, 0}};

	RenderTexture2D target = LoadRenderTexture(screenWidth, screenHeight);
    SetTargetFPS(60);

	// Init
	ClearGrid();
	PopulateGrid();
	PrintGrid();

    while (!WindowShouldClose()) {

        // UPDATE
		if (gameMode == GAMEMODE_GAME) {

			animTick += animRate;
			if (animTick > 1) {
				animTick = 0;
				animFrame = !animFrame;
			}

			timeLeft -= 0.06;
			//printf("TYPES: %d, %d, %d, %d, %d\n", types[0], types[1], types[2], types[3], types[4]); //TEMP

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