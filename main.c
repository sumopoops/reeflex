#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#define PIXEL_SIZE 15
#define GRID_WIDTH 6
#define GRID_HEIGHT 5
#define ENEMY_TYPE_COUNT 4
#define STARTING_LEVEL 6
#define STARTING_WORLD 1
#define INVIS_SWITCH_START 0.2
#define SPRITE_ARRAY_SIZE 30



//---------------------------------------------------------------------------------------- STRUCTS

typedef struct Sprite {
	Rectangle rec;
	Vector2 loc;
	bool anim;
	int frames;
	int currentFrame;
	bool repeatAnim;
	bool exists;
	float animTick;
	float animSpeed;
	void (*eventOnFinish)();
	float holdLast;
} Sprite;

typedef struct Circle {
	Vector2 loc;
	int rad;
	int speed;
} Circle;



//---------------------------------------------------------------------------------------- ENUMS

enum gameModes {
	GAMEMODE_LOADING,
	GAMEMODE_TITLE,
	GAMEMODE_GAME,
	GAMEMODE_HELP,
	GAMEMODE_GAMEOVER,
	GAMEMODE_WIN
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
Sprite *sprites;
int eventQueue = false;
bool controlsEnabled = true;
unsigned char gameMode = GAMEMODE_TITLE;
Sound SND_gameover;
int world = STARTING_WORLD;
float world2Tick = 0;
bool lightsOn = true;
float invisTick = 0;
float invisSwitch = INVIS_SWITCH_START;



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
	invisSwitch = INVIS_SWITCH_START;

}

void MoveEnemies(bool printResult) {
	for (int y=0; y<GRID_HEIGHT; y++) {
		for (int x=0; x<GRID_WIDTH; x++) {
			if (grid[y][x]) {
				int moveDirY = GetRandomValue(-1, 1);
				int moveDirX = GetRandomValue(-1, 1);
				if (!moveDirX && !moveDirY) continue;
				if (moveDirY+y < 0 || moveDirY+y > (GRID_HEIGHT-1) || moveDirX+x < 0 || moveDirX+x > (GRID_WIDTH-1)) {
					if (printResult) printf("\e[30mX: %d Y: %d V: %d  \e[91m   X   \e[0m  \e[30mX: %d Y: %d V: %d\e[0m\n", x, y, grid[y][x], x+moveDirX, y+moveDirY, grid[y+moveDirY][x+moveDirX]);
					// Move not allowed
					continue;
				}

				if (printResult) printf("X: %d Y: %d V: %d  \e[95m<----->\e[0m  X: %d Y: %d V: %d\n", x, y, grid[y][x], x+moveDirX, y+moveDirY, grid[y+moveDirY][x+moveDirX]);
				// If space is empty, move enemy there
				if (grid[y+moveDirY][x+moveDirX] == 0) {
					grid[y+moveDirY][x+moveDirX] = grid[y][x];
					grid[y][x] = 0;
				} else {
					// Swap positions with enemy
					int tempSwap = grid[y][x];
					grid[y][x] = grid[y+moveDirY][x+moveDirX];
					grid[y+moveDirY][x+moveDirX] = tempSwap;
				}
			}
		}
	}
}

