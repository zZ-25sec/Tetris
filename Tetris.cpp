// RussianTetris.cpp - 修复闪烁问题的俄罗斯方块游戏
#include <iostream>
#include <vector>
#include <memory>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <thread>
#include <algorithm>
#include <conio.h>
#include <windows.h>

using namespace std;

// ==================== 方块颜色枚举 ====================
enum class BlockColor {
    CYAN, BLUE, ORANGE, YELLOW, GREEN, PURPLE, RED, EMPTY
};

// ==================== 游戏状态枚举 ====================
enum class GameState {
    MENU, PLAYING, PAUSED, GAME_OVER
};

// ==================== 方块基类 ====================
class Block {
protected:
    BlockColor color;
    int rotation;
    int x, y;
    
public:
    Block(BlockColor c, int startX, int startY) 
        : color(c), rotation(0), x(startX), y(startY) {}
    
    virtual ~Block() = default;
    
    // 虚函数 - 实现多态
    virtual vector<vector<int>> getShape() const = 0;
    
    // 移动方法
    void moveLeft() { x--; }
    void moveRight() { x++; }
    void moveDown() { y++; }
    void rotate() { rotation = (rotation + 1) % 4; }
    
    // 获取器
    BlockColor getColor() const { return color; }
    int getX() const { return x; }
    int getY() const { return y; }
    int getRotation() const { return rotation; }
    void setPos(int newX, int newY) { x = newX; y = newY; }
};

// ==================== 具体方块类 - 应用继承 ====================
class IBlock : public Block {
public:
    IBlock(int x, int y) : Block(BlockColor::CYAN, x, y) {}
    
    vector<vector<int>> getShape() const override {
        if (rotation % 2 == 0) {
            return {
                {0,0,0,0},
                {1,1,1,1},
                {0,0,0,0},
                {0,0,0,0}
            };
        }
        return {
            {0,0,1,0},
            {0,0,1,0},
            {0,0,1,0},
            {0,0,1,0}
        };
    }
};

class JBlock : public Block {
public:
    JBlock(int x, int y) : Block(BlockColor::BLUE, x, y) {}
    
    vector<vector<int>> getShape() const override {
        switch(rotation % 4) {
            case 0: return {{1,0,0},{1,1,1},{0,0,0}};
            case 1: return {{0,1,1},{0,1,0},{0,1,0}};
            case 2: return {{0,0,0},{1,1,1},{0,0,1}};
            default: return {{0,1,0},{0,1,0},{1,1,0}};
        }
    }
};

class LBlock : public Block {
public:
    LBlock(int x, int y) : Block(BlockColor::ORANGE, x, y) {}
    
    vector<vector<int>> getShape() const override {
        switch(rotation % 4) {
            case 0: return {{0,0,1},{1,1,1},{0,0,0}};
            case 1: return {{0,1,0},{0,1,0},{0,1,1}};
            case 2: return {{0,0,0},{1,1,1},{1,0,0}};
            default: return {{1,1,0},{0,1,0},{0,1,0}};
        }
    }
};

class OBlock : public Block {
public:
    OBlock(int x, int y) : Block(BlockColor::YELLOW, x, y) {}
    
    vector<vector<int>> getShape() const override {
        return {{0,1,1,0},{0,1,1,0},{0,0,0,0}};
    }
};

class SBlock : public Block {
public:
    SBlock(int x, int y) : Block(BlockColor::GREEN, x, y) {}
    
    vector<vector<int>> getShape() const override {
        if (rotation % 2 == 0) {
            return {{0,1,1},{1,1,0},{0,0,0}};
        }
        return {{0,1,0},{0,1,1},{0,0,1}};
    }
};

class TBlock : public Block {
public:
    TBlock(int x, int y) : Block(BlockColor::PURPLE, x, y) {}
    
    vector<vector<int>> getShape() const override {
        switch(rotation % 4) {
            case 0: return {{0,1,0},{1,1,1},{0,0,0}};
            case 1: return {{0,1,0},{0,1,1},{0,1,0}};
            case 2: return {{0,0,0},{1,1,1},{0,1,0}};
            default: return {{0,1,0},{1,1,0},{0,1,0}};
        }
    }
};

