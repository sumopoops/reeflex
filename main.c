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
	int currentFrame;
	Vector2 frameAjustment;
	bool repeatAnim;
	bool exists;
	float animTick;
	float animSpeed;
} Sprite;

typedef struct Circle {
	Vector2 loc;
	int rad;
	int speed;
} Circle;



//---------------------------------------------------------------------------------------- ENUMS

enum gameModes {
	GAMEMODE_TITLE,
	GAMEMODE_GAME,
	GAMEMODE_HELP,
	GAMEMODE_GAMEOVER
};



//---------------------------------------------------------------------------------------- GLOBALS

const int screenWidth = 60;
const int screenHeight = 60;
int grid[GRID_HEIGHT][GRID_WIDTH];
Texture2D textures[10];
int types[ENEMY_TYPE_COUNT] = {0, 0, 0, 0};
int remainingTargets = 0;
int currentLevel = 1;
int mode = 1;
float timeLeft = 56;
int lives = 3;
int shakeCount = 0;



//---------------------------------------------------------------------------------------- FUNCTIONS

void ResetLevel() {

	// Clear grid
	for (int y=0; y<GRID_HEIGHT; y++) {
		for (int x=0; x<GRID_WIDTH; x++) grid[y][x] = 0;
	}
	remainingTargets = 0;
	for (int i=0; i<ENEMY_TYPE_COUNT; i++) types[i] = 0;

	// Populate grid
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

	// Reset timer
	timeLeft = 56;
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

void DrawIconGrid(Texture2D sheet, int frame, Vector2 shakeVector) {
	for (int y=0; y<GRID_HEIGHT; y++) {
		for (int x=0; x<GRID_WIDTH; x++) {
			if (grid[y][x])
			DrawTextureRec(sheet, (Rectangle){(grid[y][x]-1)*10, frame*10, 10, 10}, (Vector2){(x*10)+shakeVector.x, (y*10)+shakeVector.y}, WHITE);
		}
	}
}

Sprite BlankSprite() {
	Sprite newBlankSprite = {0};
	return newBlankSprite;
}

Sprite NewSprite(Rectangle rec, Vector2 loc, bool anim, int frames, Vector2 frameAjustment, bool repeatFrames, float animSpeed) {
	Sprite newSprite;
	newSprite.exists = true;
	newSprite.rec = rec;
	newSprite.loc = loc;
	newSprite.anim = anim;
	newSprite.frames = frames;
	newSprite.frameAjustment = frameAjustment;
	newSprite.repeatAnim = repeatFrames;
	newSprite.animTick = 0;
	newSprite.animSpeed = animSpeed;
	newSprite.currentFrame = 0;
	return newSprite;
}

void InitSpriteArray(Sprite spriteArray[]) {
	for (int i=0; i<(sizeof(*spriteArray)/sizeof(Sprite)); i++) {
		spriteArray[i] = BlankSprite();
	}
}

bool AttackEnemy(int type) {
	if (types[type] > 0) {
		types[type]--;
		remainingTargets--;
		// Find enemy of that type and change sprite
		for (int y=0; y<GRID_HEIGHT; y++) {
			for (int x=0; x<GRID_WIDTH; x++) {
				if (grid[y][x] != type+1) continue;
				grid[y][x] = 5;
				return true;
			}
		}
		shakeCount = 3;
		return true;
	} else {
		return false;
	}
}

Circle NewCircle() {
	Circle newCirc;
	newCirc.loc.x = GetRandomValue(-10, 70);
	newCirc.loc.y = 80;
	newCirc.rad = GetRandomValue(3, 20);
	newCirc.speed = GetRandomValue(1, 3);
	return newCirc;
}

void UpdateSprites(Sprite *spritesArray) {
	Sprite *s = spritesArray;
	for (int i=0; i<sizeof(*s)/sizeof(Sprite); i++) {
		if (!s[i].exists) continue;
		s[i].animTick += s[i].animSpeed;
		if (s[i].animTick > 1) {
			s[i].animTick = 0;
			s[i].currentFrame++;
			if (s[i].currentFrame > s[i].frames-1) s[i].currentFrame = 0;
		}
	}
}

void DrawSprites(Sprite *spriteArray, Texture2D spriteSheet) {
	for (int i=0; i<sizeof(*spriteArray)/sizeof(Sprite); i++) {
		if (spriteArray[i].exists) {
			Rectangle frameRec;
			frameRec.x = spriteArray[i].rec.x + spriteArray[i].frameAjustment.x* spriteArray[i].currentFrame;
			frameRec.y = spriteArray[i].rec.y + spriteArray[i].frameAjustment.y * spriteArray[i].currentFrame;
			frameRec.width = spriteArray[i].rec.width;
			frameRec.height = spriteArray[i].rec.height;
			DrawTextureRec(spriteSheet, frameRec, spriteArray[i].loc, WHITE);
		}
	}
}



//---------------------------------------------------------------------------------------- MAIN

int main() {

	// Variables
	const char windowed = 15; // Make 0 for Fullscreen
	const float enemyAnimRate = 0.04;
	unsigned char gameMode = GAMEMODE_TITLE;
	float enemyAnimTick = 0;
	int animFrame = 0;
	Sprite *sprites = malloc(sizeof(Sprite)*30);
	InitSpriteArray(sprites);
	sprites[0] = NewSprite((Rectangle){60, 20, 27, 5}, (Vector2){16, 47}, true, 8, (Vector2){0, 5}, true, 0.18);
	const Color COL_WHITE = {238, 238, 238, 255};
	const Color COL_BLACK = {33, 33, 33, 255};
	Circle circles[10] = {0};
	int circleArrLength = sizeof(circles)/sizeof(Circle);
	for (int i=0; i<circleArrLength; i++) circles[i] = NewCircle();
	bool controlsEnabled = true;

	// Shake shake
	Vector2 shakeVector = {0, 0};
	float shakeTick = 0;
	float shakeRate = 0.5;

	// Init Window Stuff
	float scale, playAreaX;
	SetConfigFlags(FLAG_VSYNC_HINT);
	if (windowed) {
		InitWindow(screenWidth*windowed, screenHeight*windowed, "REEFLX");
		scale = (float)windowed;
		playAreaX = 0;
	} else {
		InitWindow(GetScreenWidth(), GetScreenHeight(), "REEFLX");
		scale = (float)GetScreenHeight()/screenHeight;
		playAreaX = (float)(GetScreenWidth()-(screenWidth*scale))*0.5;
		ToggleFullscreen();
		HideCursor();
	}

	// Audio
	InitAudioDevice();

	// Load assets
	Texture2D TX_sprites = LoadTexture("img/sprites.png");
	Sound SND_bleep = LoadSound("snd/bleep.ogg");
	Sound SND_looseLife = LoadSound("snd/bleep.ogg");
	Sound SND_gameover = LoadSound("snd/gameover.ogg");

	// Sprites
	Sprite SP_types = {{0, 0, 40, 10}, {10, 18}};
	Sprite SP_logo = {{0, 40, 41, 27}, {9, 10}};
	Sprite SP_hud = {{0, 20, 60, 10}, {0, 50}};
	Sprite SP_targetRem = {{0, 30, 1, 2}, {0, 0}};
	Sprite SP_timeDot = {{1, 30, 1, 1}, {0, 0}};
	Sprite SP_letter_A = {{3, 35, 3, 5}, {0, 0}};
	Sprite SP_letter_S = {{7, 35, 3, 5}, {0, 0}};
	Sprite SP_letter_K = {{19, 35, 3, 5}, {0, 0}};
	Sprite SP_letter_L = {{23, 35, 3, 5}, {0, 0}};
	Sprite SP_gameover = {{0, 67, 39, 17}, {10, 21}};

	RenderTexture2D target = LoadRenderTexture(screenWidth, screenHeight);
    SetTargetFPS(60);
	//SetWindowPosition(500, 300); //TEMP

	// Init
	ResetLevel();

    while (!WindowShouldClose()) {

        // UPDATE
		printf("TEST\n");
		if (gameMode == GAMEMODE_TITLE) {

			if (controlsEnabled) {
				switch (GetKeyPressed()) {
					case KEY_ENTER: case KEY_A: case KEY_S: case KEY_K: case KEY_L:
						InitSpriteArray(sprites);
						gameMode = GAMEMODE_HELP;
				}
			}
			for (int i=0; i<circleArrLength; i++) {
				circles[i].loc.y -= circles[i].speed;
				if (circles[i].loc.y < -20) {
					circles[i] = NewCircle();
				}
			}

		} else if (gameMode == GAMEMODE_GAME) {

			// Animation tick
			enemyAnimTick += enemyAnimRate;
			if (enemyAnimTick > 1) {
				enemyAnimTick = 0;
				animFrame = !animFrame;
			}

			// Shake tick
			shakeTick += shakeRate;
			if (shakeTick > 1) {
				if (shakeCount > 0) {
					shakeVector.x = GetRandomValue(-2, 2);
					shakeVector.y = GetRandomValue(-2, 2);
					shakeTick = 0;
					shakeCount--;
				} else {
					shakeVector = (Vector2){0, 0};
				}
				
			}

			// Keyboard input
			if (controlsEnabled) {
				int enemyTypeKey = 4;
				switch (GetKeyPressed()) {
					case KEY_A: enemyTypeKey = 0; break;
					case KEY_S: enemyTypeKey = 1; break;
					case KEY_K: enemyTypeKey = 2; break;
					case KEY_L: enemyTypeKey = 3; break;
				}
				if (enemyTypeKey != 4) {
					if (AttackEnemy(enemyTypeKey)) {
						PlaySound(SND_bleep);
					} else {
						lives--;
						PlaySound(SND_looseLife);
						if (!lives) {
							PlaySound(SND_gameover);
							gameMode = GAMEMODE_GAMEOVER;
						}
					}
				}
			}

			timeLeft -= 0.06;
			if (timeLeft <= 0) gameMode = GAMEMODE_GAMEOVER;

			// Check for all enemies being dead
			if (!types[0] && !types[1] && !types[2] && !types[3]) {
				currentLevel++;
				ResetLevel();
			}

		} else if (gameMode == GAMEMODE_HELP) {

			if (controlsEnabled) {
				switch (GetKeyPressed()) {
					case KEY_ENTER: case KEY_A: case KEY_S: case KEY_K: case KEY_L:
						ResetLevel();
						gameMode = GAMEMODE_GAME;
				}
			}

		} else if (gameMode == GAMEMODE_GAMEOVER) {

			if (controlsEnabled) {
				switch (GetKeyPressed()) {
					case KEY_ENTER: case KEY_A: case KEY_S: case KEY_K: case KEY_L:
						currentLevel = 1;
						gameMode = GAMEMODE_HELP;
						lives = 3;
				}
			}

		}

		// Always run
		UpdateSprites(sprites);
		

		// TEXTURE DRAW
		BeginTextureMode(target);
		
			// DRAW EVERYTHING HERE
			ClearBackground((Color){33, 33, 33, 255});

			if (gameMode == GAMEMODE_TITLE) {

				for (int i=0; i<circleArrLength; i++) {
					DrawCircle(circles[i].loc.x, circles[i].loc.y, circles[i].rad, COL_WHITE);
				}
				DrawTextureRec(TX_sprites, SP_logo.rec, SP_logo.loc, WHITE);
				DrawRectangle(0, 46, 60, 7, COL_BLACK);
				//DrawTextureRec(TX_sprites, SP_press_a.rec, SP_press_a.loc, WHITE);

			} else if (gameMode == GAMEMODE_HELP) {

				// Draw enemy types
				DrawTextureRec(TX_sprites, SP_types.rec, SP_types.loc, WHITE);

				// Temp draw letters
				DrawTextureRec(TX_sprites, SP_letter_A.rec, (Vector2){14, 30}, WHITE);
				DrawTextureRec(TX_sprites, SP_letter_S.rec, (Vector2){24, 30}, WHITE);
				DrawTextureRec(TX_sprites, SP_letter_K.rec, (Vector2){34, 30}, WHITE);
				DrawTextureRec(TX_sprites, SP_letter_L.rec, (Vector2){44, 30}, WHITE);

			} else if (gameMode == GAMEMODE_GAME) {

				DrawIconGrid(TX_sprites, animFrame, shakeVector);
				DrawTextureRec(TX_sprites, SP_hud.rec, SP_hud.loc, WHITE);

				// Draw target tracker
				for (int i=0; i<remainingTargets; i++)
				DrawTextureRec(TX_sprites, SP_targetRem.rec, (Vector2){1+(i*2), 53}, WHITE);

				// Draw time bar
				for (int i=0; i<timeLeft; i++)
				DrawTextureRec(TX_sprites, SP_timeDot.rec, (Vector2){2+i, 57}, WHITE);

			} else if (gameMode == GAMEMODE_GAMEOVER) {

				DrawTextureRec(TX_sprites, SP_gameover.rec, SP_gameover.loc, WHITE);

			}
			
			// Always draw
			DrawSprites(sprites, TX_sprites);
		
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