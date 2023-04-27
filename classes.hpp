#ifndef ARCANOID_CLASSES_HPP
#define ARCANOID_CLASSES_HPP

#include <bits/stdc++.h>
#include <SFML/Graphics.hpp>

enum Resolution {
    WIDTH_SMALL = 800,
    HEIGHT_SMALL = 450,
    WIDTH_MEDIUM = 1200,
    HEIGHT_MEDIUM = 675,
    WIDTH_BIG = 1600,
    HEIGHT_BIG = 900,
};

enum Active {
    MENU,
    SETTINGS,
    GAME,
    PAUSE,
};

enum PlatformSize {
    PS_SHORT = 50,
    PS_MEDIUM = 75,
    PS_LONG = 100,
    PS_HEIGHT = 15,
};

enum BallSize {
    BS_MEDIUM = 15,
};

enum ObstacleNum {
    OB_ROW = 12,
    OB_COLUMN = 28,
};

enum Difficulty {
    DF_HARD = 3,
    DF_MEDIUM = 2,
    DF_EASY = 1,
};

enum PlatformSpeed {
    PSP_FAST = 12,
    PSP_MEDIUM = 10,
    PSP_SLOW = 6,
};

enum BallSpeed {
    BSP_FAST = 100,
    BSP_MEDIUM = 75,
    BSP_SLOW = 50,
};

enum EventType {
    NEW_GAME,
    CONTINUE,
    TO_SETTINGS,
    QUIT,
    TO_MENU,
    TO_MENU_NOSAVE,
    SWITCH_DIFFICULTY,
    SWITCH_RESOLUTION,
    DESTROY,
    FALL,
    HORIZONTAL_COLLISION,
    VERTICAL_COLLISION,
    COLLISION,
    TO_PAUSE,
    TO_GAME,
    SAVE,
    FRAME,
    SCORE_UP,
    LIVES_DOWN,
};

class DisplayObject;

struct Event {
    EventType type;
    DisplayObject* obj;
};

class DisplayObject : public sf::RectangleShape {
protected:
    sf::Color color;
    sf::FloatRect bounds;
    bool visible;
    sf::Vector2f position;
    DisplayObject(sf::Vector2f size, sf::Vector2f pos = sf::Vector2f(0, 0), sf::Color col = sf::Color(255, 255, 255)) : sf::RectangleShape(size) {
        this->setFillColor(col);
        color = col;
        this->setPosition(pos);
        position = pos;
        bounds = this->getGlobalBounds();
        visible = true;
    };
public:
    virtual void draw(sf::RenderWindow &target);
    virtual void setColor(sf::Color col);
    virtual void setVisible(bool state);
    virtual bool isVisible();
    virtual void checkCollision(DisplayObject* obj);
    virtual void eventHandler(Event e);
    virtual void checkBounds();
    virtual sf::FloatRect getBound();
};

class MovableObject : public DisplayObject {
protected:
    sf::Vector2f velocity;
    MovableObject(sf::Vector2f size, sf::Vector2f pos = sf::Vector2f(0, 0), sf::Color col = sf::Color(255,255,255), sf::Vector2f vel = sf::Vector2f(0, 0)) : DisplayObject(size, pos, col) {
        velocity = vel;
    }
public:
    virtual void move();
    virtual void move(sf::Vector2f vel);
    virtual void setVelocity(sf::Vector2f vel);
    virtual sf::Vector2f getVelocity();
};

class Statistics {
private:
    int lives, score;
    float delay;
    std::string name;
    sf::Clock clock;
public:
    Statistics(int l, int s, sf::Time t, std::string n, float delay);
    int getLives();
    int getScore();
    std::string getName();
    float getTime();
    void setLives(int num);
    void setScore(int score);
};

class TextBlock : public DisplayObject{
private:
    sf::Text *text;
public:
    TextBlock(sf::Vector2f size, sf::Vector2f pos, sf::Color col, std::string title);
    void draw(sf::RenderWindow &target) override;
    void setText(std::string str);
};

class Ball : public MovableObject {
public:
    Ball(float size, sf::Vector2f pos = sf::Vector2f(0, 0), sf::Color col = sf::Color(255,255,255), sf::Vector2f vel = sf::Vector2f(0, 0)) : MovableObject(sf::Vector2f(size, size), pos, col, vel) {};
    std::pair <Ball*, Ball*> mitosis();
    void eventHandler(Event e) override;
};