class ZBlock : public Block {
public:
    ZBlock(int x, int y) : Block(BlockColor::RED, x, y) {}
    
    vector<vector<int>> getShape() const override {
        if (rotation % 2 == 0) {
            return {{1,1,0},{0,1,1},{0,0,0}};
        }
        return {{0,0,1},{0,1,1},{0,1,0}};
    }
};

// ==================== 方块工厂 ====================
class BlockFactory {
public:
    static unique_ptr<Block> createRandomBlock(int x, int y) {
        srand(static_cast<unsigned int>(time(nullptr)));
        int type = rand() % 7;
        
        switch(type) {
            case 0: return make_unique<IBlock>(x, y);
            case 1: return make_unique<JBlock>(x, y);
            case 2: return make_unique<LBlock>(x, y);
            case 3: return make_unique<OBlock>(x, y);
            case 4: return make_unique<SBlock>(x, y);
            case 5: return make_unique<TBlock>(x, y);
            default: return make_unique<ZBlock>(x, y);
        }
    }
};

// ==================== 棋盘类 ====================
class Board {
private:
    static const int WIDTH = 10;
    static const int HEIGHT = 20;
    vector<vector<BlockColor>> grid;
    
public:
    Board() {
        grid.resize(HEIGHT, vector<BlockColor>(WIDTH, BlockColor::EMPTY));
    }
    
    void reset() {
        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                grid[y][x] = BlockColor::EMPTY;
            }
        }
    }
    
    bool canMove(const Block& block, int dx, int dy) const {
        auto shape = block.getShape();
        int newX = block.getX() + dx;
        int newY = block.getY() + dy;
        
        for (int y = 0; y < shape.size(); y++) {
            for (int x = 0; x < shape[y].size(); x++) {
                if (shape[y][x]) {
                    int boardX = newX + x;
                    int boardY = newY + y;
                    
                    if (boardX < 0 || boardX >= WIDTH || boardY >= HEIGHT) {
                        return false;
                    }
                    
                    if (boardY >= 0 && grid[boardY][boardX] != BlockColor::EMPTY) {
                        return false;
                    }
                }
            }
        }
        return true;
    }
    
    void placeBlock(const Block& block) {
        auto shape = block.getShape();
        
        for (int y = 0; y < shape.size(); y++) {
            for (int x = 0; x < shape[y].size(); x++) {
                if (shape[y][x]) {
                    int boardX = block.getX() + x;
                    int boardY = block.getY() + y;
                    
                    if (boardY >= 0 && boardX >= 0 && boardX < WIDTH && boardY < HEIGHT) {
                        grid[boardY][boardX] = block.getColor();
                    }
                }
            }
        }
    }
    
    int clearLines() {
        int linesCleared = 0;
        
        for (int y = HEIGHT - 1; y >= 0; y--) {
            bool full = true;
            
            for (int x = 0; x < WIDTH; x++) {
                if (grid[y][x] == BlockColor::EMPTY) {
                    full = false;
                    break;
                }
            }
            
            if (full) {
                linesCleared++;
                for (int moveY = y; moveY > 0; moveY--) {
                    for (int x = 0; x < WIDTH; x++) {
                        grid[moveY][x] = grid[moveY - 1][x];
                    }
                }
                // 清空最顶行
                for (int x = 0; x < WIDTH; x++) {
                    grid[0][x] = BlockColor::EMPTY;
                }
                y++; // 重新检查当前行
            }
        }
        return linesCleared;
    }
    
    bool isGameOver() const {
        for (int x = 0; x < WIDTH; x++) {
            if (grid[0][x] != BlockColor::EMPTY) {
                return true;
            }
        }
        return false;
    }
    
    int getWidth() const { return WIDTH; }
    int getHeight() const { return HEIGHT; }
    BlockColor getCell(int x, int y) const { 
        if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) 
            return BlockColor::EMPTY;
        return grid[y][x]; 
    }
};

