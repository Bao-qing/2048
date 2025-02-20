#include <cstring>
#include <cstdlib>
#include <cmath>
#include "SDL64/include/SDL2/SDL.h"
#include "SDL64/include/SDL2/SDL_image.h"
#include "SDL64/include/SDL2/SDL_ttf.h"
#include "SDL64/include/SDL2/SDL_mixer.h"
#include <fstream>
#include <random>
#include <unistd.h>
#undef main

const int LOADANIMATION = 1; // 是否加载动画

class Grid {
public:
    static const int GRID_SIZE = 4; // 地图大小
    int cells[GRID_SIZE][GRID_SIZE] = {{0}}; // 地图
    int cellsTemp[GRID_SIZE][GRID_SIZE] = {{0}}; // 缓存地图
    int cellsMove[GRID_SIZE][GRID_SIZE] = {{0}}; // 移动矩阵,记录每个格子需要移动的格数,用于滑动动画，之后再移动地图合并计算。
    int cellsUsed[GRID_SIZE][GRID_SIZE] = {{0}}; // 使用矩阵，表示产生合并的位置，用于数字膨胀动画。
    int moveFlag = 0; // 移动标志，用于判断是否有移动
    int new_block_pos1[2] = {0}; // 新方块位置1
    int new_block_pos2[2] = {0}; // 新方块位置2

    Grid() {
        memset(cells, 0, sizeof(cells));
        addRandomTile(new_block_pos1);
        addRandomTile(new_block_pos2);
    }

    void init() {
        memset(cells, 0, sizeof(cells));
        addRandomTile(new_block_pos1);
        addRandomTile(new_block_pos2);
    }

    void addRandomTile(int new_block_pos[2]) {
        int emptyCells = 0;
        int positions[GRID_SIZE * GRID_SIZE][2];

        for (int y = 0; y < GRID_SIZE; ++y) {
            for (int x = 0; x < GRID_SIZE; ++x) {
                if (cells[x][y] == 0) {
                    positions[emptyCells][0] = x;
                    positions[emptyCells][1] = y;
                    emptyCells++;
                }
            }
        }

        if (emptyCells > 0) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, emptyCells - 1);
            std::uniform_int_distribution<> tileDis(0, 9);

