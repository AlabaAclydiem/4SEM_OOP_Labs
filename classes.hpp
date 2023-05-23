#ifndef ARCANOID_CLASSES_HPP
#define ARCANOID_CLASSES_HPP

#include <bits/stdc++.h>
#include <SFML/Graphics.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

enum Resolution {
    W0 = 1920,
    H0 = 1000,

    W1 = 1600,
    H1 = 900,
    
    W2 = 1560,
    H2 = 877,

    W3 = 1520,
    H3 = 720,

    W4 = 1480,
    H4 = 720,

    W5 = 1366,
    H5 = 768,

    W6 = 1280,
    H6 = 720,

    W7 = 800,
    H7 = 450,

    FW = 1921,
    FH = 1080,
};

enum Active {
    MENU,
    SETTINGS,
    GAME,
    PAUSE,
    MESSAGE_START,
    MESSAGE_WIN,
    MESSAGE_LOSE
};

enum Coefficients {
    BALL_COEF = 1,
    PLATFORM_COEF = 1,
};

enum PlatformSize {
    PS_SHORT = 75,
    PS_MEDIUM = 100,
    PS_LONG = 150,
    PS_HEIGHT = 20,
};

enum BallSize {
    BS_MEDIUM = 9,
};

enum ObstacleNum {
    OB_ROW = 12,
    OB_COLUMN = 28,
};

enum Difficulty {
    DF_HARD = 5,
    DF_HM = 4,
    DF_MEDIUM = 3,
    DF_ME = 2,
    DF_EASY = 1,
};

enum BonusSpeed {
    BSSP_MEDIUM = 3,
};

enum PlatformSpeed {
    PSP_FAST = 13,
    PSP_FM = 11,
    PSP_MEDIUM = 10,
    PSP_MS = 8,
    PSP_SLOW = 6,
};

enum BallSpeed {
    BSP_FAST = 110,
    BSP_FM = 90,
    BSP_MEDIUM = 75,
    BSP_MS = 60,
    BSP_SLOW = 40,
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
    BONUS_FALL,
    HORIZONTAL_COLLISION,
    VERTICAL_COLLISION,
    COLLISION,
    TO_PAUSE,
    TO_GAME,
    SAVE,
    LOAD,
    FRAME,
    SCORE_UP,
    LIVES_DOWN,
    END_GAME,
    WIN,
    LOSE,
    START,
    BONUS,
    BONUS_CATCHED,
    PLATFORM_FASTEN_DECLINE,
    PLATFORM_SLOWEN_DECLINE,
    PLATFORM_SHORTEN_DECLINE,
    PLATFORM_LONGEN_DECLINE,
    BALL_FASTEN_DECLINE,
    BALL_SLOWEN_DECLINE,
    PLATFORM_FASTEN = 101,
    PLATFORM_SLOWEN = 102,
    PLATFORM_SHORTEN = 103,
    PLATFORM_LONGEN = 104,
    BALL_FASTEN = 105,
    BALL_SLOWEN = 106,
};

class DisplayObject;

struct Event {
    EventType type;
    DisplayObject* obj;
};

class SaveloadObject {
public:
    virtual void to_string(std::stringstream &strStream)=0;
    virtual SaveloadObject* from_string(std::stringstream &strStream)=0;
    virtual json to_json()=0;
    virtual SaveloadObject* from_json(json &deri)=0;
};