// ==================== 渲染器类 - 修复闪烁问题 ====================
class Renderer {
private:
    HANDLE consoleHandle;
    vector<vector<char>> screenBuffer;
    vector<vector<int>> colorBuffer;
    int screenWidth, screenHeight;
    
public:
    Renderer() {
        consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        screenWidth = 80;
        screenHeight = 30;
        
        // 初始化屏幕缓冲区
        screenBuffer.resize(screenHeight, vector<char>(screenWidth, ' '));
        colorBuffer.resize(screenHeight, vector<int>(screenWidth, 7));
        
        // 隐藏光标
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(consoleHandle, &cursorInfo);
        cursorInfo.bVisible = FALSE;
        SetConsoleCursorInfo(consoleHandle, &cursorInfo);
    }
    
    // 设置控制台窗口大小
    void setConsoleSize(int width, int height) {
        screenWidth = width;
        screenHeight = height;
        
        SMALL_RECT windowSize = {0, 0, static_cast<SHORT>(width-1), static_cast<SHORT>(height-1)};
        SetConsoleWindowInfo(consoleHandle, TRUE, &windowSize);
        
        COORD bufferSize = {static_cast<SHORT>(width), static_cast<SHORT>(height)};
        SetConsoleScreenBufferSize(consoleHandle, bufferSize);
    }
    
    // 清除缓冲区（不清屏）
    void clearBuffer() {
        for (int y = 0; y < screenHeight; y++) {
            for (int x = 0; x < screenWidth; x++) {
                screenBuffer[y][x] = ' ';
                colorBuffer[y][x] = 7;
            }
        }
    }
    
    // 设置缓冲区中的字符
    void setChar(int x, int y, char ch, int color = 7) {
        if (x >= 0 && x < screenWidth && y >= 0 && y < screenHeight) {
            screenBuffer[y][x] = ch;
            colorBuffer[y][x] = color;
        }
    }
    
    // 设置缓冲区中的字符串
    void setString(int x, int y, const string& str, int color = 7) {
        for (size_t i = 0; i < str.length(); i++) {
            setChar(x + i, y, str[i], color);
        }
    }
    
    // 渲染缓冲区到屏幕（无闪烁）
    void renderBuffer() {
        COORD startPos = {0, 0};
        DWORD charsWritten;
        
        for (int y = 0; y < screenHeight; y++) {
            SetConsoleCursorPosition(consoleHandle, {0, static_cast<SHORT>(y)});
            
            int currentColor = 7;
            SetConsoleTextAttribute(consoleHandle, currentColor);
            
            for (int x = 0; x < screenWidth; x++) {
                if (colorBuffer[y][x] != currentColor) {
                    currentColor = colorBuffer[y][x];
                    SetConsoleTextAttribute(consoleHandle, currentColor);
                }
                cout << screenBuffer[y][x];
            }
        }
    }
    
    // 绘制游戏界面
    void drawGame(const Board& board, const Block* current, const Block* next,
                  int score, int level, int lines, GameState state) {
        clearBuffer();
        
        // 绘制静态边框
        drawBorder();
        
        // 绘制棋盘
        drawBoard(board);
        
        // 绘制当前方块
        if (current) {
            drawBlock(*current, false);
        }
        
        // 绘制信息面板
        drawInfoPanel(score, level, lines);
        
        // 绘制下一个方块
        if (next) {
            drawNextBlock(*next);
        }
        
        // 绘制操作说明
        drawControls();
        
        // 根据游戏状态绘制额外信息
        if (state == GameState::GAME_OVER) {
            drawGameOver(score);
        } else if (state == GameState::PAUSED) {
            drawPause();
        }
        
        // 渲染到屏幕
        renderBuffer();
    }
    
    // 绘制菜单
    void drawMenu() {
        clearBuffer();
        
        setString(30, 5, "RUSSIAN TETRIS", 14);
        setString(30, 7, "1. Start Game", 7);
        setString(30, 8, "2. Instructions", 7);
        setString(30, 9, "3. Exit Game", 7);
        setString(25, 12, "Select option (1-3): ", 7);
        
        renderBuffer();
    }
    
private:
    // 绘制边框
    void drawBorder() {
        // 顶部边框
        setString(0, 0, "+--------------------+", 7);
        
        // 两侧边框
        for (int y = 1; y <= 20; y++) {
            setChar(0, y, '|', 7);
            setChar(21, y, '|', 7);
        }
        
        // 底部边框
        setString(0, 21, "+--------------------+", 7);
    }
    
