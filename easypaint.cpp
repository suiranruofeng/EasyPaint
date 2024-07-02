#include <SDL2/SDL.h>
#include <iostream>
#include <vector>


// 线条基类
class Line {
public:
    Line(int r, int g, int b) : r(r), g(g), b(b) {}
    virtual void draw() = 0;
//增加了颜色功能
protected:
    int r, g, b;
};

// 直线类
class StraightLine : public Line {
public:
    StraightLine(int x1, int y1, int x2, int y2, SDL_Renderer* renderer, int r, int g, int b)
        : Line(r, g, b), x1(x1), y1(y1), x2(x2), y2(y2), renderer(renderer) {}

    void draw() {
        SDL_SetRenderDrawColor(renderer, r, g, b, 255);
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
    }

private:
    int x1, y1, x2, y2;
    SDL_Renderer* renderer;
};

// 自由线类
class FreeLine : public Line {
public:
    FreeLine(SDL_Renderer* renderer, int r, int g, int b) : Line(r, g, b), renderer(renderer) {}

    void addPoint(int x, int y) {
        points.push_back(SDL_Point());
        points.back().x = x;
        points.back().y = y;
    }

    void draw() {
        SDL_SetRenderDrawColor(renderer, r, g, b, 255);
        if (points.size() >= 2) {
            for (size_t i = 1; i < points.size(); i++) {
                SDL_RenderDrawLine(renderer, points[i - 1].x, points[i - 1].y, points[i].x, points[i].y);
            }
        }
    }

private:
    std::vector<SDL_Point> points;
    SDL_Renderer* renderer;
};

//绘图类
class LineDrawingApp {
public:
    LineDrawingApp() : currentColor(0) {
        // 初始化 SDL
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            std::cout << "Unable to initialize SDL: " << SDL_GetError() << std::endl;
            return;
        }

        // 创建窗口
        window = SDL_CreateWindow("EasyPaint 1.1",
                                  SDL_WINDOWPOS_UNDEFINED,
                                  SDL_WINDOWPOS_UNDEFINED,
                                  1920, 1080,
                                  SDL_WINDOW_SHOWN);
        if (window == NULL) {
            std::cout << "Unable to create window: " << SDL_GetError() << std::endl;
            SDL_Quit();
            return;
        }

        // 创建渲染器
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (renderer == NULL) {
            std::cout << "Unable to create renderer: " << SDL_GetError() << std::endl;
            SDL_DestroyWindow(window);
            SDL_Quit();
            return;
        }
    // 设置初始背景颜色为白色
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    currentColor = 0;
    }

    void run() {
        bool running = true;
        bool isDrawing = false;
        bool isDrawingStraightLine = false;
        Line* currentLine = NULL;
        int startX = 0, startY = 0;


        while (running) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    running = false;
                }
                else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_e) {
                    // 按下 'e' 键清空画布
                    lines.clear();
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);//保持白色
                    SDL_RenderClear(renderer);
                    SDL_RenderPresent(renderer);
                }
                else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_r) {
                    // 切换颜色
                    currentColor = (currentColor + 1) % 4;
                }
                else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_q) {
                    // 按下 'q' 键退出程序
                    running = false;
                }
                else if (event.type == SDL_MOUSEBUTTONDOWN) {
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        // 鼠标左键按下时开始绘制自由线条
                        isDrawing = true;
                        currentLine = new FreeLine(renderer, colorOptions[currentColor][0], colorOptions[currentColor][1], colorOptions[currentColor][2]);
                        static_cast<FreeLine*>(currentLine)->addPoint(event.button.x, event.button.y);
                    }
                    else if (event.button.button == SDL_BUTTON_RIGHT) {
                        // 鼠标右键按下时开始绘制直线
                        isDrawingStraightLine = true;
                        startX = event.button.x;
                        startY = event.button.y;
                    }
                }
                else if (event.type == SDL_MOUSEMOTION && isDrawing) {
                    // 鼠标移动时更新自由线条
                    static_cast<FreeLine*>(currentLine)->addPoint(event.motion.x, event.motion.y);
                    // 清空画布并重新绘制所有线条
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                    SDL_RenderClear(renderer);
                    for (size_t i = 0; i < lines.size(); i++) {
                        lines[i]->draw();
                    }
                    if (currentLine) {
                        currentLine->draw();
                    }
                    SDL_RenderPresent(renderer);
                }
                else if (event.type == SDL_MOUSEBUTTONUP) {
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        // 鼠标左键释放时结束自由线条绘制
                        isDrawing = false;
                        static_cast<FreeLine*>(currentLine)->addPoint(event.button.x, event.button.y);
                        lines.push_back(currentLine);
                        currentLine = NULL;
                    }
                    else if (event.button.button == SDL_BUTTON_RIGHT) {
                        // 鼠标右键释放时结束直线绘制
                        isDrawingStraightLine = false;
                        lines.push_back(new StraightLine(startX, startY, event.button.x, event.button.y, renderer, colorOptions[currentColor][0], colorOptions[currentColor][1], colorOptions[currentColor][2]));
                        // 清空画布并重新绘制所有线条
                        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                        SDL_RenderClear(renderer);
                        for (size_t i = 0; i < lines.size(); i++) {
                            lines[i]->draw();
                        }
                        SDL_RenderPresent(renderer);
                    }
                }
            }
        }

        // 清理资源
        for (size_t i = 0; i < lines.size(); i++) {
            delete lines[i];
        }
        if (currentLine) {
            delete currentLine;
        }
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    std::vector<Line*> lines;
    int currentColor;
    static const int colorOptions[][3];


};


//颜色表
const int LineDrawingApp::colorOptions[][3] = {
    {0, 0, 0},    // 黑色
    {255, 0, 0},  // 红色
    {0, 255, 0},  // 绿色
    {0, 0, 255}   // 蓝色
};

int main(int argc, char *argv[]) {
    LineDrawingApp app;
    app.run();
    return 0;
}