class Platform : public MovableObject {
public:
    Platform(sf::Vector2f size, sf::Vector2f pos = sf::Vector2f(0, 0), sf::Color col = sf::Color(255,255,255), float vel = 0) : MovableObject(size, pos, col, sf::Vector2f (vel, 0)) {};
    void eventHandler(Event e) override;
};

class Bonus {
private:
    EventType bonus;
public:
    Bonus();
    void sendEvent();
};

class Obstacle : public DisplayObject {
private:
    std::vector<Bonus*> bonuses;
public:
    Obstacle(sf::Vector2f size, sf::Vector2f pos, sf::Color col);
    void eventHandler(Event e) override;
};

class Button : public DisplayObject {
private:
    EventType event;
    TextBlock* text;
public:
    Button(sf::Vector2f size, sf::Vector2f pos, sf::Color col, std::string title, EventType e);
    void draw(sf::RenderWindow &target) override;
    void sendEvent();
    void setColor(sf::Color col);
    bool underMouse(int mouseX, int mouseY);
    void setText(std::string str);
};

class StatusBar : public DisplayObject {
private:
    std::vector<TextBlock*> bar;
    Button* menu;
public:
    StatusBar(sf::Vector2f size, Statistics* stats);
    void draw(sf::RenderWindow &target) override;
    void update(Statistics* stats, sf::Vector2i mousePos, bool pressed);
};

class MessageBox : public DisplayObject {
private:
    Button* cancel;
    TextBlock* text;
public:
    MessageBox(sf::Vector2f size, sf::Vector2f pos = sf::Vector2f(0, 0), sf::Color col = sf::Color(255,255,255));
    void draw(sf::RenderWindow &target) override;
    void setVisible(bool state) override;
    void setText(std::string text);
};

class Settings {
private:
    static Difficulty difficulty;
    static std::pair <Resolution, Resolution> resolution;
public:
    Settings(); 
    static Difficulty getDiff();
    void setDiff(Difficulty diff);
    static std::pair<Resolution, Resolution> getResolution();
    void setResolution(std::string str);
    static std::string getDiffStr();
    static std::string getResolutionStr();
};

class Menu : public DisplayObject {
private:
    std::vector <Button*> items;
    TextBlock* text;
public:
    Menu(sf::Vector2f size, sf::Color col, std::vector<Button*> items, std::string title);
    void draw(sf::RenderWindow &target) override;
    Button* getButton(int index);
    void update(sf::Vector2i mousePos, bool pressed);
};

class GameField : public DisplayObject {
private:
    Statistics* data;
    MessageBox* message;
    StatusBar* board;
    std::vector<DisplayObject*> objects;
    std::vector<MovableObject*> move_objects;
    std::vector<Ball*> balls;
    std::vector<Platform*> platforms;
    void eventHandler(Event e) override;
    void moveObjects();
    void checkCollisions();
public:
    GameField();
    void draw(sf::RenderWindow &target) override;
    void addItem(DisplayObject *obj);
    void addItem(Ball *obj);
    void addItem(Platform *obj);
    void addItem(Statistics *obj);
    void addItem(StatusBar* obj);
    Statistics* getData();
    void update(sf::Vector2i mousePos, bool pressed);
    std::vector<DisplayObject*> getObjects();
};

class Player {
private:
    Statistics *stats;
    Platform *platform;
    std::vector <Ball*> balls;
public:
    Player(std::string name);
    Player(Statistics* s, Platform* p, std::vector <Ball*> b);
    Platform* getPlatform();
    Statistics* getStatistics();
    std::vector <Ball*> getBalls();
};

class Players {
private:
    std::vector <Player*> players;
public:
    Players();
    void addPlayer(Player *player);
    std::vector <Player*> getPlayers();
};

class Game;

class History {
private:
    std::ifstream inFile;
    std::ofstream outFile;
public:
    void load(Game* game);
    void save(Game* game);
};

class Game {
    friend class History;
private:
    sf::Clock timer;
    Active state;
    History* history;
    sf::RenderWindow *window;
    Players *sessionPlayers;
    Menu *menu, *settingsMenu, *pauseMenu;
    Settings *settings;
    GameField *gameField;
    void update();
    void eventHandler(Event e);
    void initMenus();
public:
    Game();
    void create();
    void init();
    void process();
};

class EventDispatcher {
private:
    static std::queue<Event> eventQueue, gameEventQueue;
public:
    static void setEvent(Event e);
    static bool pollEvent(Event &e);
    static void setGameEvent(Event e);
    static bool pollGameEvent(Event &e);
};

#endif