#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#define GRID_WIDTH 6
#define GRID_HEIGHT 5
#define ENEMY_TYPE_COUNT 4
#define STARTING_LEVEL 3

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
	int eventOnFinish;
} Sprite;

typedef struct Circle {
	Vector2 loc;
	int rad;
	int speed;
} Circle;

typedef struct Timer {
	float tick;
	float speed;
	int frame;
	int maxFrames;
} Timer;



//---------------------------------------------------------------------------------------- ENUMS

enum gameModes {
	GAMEMODE_TITLE,
	GAMEMODE_GAME,
	GAMEMODE_HELP,
	GAMEMODE_GAMEOVER
};

enum events {
	EVENT_EMPTY,
	EVENT_ENABLE_CONTROLS,
	EVENT_GAMEOVER_ANIM,
	EVENT_GAMEOVER
};



//---------------------------------------------------------------------------------------- GLOBALS

const int screenWidth = 60;
const int screenHeight = 60;
int grid[GRID_HEIGHT][GRID_WIDTH];
int types[ENEMY_TYPE_COUNT] = {0, 0, 0, 0};
int remainingTargets = 0;
int currentLevel = STARTING_LEVEL;
int mode = 1;
float timeLeft = 56;
int lives = 3;
int shakeCount = 0;
Sprite sprites[30];
int eventQueue = EVENT_EMPTY;
bool controlsEnabled = true;
unsigned char gameMode = GAMEMODE_TITLE;
Sound SND_gameover;
double gameTime;
int score;
int world = 1;



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