            int index = dis(gen);
            cells[positions[index][0]][positions[index][1]] = (tileDis(gen) == 0) ? 4 : 2;
            new_block_pos[0] = positions[index][0];
            new_block_pos[1] = positions[index][1];
        }
    }

    /**
     * \brief 向上移动，并没有实际移动，只是记录每个格子需要移动的格数，用于动画后再计算新地图
     * \return 单步分数
     */
    int moveup() {
        int score = 0;
        int i, j;
        int m, n;
        // 记录每个格子需要移动的格数
        // 移动矩阵初始化为0
        for (i = 0; i < 4; i++) {
            for (j = 0; j < 4; j++) {
                cellsUsed[i][j] = 0;
            }
        }
        for (i = 0; i < 4; i++) // 遍历地图
        {
            for (j = 0; j < 4; j++) {
                if (cells[i][j] != 0) {
                    if (j == 0) {
                        continue;
                    } else {
                        n = 0; // 位移
                        for (m = j - 1; m >= 0; m--) // 从当前格子向上遍历，直到遇到非空格子
                        {
                            if (cells[i][m] != 0) // 非空格
                            {
                                if (cells[i][m] != cells[i][j] ||
                                    cellsUsed[i][m] != 0) // 不相等或已经被使用，不可再移动，当前方块可移动位数为目前可移动位数加碰到的方块的可移动位数
                                {
                                    n += cellsMove[i][m];
                                    break;
                                }
                                if (cells[i][m] == cells[i][j] && cellsUsed[i][m] ==
                                    0) // 相等且未被使用，可移动，当前方块可移动位数为目前可移动位数加碰到的方块的可移动位数再加上合并的一位，并记录碰到的方块被使用，当前方块被使用

                                {
                                    // 加分
                                    score += cells[i][j] * 2;
                                    cellsUsed[i][m] = 1; // 碰到的方块被使用
                                    cellsUsed[i][j] = 2; // 当前方块被使用，1表示被合并，2表示被移动，其实都一样
                                    n++;
                                    n += cellsMove[i][m]; // 碰到的方块的位移，后面的方块要跟随移动
                                    break;
                                }
                            } else // 空格，可以移动，目前可移动位数加一
                            {
                                n++;
                            }
                        }

                        cellsMove[i][j] = n;
                    }
                }
            }
        }
        // 合并移动和使用矩阵,将使用矩阵跟随移动矩阵移动
        for (i = 0; i < 4; i++) {
            for (j = 0; j < 4; j++) {
                if (cellsMove[i][j] != 0) {
                    if (cellsUsed[i][j] != 2) {
                        // 排除2（被合并方块），否则一定会覆盖合并方块的1
                        cellsUsed[i][j - cellsMove[i][j]] = cellsUsed[i][j];
                        cellsUsed[i][j] = 0;
                    }
                }
            }
        }
        return score;
    }

    /**
     * \brief 矩阵转置
     * \param direction 移动方向
     * \return 无
     */
    static void transposition_matrix(int matrix[4][4], int direction) {
        // 矩阵的不同转置，用于不同方向的移动
        int i, j;
        if (direction == 2) {
            int temp;
            for (i = 0; i < 4; i++) {
                for (j = 0; j < 2; j++) {
                    temp = matrix[i][j];
                    matrix[i][j] = matrix[i][3 - j];
                    matrix[i][3 - j] = temp;
                }
            }
        } else if (direction == 3) {
            int temp;
            for (i = 0; i < 4; i++) {
                for (j = 0; j < 4; j++) {
                    if (i < j) {
                        temp = matrix[i][j];
                        matrix[i][j] = matrix[j][i];
                        matrix[j][i] = temp;
                    }
                }
            }
        } else if (direction == 4) {
            int temp;
            for (i = 0; i < 4; i++) {
                for (j = 0; j < 4; j++) {
                    if (i + j < 3) {
                        temp = matrix[3 - i][3 - j];
                        matrix[3 - i][3 - j] = matrix[j][i];
                        matrix[j][i] = temp;
                    }
                }
            }
        }
    }

    void transposition(int direction) {
        transposition_matrix(cells, direction);
        transposition_matrix(cellsMove, direction);
        transposition_matrix(cellsUsed, direction);
    }

    int renewBoard(int direction) {
        memset(cellsMove, 0, sizeof(cellsMove));
        memset(cellsUsed, 0, sizeof(cellsUsed));
        // int score_add = 0;

        transposition(direction); // 游戏中实际上仅有一个向上的游戏操作，其他操作都由矩阵转置实现，即将矩阵转置后，统一向上移动，再转置回来
        int score_add = moveup(); // 向上移动，返回单步分数
        // score += score_add;                           // 加分
        transposition(direction); // 转置回来地图
        // 如果没有移动，不生成新的方块，不进行动画evt.key.keysym.scancode
        moveFlag = 0;
        for (int i = 0; i < 4; i++) // 遍历移动矩阵，判断是否有移动
        {
            for (int j = 0; j < 4; j++) {
                if (cellsMove[i][j] != 0) {
                    moveFlag = 1;
                }
            }
        }
        return score_add;
    }

    void renewCells(int direction) {
        transposition(direction); // 重新转置地图，准备计算新的地图
        // transposition(move, evt.key.keysym.scancode); // 重新转置移动矩阵，准备计算新的地图

        // 缓存地图
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                cellsTemp[i][j] = cells[i][j];
            }
        }

        for (int i = 0; i < 4; i++) // 移动地图，根据移动矩阵，将每个格子移动到对应位置，移动到相同格子的数字相加
        {
            for (int j = 0; j < 4; j++) {
                if (cellsMove[i][j] != 0) {
                    cells[i][j - cellsMove[i][j]] += cells[i][j];
                    cells[i][j] = 0;
                }
            }
        }
        transposition(direction); // 计算完成，转置回来地图
        // transposition(direction); // 转置回来移动矩阵，不过移动矩阵已经不再使用，所以这一步可以省略
    }

    unsigned short nextStep() {
        // 统计空格数，如果有空格，生成新的方块
        int empty = 0;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if (cells[i][j] == 0) {
                    empty++;
                }
            }
        }
        // 如果没有空格，不生成新的方块，并且判断是否游戏结束
        if (empty != 0) {
            if (moveFlag != 0) // 再次使用move——flag，判断没有移动，不生成新的方块
            {
                addRandomTile(new_block_pos1);
                new_block_pos2[0] = 5; // 标记为5，表示不用
            } else {
                new_block_pos1[0] = 5;
                new_block_pos2[0] = 5;
            }
        } else {
            new_block_pos1[0] = 5;
            new_block_pos2[0] = 5;
        }
        // 判断是否游戏结束
        int end = 1;
        if (empty == 0 || (empty == 1 && new_block_pos1[0] <= 4)) {
            for (int i = 0; i < 4; i++) // 遍历，当没有空格时，判断是否有相邻的相同数字，如果有，游戏未结束
            {
                for (int j = 0; j < 4; j++) {
                    if (i < 3) {
                        if (cells[i][j] == cells[i + 1][j]) {
                            end = 0;
                        }
                    }
                    if (j < 3) {
                        if (cells[i][j] == cells[i][j + 1]) {
                            end = 0;
                        }
                    }
                }
            }
        } else {
            end = 0;
        }

        // 查找2048，如果有，游戏胜利
        // all[1][1] = 2048;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if (cells[i][j] == 2048) {
                    end = 2;
                    // draw(pRenderer, all, tex, font, gameover, score); // 绘图，以游戏胜利形式绘图
                    // continue;                                         // 游戏胜利，不再进行下面的操作，防止游戏胜利的绘图被覆盖
                }
            }
        }

        if (new_block_pos2[0] <= 4) {
            cellsUsed[new_block_pos2[0]][new_block_pos2[1]] = 3;
        }
        if (new_block_pos1[0] <= 4) {
            cellsUsed[new_block_pos1[0]][new_block_pos1[1]] = 3;
        }

        return end;
    }

    const int *operator[](int index) const { return cells[index]; }
};

