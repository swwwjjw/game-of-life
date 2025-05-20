#include "GameOfLife.h"
#include <iostream>
#include <vector>

class GOL {
    SDL_Window* window; //Окно
    SDL_Renderer* renderer; //Рендерер
    bool running; //Флаг продолжения игры
    bool pause; //Флаг паузы
    const int BLOCK_SIZE = 10; //Размер ячейки сетки в пикселях
    int DELAY = 100; //Задержка
    int PLAYGROUND_HEIGHT = 50; //Высота поля в клетках
    int PLAYGROUND_WIDTH = 50; //Ширина поля в клетках
    int SCREEN_HEIGHT; //Высота поля в пикселях
    int SCREEN_WIDTH; //Ширина поля в пикселях
    std::vector<std::vector<bool>> playground; //Вектор для отслеживания состояния клеток

public:
    //Конструктор
    GOL(int delay, int height, int width) : pause(true), running(false), playground(width, std::vector<bool>(height, false)) {
        DELAY = delay;
        PLAYGROUND_HEIGHT = height;
        PLAYGROUND_WIDTH = width;
        SCREEN_HEIGHT = PLAYGROUND_HEIGHT * BLOCK_SIZE;
        SCREEN_WIDTH = PLAYGROUND_WIDTH * BLOCK_SIZE;
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr << "Could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
            exit(-1);
        }
        window = SDL_CreateWindow("MainWindow", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (!window) {
            std::cerr << "Could not be created! SDL_Error: " << SDL_GetError() << std::endl;
            SDL_Quit();
            exit(-1);
        }
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (!renderer) {
            std::cerr << "Could not be created! SDL_Error: " << SDL_GetError() << std::endl;
            SDL_DestroyWindow(window);
            SDL_Quit();
            exit(-1);
        }
        //Изначальное заполнение поля
        /*srand(3);
        for (int x = 0; x < PLAYGROUND_WIDTH; x++) {
            for (int y = 0; y < PLAYGROUND_HEIGHT; y++) {
                playground[x][y] = rand() % 2;
            }
        }*/
    }

    //Деструктор, очистка ресурсов
    ~GOL() {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    //Основной цикл игры
    void run() {
        running = true;
        SDL_Event event;
        size_t lastUpdateTime = SDL_GetTicks64();
        while (running) {
            size_t currentTime = SDL_GetTicks64();
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    running = false;
                    break;
                }
                if (event.type == SDL_KEYDOWN) {
                    if (event.key.keysym.sym == SDLK_SPACE) {
                        pause = !pause;
                        break;
                    }
                    else if (event.key.keysym.sym == SDLK_ESCAPE) {
                        clearGrid();
                        break;
                    }
                }
                if (event.type == SDL_MOUSEBUTTONDOWN) {
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        int x = event.button.x / BLOCK_SIZE;
                        int y = event.button.y / BLOCK_SIZE;
                        if (x >= 0 && y >= 0 && x < PLAYGROUND_WIDTH && y < PLAYGROUND_HEIGHT) {
                            playground[x][y] = !playground[x][y];
                        }
                        pause = true;
                        break;
                    }
                }
            }
            if (currentTime - lastUpdateTime >= DELAY) {
                if (!pause) {
                    updateGrid();
                }
                lastUpdateTime = currentTime;
            }
            drawGrid();
        }
    }

private:
    //Отрисовка поля
    void drawGrid() {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Белый фон
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255); // Серый цвет для сетки
        for (int x = 0; x < SCREEN_WIDTH; x += BLOCK_SIZE) {
            SDL_RenderDrawLine(renderer, x, 0, x, SCREEN_HEIGHT);
        }
        for (int y = 0; y < SCREEN_HEIGHT; y += BLOCK_SIZE) {
            SDL_RenderDrawLine(renderer, 0, y, SCREEN_WIDTH, y);
        }
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        for (int x = 0; x < PLAYGROUND_WIDTH; x++) {
            for (int y = 0; y < PLAYGROUND_HEIGHT; y++) {
                if (playground[x][y]) {
                    SDL_Rect rectangle = { x * BLOCK_SIZE, y * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE };
                    SDL_RenderFillRect(renderer, &rectangle);
                }
            }
        }
        SDL_RenderPresent(renderer);
    }

    //Очистка поля
    void clearGrid() {
        for (size_t i = 0; i < PLAYGROUND_HEIGHT; i++) {
            for (size_t j = 0; j < PLAYGROUND_WIDTH; j++) {
                playground[i][j] = false;
            }
        }
    }

    //Подсчет количества соседей
    size_t countNeighbors(int x, int y) {
        size_t count = 0;
        for (int i = -1; i < 2; i++) {
            for (int j = -1; j < 2; j++) {
                if (i == 0 && j == 0) {
                    continue;
                }
                int neighborX = x + i;
                int neighborY = y + j;
                if (neighborX >= 0 && neighborY >= 0 && neighborX < PLAYGROUND_WIDTH && neighborY < PLAYGROUND_HEIGHT) {
                    if (playground[neighborX][neighborY]) {
                        count++;
                    }
                }
            }
        }
        return count;
    }

    //Обновление поля
    void updateGrid() {
        std::vector<std::vector<bool>> newPlayground = playground;
        for (int x = 0; x < PLAYGROUND_WIDTH; x++) {
            for (int y = 0; y < PLAYGROUND_HEIGHT; y++) {
                size_t neighbors = countNeighbors(x, y);
                if (playground[x][y]) {
                    newPlayground[x][y] = (neighbors == 2 || neighbors == 3);
                }
                else {
                    newPlayground[x][y] = neighbors == 3;
                }
            }
        }
        playground = newPlayground;
    }
};

//Вывод справки
void printHelp() {
    std::cout << "Usage: GameOfLife [options]\n"
        << "Options:\n"
        << "  -w <width>    Window width (default: 50)\n"
        << "  -h <height>   Window height (default: 50)\n"
        << "  -d <delay>     Delay (default: 100)\n"
        << "  --help        Show this help message\n";
}

int main(int argc, char** argv) {
    int delay = 100, height = 50, width = 50;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-w" && i + 1 < argc) {
            width = std::atoi(argv[++i]);
        }
        else if (arg == "-h" && i + 1 < argc) {
            height = std::atoi(argv[++i]);
        }
        else if (arg == "-d" && i + 1 < argc) {
            delay = std::atoi(argv[++i]);
        }
        else if (arg == "--help") {
            printHelp();
            return 0;
        }
        else {
            std::cerr << "Unknown argument: " << arg << "\n";
            printHelp();
            return 1;
        }
    }
    if (delay < 0 || height <= 0 || width <= 0) {
        std::cerr << "Values must be non-negative!" << std::endl;
        return -1;
    }
    try {
        GOL game(delay, height, width);
        game.run();
    }
    catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << std::endl;
        return -1;
    }
    return 0;
}