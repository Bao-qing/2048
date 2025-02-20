@echo off

:: 创建 dist 目录
if not exist dist mkdir dist
if not exist dist\ttf mkdir dist\ttf
if not exist dist\img mkdir dist\img
if not exist dist\audio mkdir dist\audio

:: 使用 g++ 编译
g++ -std=c++11 -o dist/main main.cpp -I./SDL64/include -L./SDL64/lib -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -mwindows

:: 复制 DLL 文件
copy /Y SDL64\bin\SDL2.dll dist\
copy /Y SDL64\bin\SDL2_image.dll dist\
copy /Y SDL64\bin\SDL2_ttf.dll dist\
copy /Y SDL64\bin\SDL2_mixer.dll dist\
copy /Y SDL64\bin\zlib1.dll dist\
copy /Y SDL64\bin\libfreetype-6.dll dist\

:: 复制资源文件
xcopy /E /Y ttf\* dist\ttf\
xcopy /E /Y img\* dist\img\
xcopy /E /Y audio\* dist\audio\

pause