class Renderer {
public:
    SDL_Rect rect_tryagain{}; // 重新开始按钮位置
    Renderer() {
        SDL_Init(SDL_INIT_EVERYTHING);
        pWin = SDL_CreateWindow("Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
        pRenderer = SDL_CreateRenderer(pWin, -1, SDL_RENDERER_ACCELERATED);
        TTF_Init(); // 初始化字体库
        font = TTF_OpenFont("ttf/font.ttf", 80); // 打开字体
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
        TTF_SetFontStyle(font, TTF_STYLE_BOLD);
        loadBackground();
        creatTexture(); // 创建数字纹理

        rect_tryagain = {
            (int) (ORIGIN_X + 0.3853 * MAP_WIDTH), (int) (ORIGIN_Y + 0.665 * MAP_HEIGHT), (int) BLOCK_WIDTH,
            (int) (0.0867 * MAP_HEIGHT)
        }; // tryagain的位置
    }

    void loadBackground() {
        //**渲染主游戏背景**//
        SDL_SetRenderDrawColor(pRenderer, 238, 228, 218, 255);
        SDL_RenderClear(pRenderer);
        SDL_Rect pos = {0, 0, (int) WIDTH, (int) HEIGHT};
        // 读取背景图片
        SDL_Surface *bmp = IMG_Load("img/bg.png");
        // 创建纹理
        bg = SDL_CreateTextureFromSurface(pRenderer, bmp);
        SDL_FreeSurface(bmp);
        // 纹理写入渲染器
        SDL_RenderCopy(pRenderer, bg, nullptr, &pos);
        // 刷新渲染器
        SDL_RenderPresent(pRenderer);
    }
    /**
     * \brief 获取数字对应的颜色
     * \param number 数字
     * \return 颜色结构体
     */
    static SDL_Color getcolor(int number) {
        // 使用结构体数组储存颜色
        /*颜色：
        2：{238, 228, 218};
        4：237, 224, 200
        8：rgb(242, 177, 121)
        16：rgb(245, 149, 99)
        32；rgb(246, 124, 95)
        64：rgb(246, 94, 59)
        128：rgb(237, 207, 114)
        256：rgb(237, 204, 97)
        512：rgb(237, 200, 80)
        ......
        */
        SDL_Color colors[8] = {
            {238, 228, 218, 255},
            {237, 224, 200, 255},
            {242, 177, 121, 255},
            {245, 149, 99, 255},
            {246, 124, 95, 255},
            {246, 94, 59, 255},
            {237, 207, 114, 255},
            {237, 204, 97, 255}
        };
        switch (number) {
            case 2:
                return colors[0];
            case 4:
                return colors[1];
            case 8:
                return colors[2];
            case 16:
                return colors[3];
            case 32:
                return colors[4];
            case 64:
                return colors[5];
            case 128:
                return colors[6];
            default:
                return colors[7];
        };
    }

    void draw(int all[4][4], int gameover, int score, int max_score) {
        int move[4][4] = {{0}}; // 移动矩阵
        int direction = 1; // 移动方向
        int scoreadd = 0; // 单步分数
        animation(move, all, direction, score, max_score, scoreadd, 0,gameover); // 不加载动画
    };

    /**
     * \brief 动画，移动时划过
     * \param move 移动矩阵
     * \param all 地图
     * \param direction 移动方向
     * \return 无
     */
    void animation(int move[4][4], int all[4][4], int direction, int score, int max_score, int scoreadd,
                   uint8_t loadAnimation,int gameover=0) {
        int move_times = ANIMATION_TIMES; // 每一步，每个滑行时渲染的帧数
        int delay_time = ANIMATION_DELAY; // 每帧之间的延迟，可以控制动画速度

        SDL_Rect pos = {0, 0, (int) WIDTH, (int) HEIGHT};
        SDL_Rect rect;
        SDL_Rect font_rect;
        SDL_Color font_color = {119, 110, 101, 255};
        SDL_Texture *texture_score_add = nullptr;
        SDL_RenderClear(pRenderer);
        SDL_RenderCopy(pRenderer, bg, nullptr, &pos); // 先绘制背景，再在背景上绘制方块和文本

        char number[6]; // 用于itoa函数的缓冲区
        int hz = 0; // 帧数
        size_t add_scorelen = 0;

        if (!loadAnimation) {
            hz = move_times;
        }
        // 制作纹理
        if (score) {
            font_color.r = 119;
            font_color.g = 110;
            font_color.b = 101;
            itoa(scoreadd, number, 10);
            add_scorelen = strlen(itoa(scoreadd, number, 10));
            char temp[10];
            strcpy(temp, "+");
            strcat(temp, number);
            strcpy(number, temp);
            SDL_Surface *surface_score_add = TTF_RenderText_Blended(font, number, font_color);
            // 创建纹理
            texture_score_add = SDL_CreateTextureFromSurface(pRenderer, surface_score_add);
            SDL_FreeSurface(surface_score_add);
        }

        for (; hz <= move_times; hz++) // 逐帧计算位置，逐帧输出图像
        {
            SDL_RenderCopy(pRenderer, bg, nullptr, &pos); // 每次都要从新绘制背景，否则会有残影
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    if (all[i][j] != 0) {
                        SDL_Color block_color = getcolor(all[i][j]); // 获取颜色
                        // 位置
                        if (direction == 1) // 根据移动方向，计算位置
                        {
                            rect.x = (ORIGIN_X + GAP_AND_WIDTH_X * i);
                            rect.y = ORIGIN_Y + GAP_AND_HEIGTH_Y * j - (GAP_AND_HEIGTH_Y * (move[i][j])) * (
                                         (1 + sin(PI * hz / move_times - PI / 2)) / 2); // 计算位置
                        } else if (direction == 2) {
                            rect.x = ORIGIN_X + GAP_AND_WIDTH_X * i;
                            rect.y = ORIGIN_Y + GAP_AND_HEIGTH_Y * j + (GAP_AND_HEIGTH_Y * (move[i][j])) * (
                                         (1 + sin(PI * hz / move_times - PI / 2)) / 2);
                        } else if (direction == 3) {
                            rect.x = ORIGIN_X + GAP_AND_WIDTH_X * i - (GAP_AND_WIDTH_X * (move[i][j])) * (
                                         (1 + sin(PI * hz / move_times - PI / 2)) / 2);
                            rect.y = ORIGIN_Y + GAP_AND_HEIGTH_Y * j;
                        } else if (direction == 4) {
                            rect.x = ORIGIN_X + GAP_AND_WIDTH_X * i + (GAP_AND_WIDTH_X * (move[i][j])) * (
                                         (1 + sin(PI * hz / move_times - PI / 2)) / 2);
                            rect.y = ORIGIN_Y + GAP_AND_HEIGTH_Y * j;
                        }

                        rect.w = BLOCK_WIDTH;
                        rect.h = BLOCK_HEIGHT;
                        SDL_SetRenderDrawColor(pRenderer, block_color.r, block_color.g, block_color.b, block_color.a); // 设置颜色
                        SDL_RenderFillRect(pRenderer, &rect); // 绘制方块

                        // 文字
                        // 调整字体位置和大小
                        font_rect = rect; // 先定位到目前方块位置，再调整
                        font_rect.x += BLOCK_FONT_OFFSET_X;
                        font_rect.y += BLOCK_FONT_OFFSET_Y;
                        font_rect.w = BLOCK_FONT_WIDTH;
                        font_rect.h = BLOCK_FONT_HEIGHT;
                        if (all[i][j] > 10) {
                            font_rect.x -= BLOCK_WIDTH / 10;
                            font_rect.w += BLOCK_WIDTH / 5;
                        }
                        if (all[i][j] > 100) {
                            font_rect.x -= BLOCK_WIDTH / 11;
                            font_rect.w += BLOCK_WIDTH / 5;
                        }
                        // 渲染文本
                        int texture_index = 1;
                        while ((all[i][j]>>texture_index) != 1) {
                            texture_index ++;
                        }
                        SDL_RenderCopy(pRenderer, number_texture[texture_index-1], nullptr, &font_rect);
                    }
                }
            }

            // 棋盘绘制完成，绘制分数和浮动加分
            // 浮动加分
            if (scoreadd) {

                // 定位
                // font_rect.x = 0.78 * WIDTH;
                font_rect.y = 0.03125 * HEIGHT;
                // font_rect.w = 0.0666666 * WIDTH;
                font_rect.h = 0.04125 * HEIGHT;

                int offset_float_score = ADD_SCORE_FLOAT_LENTH * (1 + sin(PI * hz / move_times - PI / 2)) / 2;
                font_rect.x = 0.6116666667 * WIDTH - 3 * add_scorelen; // 根据数字位数微调
                font_rect.w = 0.0375 * HEIGHT + 10 * add_scorelen;
                font_rect.y -= offset_float_score;
                SDL_RenderCopy(pRenderer, texture_score_add, nullptr, &font_rect);

            }

            score_draw(score, max_score); // 绘制分数
            SDL_RenderPresent(pRenderer);
            SDL_Delay(delay_time); // 每帧之间的延迟，可以控制动画速度
        }
        SDL_DestroyTexture(texture_score_add);