    // 绘制棋盘
    void drawBoard(const Board& board) {
        for (int y = 0; y < board.getHeight(); y++) {
            for (int x = 0; x < board.getWidth(); x++) {
                BlockColor cell = board.getCell(x, y);
                if (cell != BlockColor::EMPTY) {
                    setChar(1 + x * 2, 1 + y, '[', getConsoleColor(cell));
                    setChar(2 + x * 2, 1 + y, ']', getConsoleColor(cell));
                } else {
                    setChar(1 + x * 2, 1 + y, '.', 8);
                    setChar(2 + x * 2, 1 + y, ' ', 8);
                }
            }
        }
    }
    
    // 绘制方块
    void drawBlock(const Block& block, bool isNext = false) {
        int startX = isNext ? 28 : 1;
        int startY = isNext ? 10 : 1;
        
        int color = getConsoleColor(block.getColor());
        auto shape = block.getShape();
        
        for (int y = 0; y < shape.size(); y++) {
            for (int x = 0; x < shape[y].size(); x++) {
                if (shape[y][x]) {
                    int screenX = startX + (block.getX() + x) * 2;
                    int screenY = startY + block.getY() + y;
                    
                    if (!isNext) {
                        if (screenY >= 1 && screenY < 21) {
                            setChar(screenX, screenY, '[', color);
                            setChar(screenX + 1, screenY, ']', color);
                        }
                    } else {
                        if (screenY >= 10 && screenY < 14) {
                            setChar(screenX, screenY, '[', color);
                            setChar(screenX + 1, screenY, ']', color);
                        }
                    }
                }
            }
        }
    }
    
    // 绘制信息面板
    void drawInfoPanel(int score, int level, int lines) {
        setString(25, 2, "RUSSIAN TETRIS", 14);
        setString(25, 4, "Score: " + to_string(score), 7);
        setString(25, 5, "Level: " + to_string(level), 7);
        setString(25, 6, "Lines: " + to_string(lines), 7);
    }
    
    // 绘制下一个方块
    void drawNextBlock(const Block& block) {
        setString(25, 8, "Next Block:", 7);
        drawBlock(block, true);
    }
    
    // 绘制操作说明
    void drawControls() {
        setString(25, 15, "CONTROLS:", 7);
        setString(25, 16, "Left/Right: Move", 7);
        setString(25, 17, "Up: Rotate", 7);
        setString(25, 18, "Down: Speed up", 7);
        setString(25, 19, "Space: Hard drop", 7);
        setString(25, 20, "P: Pause", 7);
        setString(25, 21, "ESC: Quit", 7);
    }
    
    // 绘制游戏结束界面
    void drawGameOver(int score) {
        // 半透明覆盖层
        for (int y = 8; y <= 14; y++) {
            for (int x = 10; x <= 40; x++) {
                setChar(x, y, ' ', 0);
            }
        }
        
        setString(15, 10, "====================", 12);
        setString(15, 11, "     GAME OVER!     ", 12);
        setString(15, 12, "====================", 12);
        setString(15, 14, "Final Score: " + to_string(score), 7);
        setString(15, 16, "Press ENTER to restart", 7);
        setString(15, 17, "Press ESC to quit", 7);
    }
    
    // 绘制暂停界面
    void drawPause() {
        // 半透明覆盖层
        for (int y = 8; y <= 13; y++) {
            for (int x = 10; x <= 40; x++) {
                setChar(x, y, ' ', 0);
            }
        }
        
        setString(15, 10, "====================", 14);
        setString(15, 11, "      PAUSED        ", 14);
        setString(15, 12, "====================", 14);
        setString(15, 14, "Press P to continue", 7);
    }
    
    // 获取控制台颜色
    int getConsoleColor(BlockColor color) const {
        switch(color) {
            case BlockColor::CYAN: return 11;
            case BlockColor::BLUE: return 9;
            case BlockColor::ORANGE: return 6;
            case BlockColor::YELLOW: return 14;
            case BlockColor::GREEN: return 10;
            case BlockColor::PURPLE: return 13;
            case BlockColor::RED: return 12;
            default: return 7;
        }
    }
};