void PrintGrid(bool clearScreen) {
	printf("\n\e[33m____________________________________________\e[0m\n\n");
	int explosions = 0;
	if (clearScreen) printf("\033[2J\033[H");
	for (int y=0; y<GRID_HEIGHT; y++) {
		printf("\n");
		for (int x=0; x<GRID_WIDTH; x++) {
			grid[y][x] ? printf("\e[32m%d\e[0m ", grid[y][x]) : printf("%d ", grid[y][x]);
			if (grid[y][x] == 5) explosions++;
		}
	}
	printf("\n\nTYPES: \33[31m");
	for (int i=0; i<ENEMY_TYPE_COUNT; i++) printf("%d ", types[i]);
	printf("%d", explosions);
	printf("\33[0m\n\n");
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

Sprite NewSprite(Rectangle rec, Vector2 loc, int frames, bool repeatFrames, float animSpeed, void (*eventOnFinish)(), float holdLastFrame) {
	Sprite newSprite;
	newSprite.exists = true;
	newSprite.rec = rec;
	newSprite.loc = loc;
	newSprite.frames = frames;
	newSprite.repeatAnim = repeatFrames;
	newSprite.animTick = 0;
	newSprite.animSpeed = animSpeed;
	newSprite.currentFrame = 0;
	newSprite.eventOnFinish = eventOnFinish;
	newSprite.holdLast = holdLastFrame;
	return newSprite;
}

void InitSpriteArray() {
	for (int i=0; i<SPRITE_ARRAY_SIZE; i++) {
		sprites[i] = BlankSprite();
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
			if (sprites[i].currentFrame == sprites[i].frames-1 && sprites[i].holdLast > 0) {
				sprites[i].holdLast -= 0.1;
				continue;
			}
			sprites[i].currentFrame++;
			if (sprites[i].currentFrame > sprites[i].frames-1) {
				if (sprites[i].repeatAnim) {
					sprites[i].currentFrame = 0;
				} else {
					if (sprites[i].eventOnFinish) sprites[i].eventOnFinish();
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
			frameRec.x = sprites[i].rec.x + sprites[i].rec.width * sprites[i].currentFrame;
			frameRec.y = sprites[i].rec.y;
			frameRec.width = sprites[i].rec.width;
			frameRec.height = sprites[i].rec.height;
			DrawTextureRec(spriteSheet, frameRec, sprites[i].loc, WHITE);
		}
	}
}

void EnableControls() {
	controlsEnabled = true;
}

void Gameover() {
	PlaySound(SND_gameover);
	sprites[7] = NewSprite((Rectangle){0, 67, 60, 60}, (Vector2){0, 0}, 14, false, 0.3, EnableControls, 0);
	gameMode = GAMEMODE_GAMEOVER;
}

void WorldChangeAnim() {
	controlsEnabled = false;
	sprites[10] = NewSprite((Rectangle){176, 0, 31, 16}, (Vector2){14, 6}, 8, false, 0.15, false, 1.3);
	sprites[11] = NewSprite((Rectangle){270, 3+(16*world), 16, 16}, (Vector2){21, 21}, 7, false, 0.15, EnableControls, 1.40);
	sprites[9] = NewSprite((Rectangle){0, 127, 60, 51}, (Vector2){0, 0}, 17, false, 0.16, false, 0); // Fade
}



//---------------------------------------------------------------------------------------- MAIN

int main() {

	// Variables
	sprites = malloc(30*sizeof(Sprite));
	const char windowed = PIXEL_SIZE;
	const float enemyAnimRate = 0.04;
	float enemyAnimTick = 0;
	int animFrame = 0;
	InitSpriteArray();
	sprites[0] = NewSprite((Rectangle){43, 61, 27, 5}, (Vector2){16, 47}, 8, true, 0.18, false, 0);
	const Color COL_WHITE = {238, 238, 238, 255};
	const Color COL_BLACK = {33, 33, 33, 255};
	Circle circles[10] = {0};
	int circleArrLength = sizeof(circles)/sizeof(Circle);
	for (int i=0; i<circleArrLength; i++) circles[i] = NewCircle();
	float pressA_scrollY = 40;

	// Shake shake
	Vector2 shakeVector = {0, 0};
	float shakeTick = 0;
	float shakeRate = 0.5;

	// Init Window Stuff
	float scale, playAreaX;
	//SetConfigFlags(FLAG_VSYNC_HINT);
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
	Sound SND_click = LoadSound("snd/click.ogg");
	Sound SND_title_music = LoadSound("snd/title_music.ogg");
	Sound SND_win = LoadSound("snd/win.ogg");
	SND_gameover = LoadSound("snd/gameover.ogg");
	Music MUS_world1 = LoadMusicStream("snd/world1.ogg");
	Music MUS_world2 = LoadMusicStream("snd/world2.ogg");
	Music MUS_world3 = LoadMusicStream("snd/world3.ogg");

	// Sprites
	Sprite SP_types = {{0, 0, 40, 10}, {10, 18}};
	Sprite SP_logo = {{0, 40, 41, 27}, {9, 10}};
	Sprite SP_hud = {{0, 20, 60, 10}, {0, 50}};
	Sprite SP_targetRem = {{0, 30, 1, 1}, {0, 0}};
	Sprite SP_timeDot = {{1, 30, 1, 2}, {0, 0}};
	Sprite SP_letter_A = {{3, 35, 3, 5}, {0, 0}};
	Sprite SP_letter_S = {{7, 35, 3, 5}, {0, 0}};
	Sprite SP_letter_K = {{19, 35, 3, 5}, {0, 0}};
	Sprite SP_letter_L = {{23, 35, 3, 5}, {0, 0}};
	Sprite SP_gameover = {{780, 67, 60, 60}, {0, 0}};
	Sprite SP_pressA = {{43, 61, 27, 5}, {16, 47}};

	// Init
	RenderTexture2D target = LoadRenderTexture(screenWidth, screenHeight);
    SetTargetFPS(60);
	ResetLevel();
	PlaySound(SND_title_music);

    while (!WindowShouldClose()) {

        // UPDATE

		if (gameMode == GAMEMODE_TITLE) {

			if (controlsEnabled) {
				switch (GetKeyPressed()) {
					case KEY_ENTER: case KEY_A: case KEY_S: case KEY_K: case KEY_L:
						InitSpriteArray();
						gameMode = GAMEMODE_HELP;
						StopSound(SND_title_music);
						PlaySound(SND_click);
				}
			}
			for (int i=0; i<circleArrLength; i++) {
				circles[i].loc.y -= circles[i].speed;
				if (circles[i].loc.y < -20) {
					circles[i] = NewCircle();
				}
			}

		} else if (gameMode == GAMEMODE_GAME) {

			// Music
			switch (world) {
				case 1: UpdateMusicStream(MUS_world1); break;
				case 2: UpdateMusicStream(MUS_world2); break;
				case 3: UpdateMusicStream(MUS_world3); break;
			}

			// Animation tick / World 2 scramble enemies
			enemyAnimTick += enemyAnimRate;
			if (enemyAnimTick > 1) {
				enemyAnimTick = 0;
				animFrame = !animFrame;
			}

			// World 2
			if (world == 2) {
				world2Tick += 0.015;
				if (world2Tick > 1) {
					world2Tick = 0;
					MoveEnemies(false);
				}
			}

			// World 3
			if (world == 3) {
				invisTick += 0.01;
				if (invisTick > invisSwitch) {
					invisTick = 0;
					lightsOn = !lightsOn;
					invisSwitch *= 1.1;
				}
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
								sprites[0] = NewSprite((Rectangle){88, 13, 41, 14}, (Vector2){9, 18}, 1, false, 0.014, EnableControls, 0);
								sprites[1] = NewSprite((Rectangle){80, 0, 13, 12}, (Vector2){10, 19}, 8, false, 0.2, false, 0);
								sprites[2] = NewSprite((Rectangle){80, 0, 13, 12}, (Vector2){23, 19}, 1, false, 0.014, false, 0);
								sprites[3] = NewSprite((Rectangle){80, 0, 13, 12}, (Vector2){36, 19}, 1, false, 0.014, false, 0);
								break;
							case 1:
								sprites[0] = NewSprite((Rectangle){88, 13, 41, 14}, (Vector2){9, 18}, 1, false, 0.014, EnableControls, 0);
								sprites[2] = NewSprite((Rectangle){80, 0, 13, 12}, (Vector2){23, 19}, 8, false, 0.2, false, 0);
								sprites[3] = NewSprite((Rectangle){80, 0, 13, 12}, (Vector2){36, 19}, 1, false, 0.014, false, 0);
								break;
							case 0:
								pressA_scrollY = 50;
								sprites[0] = NewSprite((Rectangle){88, 13, 41, 14}, (Vector2){9, 18}, 1, false, 0.014, Gameover, 0);
								sprites[3] = NewSprite((Rectangle){80, 0, 13, 12}, (Vector2){36, 19}, 8, false, 0.2, false, 0);
								break;
						}
					}
				}
			}

			// Timer runs out
			if (controlsEnabled) timeLeft -= 0.06;
			if (timeLeft <= 0) {
				pressA_scrollY = 50;
				Gameover();
			}

			// Level completed
			if (!types[0] && !types[1] && !types[2] && !types[3]) {
				currentLevel += 6;
				if (currentLevel > 30) {

					// World completed
					currentLevel = STARTING_LEVEL;
					world++;
					
					// World music and animation
					switch (world) {
						case 2:
							StopMusicStream(MUS_world1);
							PlayMusicStream(MUS_world2);
							WorldChangeAnim();
							break;
						case 3:
							StopMusicStream(MUS_world2);
							PlayMusicStream(MUS_world3);
							WorldChangeAnim();
							break;
						case 4:
							StopMusicStream(MUS_world3);
							PlaySound(SND_win);
							gameMode = GAMEMODE_WIN;
							break;
					}

				}
				ResetLevel();
			}

		} else if (gameMode == GAMEMODE_HELP) {

			if (controlsEnabled) {
				switch (GetKeyPressed()) {
					case KEY_ENTER: case KEY_A: case KEY_S: case KEY_K: case KEY_L:

						// Reset gameplay / Start game
						PlaySound(SND_click);
						StopMusicStream(MUS_world1);
						PlayMusicStream(MUS_world1);
						world = STARTING_WORLD;
						WorldChangeAnim();
						lightsOn = true;
						ResetLevel();
						gameMode = GAMEMODE_GAME;
				}
			}

		} else if (gameMode == GAMEMODE_GAMEOVER) {

			if (controlsEnabled) {
				switch (GetKeyPressed()) {
					case KEY_ENTER: case KEY_A: case KEY_S: case KEY_K: case KEY_L:
						currentLevel = STARTING_LEVEL;
						gameMode = GAMEMODE_HELP;
						StopSound(SND_gameover);
						PlaySound(SND_click);
						lives = 3;
				}
			}

			if (pressA_scrollY > 0) pressA_scrollY -= 0.4;

		}

		// Always run
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

				if (lightsOn) DrawIconGrid(TX_sprites, animFrame, shakeVector);
				DrawTextureRec(TX_sprites, SP_hud.rec, SP_hud.loc, WHITE);

				// Draw target tracker
				for (int i=0; i<remainingTargets; i++)
				DrawTextureRec(TX_sprites, SP_targetRem.rec, (Vector2){1+(i*2), 53}, WHITE);

				// Draw time bar
				for (int i=0; i<timeLeft; i++)
				DrawTextureRec(TX_sprites, SP_timeDot.rec, (Vector2){2+i, 56}, WHITE);

			} else if (gameMode == GAMEMODE_GAMEOVER) {

				DrawTextureRec(TX_sprites, SP_gameover.rec, SP_gameover.loc, WHITE);
				DrawTextureRec(TX_sprites, SP_pressA.rec, (Vector2){SP_pressA.loc.x, SP_pressA.loc.y+pressA_scrollY}, WHITE);

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