        if (gameover) // 不同的gameover值，绘制不同的界面，gameover=0，正常绘制，不再添加其他界面，gameover=1，绘制游戏结束界面，gameover=2，绘制游戏胜利界面
        {
            SDL_Rect rect_start = {(int) ORIGIN_X, (int) ORIGIN_Y, (int) MAP_WIDTH, (int) MAP_HEIGHT};
            // 游戏结束界面的位置，即游戏棋盘的位置

            // 游戏结束
            if (gameover == 1) {
                SDL_Surface *over = IMG_Load("img/gameover.png"); // 读取gameover图片
                SDL_Texture *tex_over = SDL_CreateTextureFromSurface(pRenderer, over); // 创建纹理
                SDL_SetTextureBlendMode(tex_over, SDL_BLENDMODE_BLEND); // 设置纹理混合模式
                SDL_SetTextureAlphaMod(tex_over, 120); // 设置纹理透明度
                // 纹理写入渲染器
                SDL_RenderCopy(pRenderer, tex_over, nullptr, &rect_start);
                SDL_FreeSurface(over); // 释放表面
                SDL_DestroyTexture(tex_over); // 释放纹理
            } else {
                SDL_Surface *over = IMG_Load("img/win.png"); // 读取win图片，与gameover相同
                SDL_Texture *tex_over = SDL_CreateTextureFromSurface(pRenderer, over);
                SDL_SetTextureBlendMode(tex_over, SDL_BLENDMODE_BLEND);
                SDL_SetTextureAlphaMod(tex_over, 120);
                // 纹理写入渲染器
                SDL_RenderCopy(pRenderer, tex_over, nullptr, &rect_start);
                SDL_FreeSurface(over);
                SDL_DestroyTexture(tex_over);
            }

            // 读取tryagain图片
            SDL_Surface *tryagain = IMG_Load("img/tryagain.png");
            // 创建纹理
            SDL_Texture *tex_tryagain = SDL_CreateTextureFromSurface(pRenderer, tryagain);
            // 纹理写入渲染器
            SDL_RenderCopy(pRenderer, tex_tryagain, nullptr, &rect_tryagain);
            SDL_FreeSurface(tryagain);
            SDL_DestroyTexture(tex_tryagain);
            SDL_RenderPresent(pRenderer);
        }
    }

    /**
     * \brief 膨胀和出现动画
     * \param used 使用矩阵
     * \param move 移动矩阵
     * \param all_temp 缓存地图
     * \param all 地图
     * \param score 分数
     * \return 无
     */
    void bubble_animation(int used[4][4], int move[4][4], int all_temp[4][4], int all[4][4], int score, int max_score) {
        // 气泡动画
        //  读取最高分
        unsigned int hz = 0;
        int i, j;
        SDL_Rect pos = {0, 0, (int) WIDTH, (int) HEIGHT}; // 背景位置
        SDL_Color color; // 颜色
        SDL_Rect rect; // 方块位置
        SDL_Rect block_rect;
        SDL_Rect font_rect; // 文本位置
        SDL_Color font_color = {119, 110, 101, 255}; // 默认文本颜色，棕色
        SDL_RenderClear(pRenderer); // 清空渲染器
        SDL_RenderCopy(pRenderer, bg, nullptr, &pos); // 先绘制背景，再在背景上绘制方块和文本
        int amplitude = BUBBLE_ANIMATION_AMPLITUDE; // 膨胀动画幅度
        int amplitude_font = 10;
        int times = 15; // 膨胀动画步数
        int delay_time = 4; // 每帧之间的延迟

        char number[6];

        // 计算合并矩阵
        int count[4][4] = {0}; // 对每格存在的方块计数，如果有两个方块，则产生合并
        for (i = 0; i < 4; i++) {
            for (j = 0; j < 4; j++) {
                if (all_temp[i][j] != 0) {
                    count[i][j]++;
                }

                if (move[i][j] != 0) {
                    count[i][j]--;
                }
            }
        }

        for (hz = 0; hz <= times; hz++) {
            SDL_RenderCopy(pRenderer, bg, nullptr, &pos); // 先绘制背景，再在背景上绘制方块和文本
            for (i = 0; i < 4; i++) // 遍历地图，绘制方块和文本
            {
                for (j = 0; j < 4; j++) {
                    if (all[i][j] != 0 && used[i][j] != 1 && used[i][j] != 3) // 数字不为0，绘制方块和文本
                    {
                        color = getcolor(all[i][j]); // 获取颜色
                        rect.x = (ORIGIN_X + GAP_AND_WIDTH_X * i); // 计算方块位置
                        rect.y = (ORIGIN_Y + GAP_AND_HEIGTH_Y * j);
                        rect.w = BLOCK_WIDTH;
                        rect.h = BLOCK_HEIGHT;
                        SDL_SetRenderDrawColor(pRenderer, color.r, color.g, color.b, color.a); // 设置颜色
                        SDL_RenderFillRect(pRenderer, &rect); // 绘制方块

                        // 调整字体位置和大小
                        font_rect = rect; // 先定位到方块位置，再调整
                        font_rect.x += BLOCK_FONT_OFFSET_X;
                        font_rect.y += BLOCK_FONT_OFFSET_Y;
                        font_rect.w = BLOCK_FONT_WIDTH;
                        font_rect.h = BLOCK_FONT_HEIGHT;
                        if (all[i][j] > 10) {
                            font_rect.x -= BLOCK_WIDTH / 10;
                            font_rect.w += BLOCK_WIDTH / 5;
                        }
                        if (all[i][j] > 100) {
                            font_rect.x -= BLOCK_WIDTH / 11;
                            font_rect.w += BLOCK_WIDTH / 5;
                        }
                        int texture_index = 1;
                        while ((all[i][j]>>texture_index) != 1) {
                            texture_index ++;
                        }
                        SDL_RenderCopy(pRenderer, number_texture[texture_index-1], nullptr, &font_rect);
                    }

                    if (used[i][j] == 1) // 此处产生了合并
                    {
                        double offset = amplitude * sin(hz * PI / times); // 计算偏移量
                        if (hz == times) {
                            // 最后一帧，微调
                            offset = 0;
                        }
                        color = getcolor(all[i][j]); // 获取颜色

                        rect.x = ORIGIN_X + GAP_AND_WIDTH_X * i; // 计算方块位置
                        rect.y = ORIGIN_Y + GAP_AND_HEIGTH_Y * j;
                        rect.w = BLOCK_WIDTH;
                        rect.h = BLOCK_HEIGHT;

                        SDL_SetRenderDrawColor(pRenderer, color.r, color.g, color.b, color.a); // 设置颜色
                        block_rect = rect;
                        block_rect.x -= offset;
                        block_rect.y -= offset;
                        block_rect.w += 2 * offset;
                        block_rect.h += 2 * offset;

                        SDL_RenderFillRect(pRenderer, &block_rect); // 绘制方块

                        // 调整字体位置和大小
                        font_rect = rect; // 先定位到方块位置，再调整

                        // 字体膨胀
                        int offset_font = amplitude_font * sin(hz * PI / times);
                        //
                        if (hz == times) // 最后一帧，字体位置微调
                        {
                            offset_font = 0;
                        }
                        font_rect.x += BLOCK_FONT_OFFSET_X - offset_font;
                        font_rect.y += BLOCK_FONT_OFFSET_Y - offset_font;
                        font_rect.w = BLOCK_FONT_WIDTH + 2 * offset_font;
                        font_rect.h = BLOCK_FONT_HEIGHT + 2 * offset_font;

                        if (all[i][j] > 10) {
                            font_rect.x -= BLOCK_WIDTH / 10;
                            font_rect.w += BLOCK_WIDTH / 5;
                        }
                        if (all[i][j] > 100) {
                            font_rect.x -= BLOCK_WIDTH / 11;
                            font_rect.w += BLOCK_WIDTH / 5;
                        }
                        // 渲染文本
                        int texture_index = 1;
                        while ((all[i][j]>>texture_index) != 1) {
                            texture_index ++;
                        }
                        SDL_RenderCopy(pRenderer, number_texture[texture_index-1], nullptr, &font_rect);
                    }

                    if (used[i][j] == 3) // 此处出现方块
                    {
                        float offset = BLOCK_WIDTH / 2 * (((float) times - (float) hz) / (float) times); // 计算偏移量
                        color = getcolor(all[i][j]); // 获取颜色

                        rect.x = ORIGIN_X + GAP_AND_WIDTH_X * i; // 计算方块位置
                        rect.y = ORIGIN_Y + GAP_AND_HEIGTH_Y * j;
                        rect.w = (int) BLOCK_WIDTH;
                        rect.h = (int) BLOCK_HEIGHT;

                        SDL_SetRenderDrawColor(pRenderer, color.r, color.g, color.b, color.a); // 设置颜色
                        block_rect = rect;
                        block_rect.x += offset;
                        block_rect.y += offset;
                        block_rect.w -= 2 * offset;
                        block_rect.h -= 2 * offset;

                        SDL_RenderFillRect(pRenderer, &block_rect); // 绘制方块

                        // 调整字体位置和大小
                        font_rect = rect; // 先定位到方块位置，再调整
                        font_rect.x += BLOCK_FONT_OFFSET_X;
                        font_rect.y += BLOCK_FONT_OFFSET_Y;
                        font_rect.w = BLOCK_FONT_WIDTH;
                        font_rect.h = BLOCK_FONT_HEIGHT;
                        if (all[i][j] > 10) {
                            font_rect.x -= BLOCK_WIDTH / 10;
                            font_rect.w += BLOCK_WIDTH / 5;
                        }
                        if (all[i][j] > 100) {
                            font_rect.x -= BLOCK_WIDTH / 11;
                            font_rect.w += BLOCK_WIDTH / 5;
                        }
                        // 动画缩放

                        font_rect.x += offset;
                        font_rect.y += offset;
                        font_rect.w -= 2 * offset;
                        font_rect.h -= 2 * offset;

                        // 渲染文本
                        int texture_index = 1;
                        while ((all[i][j]>>texture_index) != 1) {
                            texture_index ++;
                        }
                        SDL_RenderCopy(pRenderer, number_texture[texture_index-1], nullptr, &font_rect); // 绘制文本，将字体纹理写入渲染器
                    }
                }
            }
            score_draw(score, max_score); // 绘制分数
            SDL_RenderPresent(pRenderer);
            SDL_Delay(delay_time); // 每帧之间的延迟，可以控制动画速度
        }
    }

    void score_draw(int score, int max_score) {
        SDL_Rect font_rect; // 文本位置
        // 渲染分数
        // score = 204;
        // 制作分数纹理（如果缓存与现在不同）
        char score_str[10]; // 分数字符串
        int scorelen; // 分数位数
        scorelen = strlen(itoa(score, score_str, 10)); // 获取分数位数
        if(score_texture_value != score) {
            itoa(score, score_str, 10); // 将分数转换为字符串
            SDL_Surface *surface_score = nullptr; // 分数表面
            SDL_Color font_color_score = {255, 255, 255, 255}; // 分数颜色，白色
            surface_score = TTF_RenderText_Blended(font, score_str, font_color_score); // 创建分数表面
            score_texture = SDL_CreateTextureFromSurface(pRenderer, surface_score); // 创建分数纹理
            score_texture_value = score; // 缓存分数
            SDL_FreeSurface(surface_score);
        }
        if (scorelen == 1) // 根据数字位数微调
        {
            font_rect.w = 0.03333333 * WIDTH;
        } else if (scorelen == 2) {
            font_rect.w = 0.0666666 * WIDTH;
        } else {
            font_rect.w = 0.0833333 * WIDTH;
        }

        font_rect.x = SCORE_POSITION_X - 0.005 * WIDTH * scorelen; // 根据数字位数微调

        font_rect.y = SCORE_POSITION_Y;
        font_rect.h = SCORE_HEIGHT;
        //printf("--score: %s--\n", score_str);
        SDL_RenderCopy(pRenderer, score_texture, nullptr, &font_rect); // 绘制分数，将分数纹理写入渲染器

        // 渲染最高分                                                       // 渲染最高分，与渲染分数相同，不再注释
        char max_score_str[10];
        int max_scorelen;
        max_scorelen = strlen(itoa(max_score, max_score_str, 10));
        if (max_score_texture_value != max_score) {
            itoa(max_score, max_score_str, 10);
            SDL_Surface *surface_max_score = nullptr;
            SDL_Color font_color_max_score = {255, 255, 255, 255};
            surface_max_score = TTF_RenderText_Blended(font, max_score_str, font_color_max_score);
            max_score_texture = SDL_CreateTextureFromSurface(pRenderer, surface_max_score);
            max_score_texture_value = max_score;
            SDL_FreeSurface(surface_max_score);
        }
        if (max_scorelen == 1) // 根据数字位数微调
        {
            font_rect.w = (int)(0.03333333 * WIDTH);
        } else if (max_scorelen == 2) {
            font_rect.w = (int)(0.0666666 * WIDTH);
        } else {
            font_rect.w = (int)(0.0833333 * WIDTH);
        }
        font_rect.x = (int)((float)MAX_SCORE_POSITION_X - 0.005 * WIDTH * max_scorelen); // 根据数字位数微调
        font_rect.y = MAX_SCORE_POSITION_Y;
        font_rect.h = MAX_SCORE_HEIGHT;
        //printf("--max_score: %s--\n", max_score_str);
        SDL_RenderCopy(pRenderer, max_score_texture, nullptr, &font_rect);

    }

    void release_resource() {
        // 此时已经退出消息循环，游戏已经被关闭，释放资源
        SDL_DestroyRenderer(pRenderer); // 释放渲染器
        SDL_DestroyWindow(pWin); // 释放窗口
        SDL_DestroyTexture(bg); // 释放纹理
        SDL_Quit(); // 退出SDL
        TTF_CloseFont(font); // 释放字体
        // 释放数字纹理
        for (int i=0;i<11;i++) {
            SDL_DestroyTexture(number_texture[i]);
        }
    }

    void creatTexture() {
        char number_str[10];
        SDL_Color font_color = {119, 110, 101, 255};
        for (int i=1;i<12;i++) {
            int number = 2 << (i-1);
            // 数字大于8时，颜色变成白色
            if (number >= 8) {
                font_color.r = 255;
                font_color.g = 255;
                font_color.b = 255;
            } else {
                font_color.r = 119;
                font_color.g = 110;
                font_color.b = 101;
            }

            SDL_Surface *surface = TTF_RenderText_Blended(font, itoa(number, number_str, 10), font_color);

            // 创建纹理
            SDL_Texture *texture = SDL_CreateTextureFromSurface(pRenderer, surface);
            number_texture[i-1] = texture;
        }
    }
