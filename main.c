#include "raylib.h"

//---------------------------------------------------------------------------------------- STRUCTS





//---------------------------------------------------------------------------------------- GLOBALS

const int screenWidth = 60;
const int screenHeight = 60;
int grid[10][10];
Texture2D textures[10];



//---------------------------------------------------------------------------------------- FUNCTIONS

void TempPopulateGrid() {
	for (int a=0; a<10; a++) {
		for (int b=0; b<10; b++) {
			grid[a][b] = 1;
		}
	}
	grid[1][2] = 1;
}

void DrawIconGrid() {
	for (int a=0; a<10; a++)
	for (int b=0; b<10; b++) {
		//if (!grid[a][b]) break;
		DrawTexture(textures[grid[b][a]], a*10, b*10, WHITE);
		printf("Grid Value: %d\n\n", grid[a][b]);
	}
}



//---------------------------------------------------------------------------------------- MAIN

int main() {

	// Init Window Stuff
	const char windowed = 0; // Make 0 for Fullscreen
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

	RenderTexture2D target = LoadRenderTexture(screenWidth, screenHeight);
    SetTargetFPS(60);

    while (!WindowShouldClose()) {

        // UPDATE

		// TEXTURE DRAW
		BeginTextureMode(target);
		
			// DRAW EVERYTHING HERE
			ClearBackground((Color){33, 33, 33, 255});
			DrawIconGrid();
		
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