// ==================== 游戏引擎类 ====================
class GameEngine {
private:
    Board board;
    unique_ptr<Block> currentBlock;
    unique_ptr<Block> nextBlock;
    GameState state;
    int score;
    int level;
    int linesCleared;
    int fallSpeed;
    bool isLocked;
    int lockDelay;
    
public:
    GameEngine() : state(GameState::MENU), score(0), level(1), linesCleared(0), 
                  fallSpeed(1000), isLocked(false), lockDelay(0) {
        srand(static_cast<unsigned int>(time(nullptr)));
    }
    
    void startGame() {
        board.reset();
        score = 0;
        level = 1;
        linesCleared = 0;
        updateSpeed();
        generateNewBlock();
        state = GameState::PLAYING;
    }
    
    void update() {
        if (state != GameState::PLAYING) return;
        
        static int timer = 0;
        timer += 50;
        
        if (timer >= fallSpeed) {
            moveDown();
            timer = 0;
        }
        
        if (isLocked) {
            lockDelay++;
            if (lockDelay >= 30) {
                lockBlock();
                lockDelay = 0;
                isLocked = false;
            }
        }
        
        if (board.isGameOver()) {
            state = GameState::GAME_OVER;
        }
    }
    
    void processInput() {
        if (_kbhit()) {
            int key = _getch();
            
            // 如果是特殊键（箭头键），需要再次读取
            if (key == 0 || key == 224) { // 特殊键的前缀
                int specialKey = _getch(); // 获取特殊键的实际值
                
                if (state == GameState::PLAYING) {
                    switch(specialKey) {
                        case 72: rotate(); break;     // 上箭头
                        case 75: moveLeft(); break;   // 左箭头
                        case 77: moveRight(); break;  // 右箭头
                        case 80: moveDown(); break;   // 下箭头
                    }
                }
            } else {
                // 普通按键
                if (state == GameState::PLAYING) {
                    switch(key) {
                        case 32: hardDrop(); break;   // 空格
                        case 'p':
                        case 'P': state = GameState::PAUSED; break;
                        case 27: state = GameState::MENU; break; // ESC
                    }
                } else if (state == GameState::PAUSED) {
                    if (key == 'p' || key == 'P') {
                        state = GameState::PLAYING;
                    }
                } else if (state == GameState::GAME_OVER) {
                    if (key == 13) { // 回车
                        startGame();
                    } else if (key == 27) { // ESC
                        state = GameState::MENU;
                    }
                } else if (state == GameState::MENU) {
                    if (key == '1') {
                        startGame();
                    } else if (key == '2') {
                        showHelp();
                    } else if (key == '3') {
                        exit(0);
                    }
                }
            }
        }
    }
    
    void moveLeft() {
        if (currentBlock && board.canMove(*currentBlock, -1, 0)) {
            currentBlock->moveLeft();
            resetLock();
        }
    }
    
    void moveRight() {
        if (currentBlock && board.canMove(*currentBlock, 1, 0)) {
            currentBlock->moveRight();
            resetLock();
        }
    }
    
    void moveDown() {
        if (!currentBlock) return;
        
        if (board.canMove(*currentBlock, 0, 1)) {
            currentBlock->moveDown();
            resetLock();
        } else {
            isLocked = true;
        }
    }
    
    void rotate() {
        if (!currentBlock) return;
        
        int oldRotation = currentBlock->getRotation();
        currentBlock->rotate();
        
        if (!board.canMove(*currentBlock, 0, 0)) {
            // 旋转后碰撞，恢复原状
            while (currentBlock->getRotation() != oldRotation) {
                currentBlock->rotate();
            }
        } else {
            resetLock();
        }
    }
    
    void hardDrop() {
        if (!currentBlock) return;
        
        while (board.canMove(*currentBlock, 0, 1)) {
            currentBlock->moveDown();
        }
        lockBlock();
    }
    
    void lockBlock() {
        if (!currentBlock) return;
        
        board.placeBlock(*currentBlock);
        int lines = board.clearLines();
        
        if (lines > 0) {
            updateScore(lines);
        }
        
        generateNewBlock();
        resetLock();
    }
    