private:
    SDL_Window *pWin; // 窗口
    SDL_Renderer *pRenderer; // 渲染器
    SDL_Texture *bg{}; // 背景
    TTF_Font *font; // 字体

    SDL_Texture* number_texture[11]{}; // 数字纹理
    SDL_Texture* score_texture{}; // 分数纹理缓存
    SDL_Texture* max_score_texture{}; // 最高分纹理缓存
    int score_texture_value = -1; // 分数
    int max_score_texture_value = -1; // 最高分
    // 动画参数
    const float PI = 3.1415926;
    const float SCALE = 0.7; // 缩放比例

    const float WIDTH = 600.0 * SCALE; // 窗口宽度
    const float HEIGHT = 800.0 * SCALE; // 窗口高度

    const float ORIGIN_X = 60.0 * SCALE; // 起始位置_X
    const float ORIGIN_Y = 184.0 * SCALE; // 起始位置_Y

    const float BLOCK_WIDTH = 106.66 * SCALE; // 方块宽度
    const float BLOCK_HEIGHT = 105.33 * SCALE; // 方块高度
    const float GAP_X = 14.34 * SCALE; // 间隙_X
    const float GAP_Y = 13.17 * SCALE; // 间隙_Y
    const float GAP_AND_WIDTH_X = BLOCK_WIDTH + GAP_X; // 方块间隔_X,包括方块宽度和间隙
    const float GAP_AND_HEIGTH_Y = BLOCK_HEIGHT + GAP_Y; // 方块间隔_Y，包括方块高度和间隙
    const float MAP_WIDTH = 4 * BLOCK_WIDTH + 3 * GAP_X; // 地图宽度
    const float MAP_HEIGHT = 4 * BLOCK_HEIGHT + 3 * GAP_Y; // 地图高度

    const float BLOCK_FONT_OFFSET_X = 0.328 * BLOCK_WIDTH; // 方块上的字体偏移_X
    const float BLOCK_FONT_OFFSET_Y = 0.171 * BLOCK_HEIGHT; // 方块上的字体偏移_Y
    const float BLOCK_FONT_WIDTH = 0.328 * BLOCK_WIDTH; // 方块上的字体宽度
    const float BLOCK_FONT_HEIGHT = 0.665 * BLOCK_HEIGHT; // 方块上的字体高度

    const float SCORE_POSITION_X = 0.615 * WIDTH; // 分数位置_X
    const float SCORE_POSITION_Y = 0.0475 * HEIGHT; // 分数位置_Y
    const float SCORE_HEIGHT = 0.04125 * HEIGHT; // 分数高度
    const float MAX_SCORE_POSITION_X = 0.8 * WIDTH; // 最高分位置_X
    const float MAX_SCORE_POSITION_Y = 0.0475 * HEIGHT; // 最高分位置_Y
    const float MAX_SCORE_HEIGHT = SCORE_HEIGHT; // 最高分高度

    const int ANIMATION_TIMES = 20; // 动画帧数
    const int ANIMATION_DELAY = 5; // 动画帧间延时
    const float ADD_SCORE_FLOAT_LENTH = 0.03333333 * WIDTH; // 分数增加的浮动长度
    const float BUBBLE_ANIMATION_AMPLITUDE = 0.025 * WIDTH; // 膨胀动画振幅
};

