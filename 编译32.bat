gcc main.c -o main img/demo.o -ISDL/include -LSDL/lib -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -mwindows
echo "SUCCESS"
pause