    void generateNewBlock() {
        if (!nextBlock) {
            nextBlock = BlockFactory::createRandomBlock(board.getWidth() / 2 - 2, 0);
        }
        
        currentBlock = move(nextBlock);
        nextBlock = BlockFactory::createRandomBlock(board.getWidth() / 2 - 2, 0);
        
        if (!board.canMove(*currentBlock, 0, 0)) {
            state = GameState::GAME_OVER;
        }
    }
    
    void updateScore(int lines) {
        int points[] = {0, 100, 300, 500, 800};
        score += points[lines] * level;
        linesCleared += lines;
        
        // 每消除10行升一级
        level = linesCleared / 10 + 1;
        updateSpeed();
    }
    
    void updateSpeed() {
        fallSpeed = max(100, 1000 - (level - 1) * 100);
    }
    
    void resetLock() {
        isLocked = false;
        lockDelay = 0;
    }
    
    void showHelp() {
        Renderer renderer;
        renderer.setConsoleSize(80, 30);
        
        renderer.clearBuffer();
        renderer.setString(10, 2, "GAME INSTRUCTIONS:", 14);
        renderer.setString(10, 4, "1. Use arrow keys to move blocks", 7);
        renderer.setString(10, 5, "2. Up arrow to rotate blocks", 7);
        renderer.setString(10, 6, "3. Space for instant drop", 7);
        renderer.setString(10, 7, "4. Clear lines to score points", 7);
        renderer.setString(10, 8, "5. More lines at once = more points", 7);
        renderer.setString(10, 10, "Scoring:", 14);
        renderer.setString(10, 11, "1 line: 100 * level", 7);
        renderer.setString(10, 12, "2 lines: 300 * level", 7);
        renderer.setString(10, 13, "3 lines: 500 * level", 7);
        renderer.setString(10, 14, "4 lines: 800 * level", 7);
        renderer.setString(10, 16, "Press any key to return...", 7);
        
        renderer.renderBuffer();
        _getch();
    }
    
    // 获取游戏状态
    GameState getState() const { return state; }
    const Board& getBoard() const { return board; }
    const Block* getCurrentBlock() const { return currentBlock.get(); }
    const Block* getNextBlock() const { return nextBlock.get(); }
    int getScore() const { return score; }
    int getLevel() const { return level; }
    int getLinesCleared() const { return linesCleared; }
};

// ==================== 主函数 ====================
int main() {
    GameEngine game;
    Renderer renderer;
    
    // 设置控制台窗口大小
    renderer.setConsoleSize(80, 30);
    
    // 初始化随机种子
    srand(static_cast<unsigned int>(time(nullptr)));
    
    auto lastUpdateTime = chrono::steady_clock::now();
    auto lastRenderTime = chrono::steady_clock::now();
    
    while (true) {
        auto currentTime = chrono::steady_clock::now();
        
        // 处理输入 - 快速响应
        game.processInput();
        
        // 更新游戏逻辑 - 固定时间间隔 (50ms = 20FPS)
        auto updateElapsed = chrono::duration_cast<chrono::milliseconds>(currentTime - lastUpdateTime).count();
        if (updateElapsed >= 50) {
            game.update();
            lastUpdateTime = currentTime;
        }
        
        // 渲染游戏 - 固定帧率 (33ms = 30FPS，避免闪烁)
        auto renderElapsed = chrono::duration_cast<chrono::milliseconds>(currentTime - lastRenderTime).count();
        if (renderElapsed >= 33) {
            GameState state = game.getState();
            
            if (state == GameState::MENU) {
                renderer.drawMenu();
            } else {
                renderer.drawGame(game.getBoard(), 
                                 game.getCurrentBlock(), 
                                 game.getNextBlock(),
                                 game.getScore(), 
                                 game.getLevel(), 
                                 game.getLinesCleared(),
                                 state);
            }
            
            lastRenderTime = currentTime;
        }
        
        // 避免CPU占用过高
        this_thread::sleep_for(chrono::milliseconds(10));
    }
    
    return 0;
}