class InputManager {
public:
    struct EVENT_TYPE  {
        int QUIT = 1;
        int KEYDOWN = 2;
        int MOUSEBUTTONDOWN = 3;
    }event_type;

    struct Envent {
        int type = 0;
        int direction = 0;
        int x = 0;
        int y = 0;
    }event;

    struct DIRECTION {
        int UP = 1;
        int DOWN = 2;
        int LEFT = 3;
        int RIGHT = 4;
        int GENERATE = 5;
    }direction;

    SDL_Event sdl_evt{}; // 事件

    int getEvent() {
        if (SDL_PollEvent(&sdl_evt)) {
            // SDL_SCANCODE_RIGHT = 79,
            // SDL_SCANCODE_LEFT = 80,
            // SDL_SCANCODE_DOWN = 81,
            // SDL_SCANCODE_UP = 82,
            if (sdl_evt.type == SDL_QUIT) {
                event.type = event_type.QUIT;
                return 1;
            }
            if (sdl_evt.type == SDL_KEYDOWN) {
                switch (sdl_evt.key.keysym.scancode) {
                    case 'd':
                    case SDL_SCANCODE_RIGHT:
                        event.type = event_type.KEYDOWN;
                        event.direction = direction.RIGHT;
                        break;
                    case 'a':case SDL_SCANCODE_LEFT:
                        event.type = event_type.KEYDOWN;
                        event.direction = direction.LEFT;
                        break;
                    case 's':case SDL_SCANCODE_DOWN:
                        event.type = event_type.KEYDOWN;
                        event.direction = direction.DOWN;
                        break;
                    case 'w':case SDL_SCANCODE_UP:
                        event.type = event_type.KEYDOWN;
                        event.direction = direction.UP;
                        break;
                    case 'p':
                        event.type = event_type.KEYDOWN;
                        event.direction = direction.GENERATE;
                        break;
                    default:
                        return 0;
                }
                return 1;
            }
            if (sdl_evt.type == SDL_MOUSEBUTTONDOWN) {
                event.type =event_type.MOUSEBUTTONDOWN;
                event.x = sdl_evt.button.x;
                event.y = sdl_evt.button.y;
                return 1;
            }
            return 0;
        }
        return 0;
    }

};