class DisplayObject : public SaveloadObject {
protected:
    sf::Shape* shape;
    sf::Color color;
    sf::FloatRect bounds;
    bool visible;
    sf::Vector2f position;
    DisplayObject(sf::Vector2f size, sf::Vector2f pos = sf::Vector2f(0, 0), sf::Color col = sf::Color(255, 255, 255)) {
        shape = new sf::RectangleShape(size);
        shape->setFillColor(col);
        color = col;
        shape->setPosition(pos);
        position = pos;
        bounds = shape->getGlobalBounds();
        visible = true;
        /*
        this->setFillColor(col);
        color = col;
        this->setPosition(pos);
        position = pos;
        bounds = this->getGlobalBounds();
        visible = true;
        */
    };
    DisplayObject(float size, sf::Vector2f pos = sf::Vector2f(0, 0), sf::Color col = sf::Color(255, 255, 255)) {
        shape = new sf::CircleShape(size);
        shape->setFillColor(col);
        color = col;
        shape->setPosition(pos);
        position = pos;
        bounds = shape->getGlobalBounds();
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
    virtual void scaleBound(sf::Vector2f koef) { 
        shape->setPosition(sf::Vector2f(bounds.left * koef.x, bounds.top * koef.y));
        shape->setScale(koef);
        bounds = shape->getGlobalBounds();
        position = shape->getPosition(); 
    }
    virtual void to_string(std::stringstream &strStream) override {}
    virtual SaveloadObject* from_string(std::stringstream &strStream) override { return nullptr; }
    virtual json to_json() override { return json{}; }
    virtual SaveloadObject* from_json(json &deri) override { return nullptr; }
    virtual void scale(float k) { shape->scale(sf::Vector2f(k, 1)); bounds = shape->getGlobalBounds(); position = shape->getPosition(); }
};

class MovableObject : public DisplayObject {
protected:
    sf::Vector2f velocity, base_vel;
    float scale_coef = 1;
    MovableObject(sf::Vector2f size, sf::Vector2f pos = sf::Vector2f(0, 0), sf::Color col = sf::Color(255,255,255), sf::Vector2f vel = sf::Vector2f(0, 0)) : DisplayObject(size, pos, col) {
        velocity = vel;
        base_vel = sf::Vector2f(abs(vel.x), abs(vel.y));
    }
    MovableObject(float size, sf::Vector2f pos = sf::Vector2f(0, 0), sf::Color col = sf::Color(255,255,255), sf::Vector2f vel = sf::Vector2f(0, 0)) : DisplayObject(size, pos, col) {
        velocity = vel;
        base_vel = sf::Vector2f(abs(vel.x), abs(vel.y));
    }
public:
    virtual void move();
    virtual void move(sf::Vector2f vel);
    virtual void setVelocity(sf::Vector2f vel);
    virtual sf::Vector2f getVelocity();
    virtual void setScale() { velocity *= scale_coef; }
    virtual void scaleSpeed(float k) { scale_coef *= k; velocity = sf::Vector2f(velocity.x / abs(velocity.x) * base_vel.x, velocity.y / abs(velocity.y) * base_vel.y); }
};

class Statistics : public SaveloadObject {
private:
    int lives, score, catched;
    float delay;
    std::string name;
    sf::Clock* clock;
public:
    Statistics(int l, int s, int c, std::string n, float delay);
    int getCatched() { return catched; }
    int getLives();
    int getScore();
    std::string getName();
    float getTime();
    sf::Clock* getClock() { return clock; };
    void setCatched(int c) { catched = c; }
    void setLives(int num);
    void setScore(int score);
    void setDelay(float d) { delay += d; }
    void to_string(std::stringstream &strStream) override;
    SaveloadObject* from_string(std::stringstream &strStream) override;
    json to_json() override;
    SaveloadObject* from_json(json &deri) override;
};

class TextBlock : public DisplayObject {
private:
    sf::Text *text;
public:
    TextBlock(sf::Vector2f size, sf::Vector2f pos, sf::Color col, std::string title);
    void draw(sf::RenderWindow &target) override;
    void setText(std::string str);
};

class Ball : public MovableObject {
public:
    Ball(float size, sf::Vector2f pos = sf::Vector2f(0, 0), sf::Color col = sf::Color(255,255,255), sf::Vector2f vel = sf::Vector2f(0, 0)) : MovableObject(size, pos, col, vel) {};
    std::pair <Ball*, Ball*> mitosis();
    void eventHandler(Event e) override;
    void to_string(std::stringstream &strStream) override;
    SaveloadObject* from_string(std::stringstream &strStream) override;
    json to_json() override;
    SaveloadObject* from_json(json &deri) override;
    int getBaseSpeedAbs(Difficulty diff);
};

class Platform : public MovableObject {
public:
    Platform(sf::Vector2f size, sf::Vector2f pos = sf::Vector2f(0, 0), sf::Color col = sf::Color(255,255,255), float vel = 0) : MovableObject(size, pos, col, sf::Vector2f (vel, 0)) {};
    void eventHandler(Event e) override;
    void to_string(std::stringstream &strStream) override;
    SaveloadObject* from_string(std::stringstream &strStream) override;
    json to_json() override;
    SaveloadObject* from_json(json &deri) override;
    float getBaseSpeedAbs();
};

class Bonus : public MovableObject {
private:
    EventType bonus;
public:
    Bonus(sf::Vector2f size, sf::Vector2f pos, float vel);
    void setBonus(EventType e);
    EventType getBonus() { return bonus; }
    void checkCollision(DisplayObject* obj) override;
    void checkBounds() override;
    void eventHandler(Event e) override;
    void to_string(std::stringstream &strStream) override;
    SaveloadObject* from_string(std::stringstream &strStream) override;
    json to_json() override;
    SaveloadObject* from_json(json &deri) override;
};

class Obstacle : public DisplayObject {
private:
    std::vector<Bonus*> bonuses;
public:
    Obstacle(sf::Vector2f size, sf::Vector2f pos, sf::Color col);
    void eventHandler(Event e) override;
    void to_string(std::stringstream &strStream) override;
    SaveloadObject* from_string(std::stringstream &strStream) override;
    json to_json() override;
    SaveloadObject* from_json(json &deri) override;
    std::vector<Bonus*> getBonuses() { return bonuses; }
    void addBonus(Bonus* bonus) { bonuses.push_back(bonus); }
    void clearBonuses() { bonuses.clear(); }
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
    Button* button;
    TextBlock* text;
public:
    MessageBox(EventType event, std::string str, sf::Vector2f size);
    void draw(sf::RenderWindow &target) override;
    void setText(std::string str);
    void update(sf::Vector2i mousePos, bool pressed);
};

class Settings : public SaveloadObject {
private:
    static Difficulty difficulty;
    static std::pair <Resolution, Resolution> resolution;
public:
    Settings(); 
    static Difficulty getDiff();
    void setDiff(Difficulty diff);
    static std::pair<Resolution, Resolution> getResolution();
    void setResolution(std::string str);
    void setResolution(std::pair<Resolution, Resolution> res) { resolution = res; }
    static std::string getDiffStr();
    static std::string getResolutionStr();
    void to_string(std::stringstream &strStream) override;
    SaveloadObject* from_string(std::stringstream &strStream) override;
    json to_json() override;
    SaveloadObject* from_json(json &deri) override;
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
    std::map<sf::Clock*, std::pair<float, EventType>> bonus_timers;
    std::vector<DisplayObject*> objects;
    std::vector<MovableObject*> move_objects;
    std::vector<Ball*> balls;
    std::vector<Platform*> platforms;
    std::vector<Bonus*> bonuses;
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
    void addTimer(std::pair<float, EventType> data) { bonus_timers[new sf::Clock()] = data; }
    Statistics* getData();
    void freezeTimers();
    void startTimers();
    void update(sf::Vector2i mousePos, bool pressed);
    std::vector<DisplayObject*> getObjects();
    void to_string(std::stringstream &strStream) override;
    SaveloadObject* from_string(std::stringstream &strStream) override;
    json to_json() override;
    SaveloadObject* from_json(json &deri) override;
};

class Player : public SaveloadObject {
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
    void to_string(std::stringstream &strStream) override;
    SaveloadObject* from_string(std::stringstream &strStream) override;
    json to_json() override;
    SaveloadObject* from_json(json &deri) override;
};

class Players : public SaveloadObject {
private:
    std::vector <Player*> players;
public:
    Players();
    void addPlayer(Player *player);
    std::vector <Player*> getPlayers();
    void to_string(std::stringstream &strStream) override;
    SaveloadObject* from_string(std::stringstream &strStream) override;
    json to_json() override;
    SaveloadObject* from_json(json &deri) override;
};

class Game;

class Proxy {
public:
    std::string to_string(std::vector <SaveloadObject*> &toSave);
    json to_json(std::vector <SaveloadObject*> &toSave);
    void from_string(std::vector <SaveloadObject*> &toLoad, std::stringstream &strStream);
    void from_json(std::vector <SaveloadObject*> &toLoad, json &deri);
};

class Game {
    friend class Proxy;
private:
    std::ifstream inFile;
    std::ofstream outFile;
    std::vector <SaveloadObject*> toSave;
    sf::Clock timer;
    Active state;
    Proxy* history;
    sf::RenderWindow *window;
    Players *sessionPlayers;
    Menu /** *menu ,*/ *settingsMenu, *pauseMenu; // Change pausenames if all works
    MessageBox* start, *win, *lose;
    Settings *settings;
    GameField *gameField;
    void update();
    void eventHandler(Event e);
    void initMenus();
    void reinit();
    void load();
    void load_json();
    void save(std::vector <SaveloadObject*> toSave);
    void save_json(std::vector <SaveloadObject*> toSave);
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