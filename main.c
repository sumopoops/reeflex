#include "raylib.h"
#include <stdio.h>

//---------------------------------------------------------------------------------------- STRUCTS





//---------------------------------------------------------------------------------------- GLOBALS

const int screenWidth = 60;
const int screenHeight = 60;
int grid[6][5];
Texture2D textures[10];



//---------------------------------------------------------------------------------------- FUNCTIONS

void TempPopulateGrid() {
	for (int a=0; a<6; a++) {
		for (int b=0; b<5; b++) {
			grid[a][b] = 1;
		}
	}
}

void DrawIconGrid() {
	for (int a=0; a<6; a++) {
		for (int b=0; b<5; b++) {
			//if (!grid[a][b]) break;
			DrawTexture(textures[grid[a][b]], a*10, b*10, WHITE);
			//printf("Grid Value: %d\n\n", grid[a][b]);
		}
	}
}



//---------------------------------------------------------------------------------------- MAIN

int main() {

	// Init Window Stuff
	const char windowed = 10; // Make 0 for Fullscreen
	float scale;
	float playAreaX;
	SetConfigFlags(FLAG_VSYNC_HINT);
	if (windowed) {
		InitWindow(screenWidth*windowed, screenHeight*windowed, "Invade");
		scale = (float)windowed;
		playAreaX = 0;
	} else {
		InitWindow(GetScreenWidth(), GetScreenHeight(), "Invade");
		scale = (float)GetScreenHeight()/screenHeight;
		playAreaX = (float)(GetScreenWidth()-(screenWidth*scale))*0.5;
		ToggleFullscreen();
	}
	HideCursor();

	// Load assets
	textures[1] = LoadTexture("img/1.png");
	Texture2D TX_sprites = LoadTexture("img/sprites.png");
	Texture2D TX_logo = LoadTexture("img/logo.png");
	Texture2D TX_hud = LoadTexture("img/hud.png");

	RenderTexture2D target = LoadRenderTexture(screenWidth, screenHeight);
    SetTargetFPS(60);

	// Init
	TempPopulateGrid();

    while (!WindowShouldClose()) {

        // UPDATE

		// TEXTURE DRAW
		BeginTextureMode(target);
		
			// DRAW EVERYTHING HERE
			ClearBackground((Color){33, 33, 33, 255});
			DrawIconGrid();
			DrawTexture(TX_hud, 0, 50, WHITE);
		
		EndTextureMode();

		// DRAW
        BeginDrawing();

			ClearBackground(BLACK);
			
			// Draw the RenderTexture scaled
			DrawTexturePro(target.texture, (Rectangle){0, 0, screenWidth, -screenHeight}, (Rectangle){playAreaX, 0, screenWidth*scale, screenHeight*scale}, (Vector2){0, 0}, 0, WHITE);
			

        EndDrawing();
    }

    CloseWindow();

    return 0;
}