class AudioManager {
public:
    AudioManager() {
        // 加载音频

        if (Mix_OpenAudio(44100, AUDIO_S16SYS, 1, 2048) != 0) {
            // fprintf(stderr, "Unable to initialize audio: %s\n", Mix_GetError());
            exit(1);
        }

        add_wav = Mix_LoadMUS("audio/add.wav");
        over_wav = Mix_LoadMUS("audio/gameover.wav");
        win_wav = Mix_LoadMUS("audio/win.wav");
    }

    void playAddSoundEffects() {
        Mix_PlayMusic(add_wav, 0);
    }

    void playGameOverSoundEffects() {
        Mix_PlayMusic(over_wav, 0);
    }

    void playWinSoundEffects() {
        Mix_PlayMusic(win_wav, 0);
    }

    void release_resource() {
        Mix_FreeMusic(add_wav);
        Mix_FreeMusic(over_wav);
        Mix_FreeMusic(win_wav);
        Mix_CloseAudio();
    }

private:
    Mix_Music *add_wav;
    Mix_Music *over_wav;
    Mix_Music *win_wav;
};

class ScoreManager {
public:
    int score = 0;

    ScoreManager() {
        std::ifstream file(filename);
        if (file.is_open())
            file >> highScore;
    }

    int GetHighScore() const { return highScore; }