Sprite NewSprite(Rectangle rec, Vector2 loc, int frames, Vector2 frameAjustment, bool repeatFrames, float animSpeed, int eventOnFinish) {
	Sprite newSprite;
	newSprite.exists = true;
	newSprite.rec = rec;
	newSprite.loc = loc;
	newSprite.frames = frames;
	newSprite.frameAjustment = frameAjustment;
	newSprite.repeatAnim = repeatFrames;
	newSprite.animTick = 0;
	newSprite.animSpeed = animSpeed;
	newSprite.currentFrame = 0;
	newSprite.eventOnFinish = eventOnFinish;
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

void UpdateSprites() {
	for (int i=0; i<30; i++) {
		if (!sprites[i].exists) continue;
		sprites[i].animTick += sprites[i].animSpeed;
		if (sprites[i].animTick > 1) {
			sprites[i].animTick = 0;
			sprites[i].currentFrame++;
			if (sprites[i].currentFrame > sprites[i].frames-1) {
				if (sprites[i].repeatAnim) {
					sprites[i].currentFrame = 0;
				} else {
					if (sprites[i].eventOnFinish) eventQueue = sprites[i].eventOnFinish;
					sprites[i].exists = false;
				}
			}
		}
	}
}

void DrawSprites(Texture2D spriteSheet) {
	for (int i=0; i<30; i++) {
		if (sprites[i].exists) {
			Rectangle frameRec;
			frameRec.x = sprites[i].rec.x + sprites[i].frameAjustment.x* sprites[i].currentFrame;
			frameRec.y = sprites[i].rec.y + sprites[i].frameAjustment.y * sprites[i].currentFrame;
			frameRec.width = sprites[i].rec.width;
			frameRec.height = sprites[i].rec.height;
			DrawTextureRec(spriteSheet, frameRec, sprites[i].loc, WHITE);
		}
	}
}

void ExecuteEventQueue() {
	switch (eventQueue) {

		case EVENT_ENABLE_CONTROLS:
			controlsEnabled = true;
			eventQueue = EVENT_EMPTY;
			break;

		case EVENT_GAMEOVER_ANIM:
			PlaySound(SND_gameover);
			sprites[7] = NewSprite((Rectangle){0, 964, 60, 60}, (Vector2){0, 0}, 14, (Vector2){60, 0}, false, 0.3, EVENT_GAMEOVER);
			gameMode = GAMEMODE_GAMEOVER;
			score = (int)(((currentLevel * 100) * world) - (((GetTime() - gameTime) / currentLevel) * 10)); //TEMP
			if (score < 0) score = 0; //TEMP
			if (currentLevel == STARTING_LEVEL && world == 1) score = 0; //TEMP
			eventQueue = EVENT_EMPTY;
			break;

		case EVENT_GAMEOVER:
			controlsEnabled = true;
			printf("SCORE: \e[34m%d\e[0m\n", score);
			eventQueue = EVENT_EMPTY;
			break;
			
	}
}



//---------------------------------------------------------------------------------------- MAIN

int main() {

	// Variables
	const char windowed = 15; // Make 0 for Fullscreen
	const float enemyAnimRate = 0.04;
	float enemyAnimTick = 0;
	int animFrame = 0;
	InitSpriteArray(sprites);
	sprites[0] = NewSprite((Rectangle){60, 20, 27, 5}, (Vector2){16, 47}, 8, (Vector2){0, 5}, true, 0.18, EVENT_EMPTY);
	const Color COL_WHITE = {238, 238, 238, 255};
	const Color COL_BLACK = {33, 33, 33, 255};
	Circle circles[10] = {0};
	int circleArrLength = sizeof(circles)/sizeof(Circle);
	for (int i=0; i<circleArrLength; i++) circles[i] = NewCircle();
	float scoreScrollY = 40;

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
	Sound SND_looseLife = LoadSound("snd/lifeloss.ogg");
	SND_gameover = LoadSound("snd/gameover.ogg");
	Font font = LoadFont("img/font.png");

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
	Sprite SP_gameover = {{0, 67, 60, 60}, {0, 0}};

	RenderTexture2D target = LoadRenderTexture(screenWidth, screenHeight);
    SetTargetFPS(60);
	//SetWindowPosition(500, 300); //TEMP

	// Init
	ResetLevel();

    while (!WindowShouldClose()) {

        // UPDATE
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
					shakeVector.x = GetRandomValue(-3, 3);
					shakeVector.y = GetRandomValue(-3, 3);
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
						
						// Enemy attack sucsess
						PlaySound(SND_bleep);

					} else {

						// Enemy attack failed
						lives--;
						controlsEnabled = false;
						PlaySound(SND_looseLife);
						shakeCount = 5;

						// Remove life and play heart animation
						switch (lives) {
							case 2:
								sprites[0] = NewSprite((Rectangle){88, 13, 41, 14}, (Vector2){9, 18}, 1, (Vector2){0, 0}, false, 0.014, EVENT_ENABLE_CONTROLS);
								sprites[1] = NewSprite((Rectangle){80, 0, 13, 12}, (Vector2){10, 19}, 8, (Vector2){13, 0}, false, 0.2, EVENT_EMPTY);
								sprites[2] = NewSprite((Rectangle){80, 0, 13, 12}, (Vector2){23, 19}, 1, (Vector2){13, 0}, false, 0.014, EVENT_EMPTY);
								sprites[3] = NewSprite((Rectangle){80, 0, 13, 12}, (Vector2){36, 19}, 1, (Vector2){13, 0}, false, 0.014, EVENT_EMPTY);
								break;
							case 1:
								sprites[0] = NewSprite((Rectangle){88, 13, 41, 14}, (Vector2){9, 18}, 1, (Vector2){0, 0}, false, 0.014, EVENT_ENABLE_CONTROLS);
								sprites[2] = NewSprite((Rectangle){80, 0, 13, 12}, (Vector2){23, 19}, 8, (Vector2){13, 0}, false, 0.2, EVENT_EMPTY);
								sprites[3] = NewSprite((Rectangle){80, 0, 13, 12}, (Vector2){36, 19}, 1, (Vector2){13, 0}, false, 0.014, EVENT_EMPTY);
								break;
							case 0:
								scoreScrollY = 50;
								sprites[0] = NewSprite((Rectangle){88, 13, 41, 14}, (Vector2){9, 18}, 1, (Vector2){0, 0}, false, 0.014, EVENT_GAMEOVER_ANIM);
								sprites[3] = NewSprite((Rectangle){80, 0, 13, 12}, (Vector2){36, 19}, 8, (Vector2){13, 0}, false, 0.2, EVENT_EMPTY);
								break;
						}
					}
				}
			}

			// Timer runs out
			if (controlsEnabled) timeLeft -= 0.06;
			if (timeLeft <= 0) {
				scoreScrollY = 50;
				eventQueue = EVENT_GAMEOVER_ANIM;
			}

			// Check for all enemies being dead
			if (!types[0] && !types[1] && !types[2] && !types[3]) {
				currentLevel += 3;
				if (currentLevel > 30) {

					// World completed
					currentLevel = STARTING_LEVEL;
					world++;

				}
				printf("LEVEL: \e[31m%d\e[0m\n", currentLevel); //TEMP
				ResetLevel();
			}

		} else if (gameMode == GAMEMODE_HELP) {

			if (controlsEnabled) {
				switch (GetKeyPressed()) {
					case KEY_ENTER: case KEY_A: case KEY_S: case KEY_K: case KEY_L:

						// Reset gameplay
						world = 1;
						ResetLevel();
						gameMode = GAMEMODE_GAME;
						gameTime = GetTime();
				}
			}

		} else if (gameMode == GAMEMODE_GAMEOVER) {

			if (controlsEnabled) {
				switch (GetKeyPressed()) {
					case KEY_ENTER: case KEY_A: case KEY_S: case KEY_K: case KEY_L:
						currentLevel = STARTING_LEVEL;
						gameMode = GAMEMODE_HELP;
						StopSound(SND_gameover);
						lives = 3;
						gameTime = GetTime();
				}
			}

			if (scoreScrollY > 0) scoreScrollY -= 0.4;

		}

		// Always run
		ExecuteEventQueue();
		UpdateSprites(sprites);

		// TEXTURE DRAW
		BeginTextureMode(target);
		
			// DRAW EVERYTHING HERE
			ClearBackground(COL_BLACK);

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
				DrawTextEx(font, "SCORE", (Vector2){15, 40+scoreScrollY}, font.baseSize, 1, COL_WHITE);
				Vector2 scoreWidth = MeasureTextEx(font, TextFormat("%i", score), font.baseSize, 1);
				DrawTextEx(font, TextFormat("%i", score), (Vector2){(int)(60 - scoreWidth.x)/2, 46+scoreScrollY}, font.baseSize, 1, COL_WHITE);

			}
			
			// Always draw
			DrawSprites(TX_sprites);
		
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