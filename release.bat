cls
gcc main.c -o reeflx.exe -O3 -Wall -std=c99 -Wno-missing-braces -I include/ -L lib/ -lraylib -lopengl32 -lgdi32 -lwinmm -mwindows