    void SaveHighScore() {
        if (score > highScore) {
            highScore = score;
            std::ofstream file(filename);
            file << highScore;
        }
    }

private:
    int highScore = 0;
    const char *filename = "img/score.txt";
};

class GameMain {
public:
    Grid grid;
    Renderer renderer;
    AudioManager audioManager;
    ScoreManager scoreManager;
    InputManager inputManager;

    void main() {
        int gameover = 0; // 游戏结束标志 1:gameover 2:win
        int max_score = scoreManager.GetHighScore(); // 最高分
        renderer.draw(grid.cells, 0, 0, max_score);
        while (true) {
            if (scoreManager.score > max_score) {
                max_score = scoreManager.score;
            }
            if (inputManager.getEvent()) // 有事件，SDL_PollEvent用于取出事件
            {
                if (inputManager.event.type == inputManager.event_type.QUIT) // 检测到退出事件，退出
                {
                    scoreManager.SaveHighScore(); // 保存最高分
                    renderer.release_resource(); // 释放资源
                    audioManager.release_resource(); // 释放音频资源
                    break;
                }
                // 鼠标点击事件，只用于gameover时检测是否点击了tryagain
                if (inputManager.event.type == inputManager.event_type.MOUSEBUTTONDOWN)
                {
                    if (inputManager.event.x > renderer.rect_tryagain.x && inputManager.event.x < renderer.rect_tryagain.x + renderer.
                        rect_tryagain.w &&
                        inputManager.event.y > renderer.rect_tryagain.y && inputManager.event.y < renderer.rect_tryagain.y + renderer.
                        rect_tryagain.h &&
                        gameover) // 点击了tryagain
                    {
                        // 重新开始,清空地图,分数清零,
                        scoreManager.score = 0;
                        gameover = 0;
                        grid.init();
                        // 绘图
                        renderer.draw(grid.cells, gameover, scoreManager.score, max_score);
                    }
                } else if (inputManager.event.type == inputManager.event_type.KEYDOWN) // 键盘事件
                {
                    if (gameover) // 游戏结束，不响应键盘事件，只响应鼠标事件，用于点击tryagain
                    {
                        continue;
                    }

                    // 按下p，生成一个1024，用于测试胜利条件，此功能被保留
                    if (inputManager.event.direction == inputManager.direction.GENERATE)
                    {
                        grid.cells[3][3] = 1024;
                        renderer.draw(grid.cells, gameover, scoreManager.score, max_score);
                        continue;
                    }


                    int score_add = grid.renewBoard(inputManager.event.direction); // 记录单步分数
                    scoreManager.score += score_add; // 计算总分
                    if (grid.moveFlag != 0) // 有移动，进行动画，否则不进行动画，以节约资源
                    {
                        if (score_add) {
                            // 有合成，播放音频
                            audioManager.playAddSoundEffects();
                        }
                        renderer.animation(grid.cellsMove, grid.cells, inputManager.event.direction, scoreManager.score,
                                           max_score,
                                           score_add, LOADANIMATION); // 动画
                    }

                    grid.renewCells(inputManager.event.direction); // 合并地图
                    gameover = grid.nextStep(); // 进行下一步并 判断是否游戏结束

                    if (gameover == 1) {
                        renderer.draw(grid.cells, gameover, scoreManager.score, max_score); // 绘图，以游戏结束形式绘图
                        audioManager.playGameOverSoundEffects(); // 播放游戏结束音频
                        renderer.draw(grid.cells, gameover, scoreManager.score, max_score); // 绘图，以游戏结束形式绘图
                        continue;
                    }
                    if (gameover == 2) {
                        audioManager.playWinSoundEffects(); // 播放游戏胜利音频
                    }
                    renderer.bubble_animation(grid.cellsUsed, grid.cellsMove, grid.cellsTemp, grid.cells,
                                              scoreManager.score, max_score); // 膨胀动画
                    renderer.draw(grid.cells, gameover, scoreManager.score, max_score); // 绘图，正常绘图
                }
            }else {
                usleep(2000);
            }
        }
    };
};

// 这里是主函数
int main() {
    GameMain gameMain;
    gameMain.main();
    return 0;
}
