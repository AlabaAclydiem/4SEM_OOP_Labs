#include <SFML/Graphics.hpp>
#include <nlohmann/json.hpp>
#include <sys/stat.h>
#include "classes.hpp"

using json = nlohmann::json;

void DisplayObject::draw(sf::RenderWindow &target) {
    if (visible) target.draw(*this);
}

void DisplayObject::setColor(sf::Color col) {
    this->setFillColor(col);
    color = col;
}

void DisplayObject::setVisible(bool state) {
    visible = state;
}

bool DisplayObject::isVisible() {
    return visible;
}

void DisplayObject::eventHandler(Event e) {};

void DisplayObject::checkBounds() {
    int w = Settings::getResolution().first, h = Settings::getResolution().second;
    if (bounds.left < 0 || bounds.left + bounds.width > w) EventDispatcher::setGameEvent({EventType::VERTICAL_COLLISION, this});
    if (bounds.top + bounds.height > h) EventDispatcher::setGameEvent({EventType::FALL, this});
}

void DisplayObject::checkCollision(DisplayObject* obj) {
    sf::FloatRect objBounds = obj->getBound();
    float horizontalIntersect = -1, verticalIntersect = -1;
    std::vector <std::pair<float, bool> > horI;
    horI.push_back({bounds.left, 1});
    horI.push_back({bounds.left + bounds.width, 1});
    horI.push_back({objBounds.left, 0});
    horI.push_back({objBounds.left + objBounds.width, 0});
    sort(horI.begin(), horI.end());
    if (horI[0].second != horI[1].second) horizontalIntersect = horI[2].first - horI[1].first;
    std::vector <std::pair<float, bool> > verI;
    verI.push_back({bounds.top, 1});
    verI.push_back({bounds.top + bounds.height, 1});
    verI.push_back({objBounds.top, 0});
    verI.push_back({objBounds.top + objBounds.height, 0});
    sort(verI.begin(), verI.end());
    if (verI[0].second != verI[1].second) verticalIntersect = verI[2].first - verI[1].first;
    if (horizontalIntersect != -1 && verticalIntersect != -1) {
        if (horizontalIntersect >= verticalIntersect) {
            EventDispatcher::setGameEvent({EventType::HORIZONTAL_COLLISION, this}); 
        } else {
            EventDispatcher::setGameEvent({EventType::VERTICAL_COLLISION, this});
        }
        EventDispatcher::setGameEvent({EventType::COLLISION, obj}); 
    }
}

sf::FloatRect DisplayObject::getBound() { 
    return bounds; 
}

void MovableObject::move() {
    if (!this->isVisible()) return;
    ((sf::RectangleShape*)this)->move(velocity);
    bounds = this->getGlobalBounds();
    position = this->getPosition();
}

void MovableObject::move(sf::Vector2f vel) {
    ((sf::RectangleShape*)this)->move(vel);
    bounds = this->getGlobalBounds();
    position = this->getPosition();
}

void MovableObject::setVelocity(sf::Vector2f vel) {
    velocity = vel;
}

sf::Vector2f MovableObject::getVelocity() {
    return velocity;
}

Statistics::Statistics(int l, int s, sf::Time t, std::string n, float d = 0) {
    lives = l;
    score = s;
    name = n;
    delay = d;
}

int Statistics::getLives() { 
    return lives; 
}

int Statistics::getScore() { 
    return score; 
}

float Statistics::getTime() {
    return clock.getElapsedTime().asSeconds() + delay;
}

std::string Statistics::getName() {
    return name;
}

void Statistics::setLives(int num) { 
    lives = num;
}

void Statistics::setScore(int num) { 
    score = num;
}

TextBlock::TextBlock(sf::Vector2f size, sf::Vector2f pos, sf::Color col, std::string title) : DisplayObject(size, pos, col) {
    sf::Font *font = new sf::Font();
    font->loadFromFile("Roboto-Light.ttf");
    text = new sf::Text(title, *font);
    text->setPosition(pos);
    switch (Settings::getResolution().first) {
        case Resolution::WIDTH_BIG:
            text->setCharacterSize(30);
            break;
        case Resolution::WIDTH_MEDIUM:
            text->setCharacterSize(20);
            break;
        case Resolution::WIDTH_SMALL:
            text->setCharacterSize(16);
            break;
    }
}

void TextBlock::setText(std::string str) {
    text->setString(str);
}

void TextBlock::draw(sf::RenderWindow &target) {
    target.draw(*this);
    target.draw(*text);
}

void Platform::eventHandler(Event e) {
    float platformWidth = Settings::getResolution().first * PlatformSize::PS_MEDIUM / 1000;
    float platformHeight = Settings::getResolution().second * PlatformSize::PS_HEIGHT / 1000;
    switch (e.type) {
    case EventType::FALL:
        setPosition(sf::Vector2f(
            (Settings::getResolution().first - platformWidth) / 2,
            Settings::getResolution().second - platformHeight
        ));
        move(sf::Vector2f(0, 0));
        break;
    case EventType::VERTICAL_COLLISION:
        if (e.obj != this) return;
        srand(rand() * time(NULL));
        velocity = -velocity;
        move(velocity);
        break;
    }   
}

void Ball::eventHandler(Event e) {
    if (e.obj != this) return;
    float ballSpeed;
    float ballSize = Settings::getResolution().second * BallSize::BS_MEDIUM / 1000;
    switch (e.type) {
    case EventType::FALL:
        setPosition(sf::Vector2f(
            (Settings::getResolution().first - ballSize) / 2,
            (Settings::getResolution().second - ballSize) / 4 * 3
        ));
        move(sf::Vector2f(0, 0));
        EventDispatcher::setGameEvent({EventType::LIVES_DOWN, nullptr});
        break;
    case EventType::VERTICAL_COLLISION:
        srand(rand() * time(NULL));
        move(-velocity);
        switch (Settings::getResolution().first) {
        case Resolution::WIDTH_BIG:
            ballSpeed = (float)BallSpeed::BSP_FAST;
            break;
        case Resolution::WIDTH_MEDIUM:
            ballSpeed = (float)BallSpeed::BSP_MEDIUM;
            break;
        case Resolution::WIDTH_SMALL:
            ballSpeed = (float)BallSpeed::BSP_SLOW;
            break;
        }
        velocity.y /= fabs(velocity.y);
        velocity.x /= fabs(velocity.x);
        velocity.x = -velocity.x;
        velocity.y *= ((float)rand() / RAND_MAX * 0.3 + 0.5) * sqrt(ballSpeed);
        velocity.x *= sqrt(ballSpeed - velocity.y * velocity.y);
        break;
    case EventType::HORIZONTAL_COLLISION:
        srand(rand() * time(NULL));
        move(-velocity);
        switch (Settings::getResolution().first) {
        case Resolution::WIDTH_BIG:
            ballSpeed = (float)BallSpeed::BSP_FAST;
            break;
        case Resolution::WIDTH_MEDIUM:
            ballSpeed = (float)BallSpeed::BSP_MEDIUM;
            break;
        case Resolution::WIDTH_SMALL:
            ballSpeed = (float)BallSpeed::BSP_SLOW;
            break;
        }
        velocity.y /= fabs(velocity.y);
        velocity.y = -velocity.y;
        velocity.x /= fabs(velocity.x);
        velocity.y *= ((float)rand() / RAND_MAX * 0.3 + 0.5) * sqrt(ballSpeed);
        velocity.x *= sqrt(ballSpeed - velocity.y * velocity.y);
        break;
    }
}

std::pair<Ball*, Ball*> Ball::mitosis() { 
    std::pair<Ball*, Ball*> temp;
    float ballSize = Settings::getResolution().second * BallSize::BS_MEDIUM / 1000;
    temp.first = new Ball(
        ballSize,
        sf::Vector2f(
            bounds.left,
            bounds.top - ballSize
        ),
        sf::Color::Cyan,
        sf::Vector2f(velocity.x, -velocity.y)
    );
    temp.second = new Ball(
        ballSize,
        sf::Vector2f(
            bounds.left - ballSize,
            bounds.top
        ),
        sf::Color::Cyan,
        sf::Vector2f(-velocity.x, velocity.y)
    );
    return temp;
}

Bonus::Bonus() {}

void Bonus::sendEvent() {}

Obstacle::Obstacle(sf::Vector2f size, sf::Vector2f pos, sf::Color col) : DisplayObject(size, pos, col) {}

void Obstacle::eventHandler(Event e) {
    if (e.obj != this) return;
    switch (e.type) {
    case EventType::COLLISION:
        visible = false;
        EventDispatcher::setGameEvent({EventType::SCORE_UP, nullptr});
        break;
    }
}

Button::Button(sf::Vector2f size, sf::Vector2f pos, sf::Color col, std::string title, EventType e) : DisplayObject(size, pos, col) {
    text = new TextBlock(size, pos, col, title);
    event = e;
};

void Button::setText(std::string str) {
    text->setText(str);
}

bool Button::underMouse(int mouseX, int mouseY) {
    return (bounds.left <= mouseX && mouseX <= bounds.left + bounds.width) 
            && (bounds.top <= mouseY && mouseY <= bounds.top + bounds.height);
}

void Button::draw(sf::RenderWindow &target) {
    target.draw(*this);
    text->draw(target);
}

void Button::sendEvent() {
    EventDispatcher::setEvent({event, nullptr});
}

void Button::setColor(sf::Color col) {
    text->setColor(col);
}

StatusBar::StatusBar(sf::Vector2f size, Statistics* stats) : DisplayObject(size, sf::Vector2f(0, 0), sf::Color::Black) {
    menu = new Button(
        sf::Vector2f(Settings::getResolution().first / 15, Settings::getResolution().second / 20),
        sf::Vector2f(0, 0),
        sf::Color::Blue,
        "Pause",
        EventType::TO_PAUSE
    );

    bar.push_back(new TextBlock(
        sf::Vector2f(Settings::getResolution().first / 10, Settings::getResolution().second / 20),
        sf::Vector2f(Settings::getResolution().first / 15, 0),
        sf::Color::Black,
        "Lives: " + std::to_string(stats->getLives())
    ));
    bar.push_back(new TextBlock(
        sf::Vector2f(Settings::getResolution().first / 15, Settings::getResolution().second / 20),
        sf::Vector2f(Settings::getResolution().first / 15 + (Settings::getResolution().first / 20 + Settings::getResolution().first / 10), 0),
        sf::Color::Black,
        "Score: " + std::to_string(stats->getScore())
    ));
    bar.push_back(new TextBlock(
        sf::Vector2f(Settings::getResolution().first / 10, Settings::getResolution().second / 20),
        sf::Vector2f(Settings::getResolution().first / 15 + (Settings::getResolution().first / 20 + Settings::getResolution().first / 10) * 2, 0),
        sf::Color::Black,
        "Time: " + std::to_string(stats->getTime())
    ));
    bar.push_back(new TextBlock(
        sf::Vector2f(Settings::getResolution().first / 10, Settings::getResolution().second / 20),
        sf::Vector2f(Settings::getResolution().first / 15 + (Settings::getResolution().first / 20 + Settings::getResolution().first / 10) * 4, 0),
        sf::Color::Black,
        "Name: " + stats->getName()
    ));
};

void StatusBar::update(Statistics* stats, sf::Vector2i mousePos, bool pressed) {
    menu->setColor(sf::Color::Blue);
    if (menu->underMouse(mousePos.x, mousePos.y)) {
        menu->setColor(sf::Color::Cyan);
        if (pressed) menu->sendEvent();
    }
    bar[0]->setText("Lives: " + std::to_string(stats->getLives()));
    bar[1]->setText("Score: " + std::to_string(stats->getScore()));
    bar[2]->setText("Time: " + std::to_string(stats->getTime()));
    bar[3]->setText("Name: " + stats->getName());
}

void StatusBar::draw(sf::RenderWindow &target) {
    target.draw(*this);
    menu->draw(target);
    for (TextBlock* text : bar) {
        text->draw(target);
    }
}

Menu::Menu(sf::Vector2f size, sf::Color col, std::vector<Button*> buttons, std::string title) : DisplayObject(size, sf::Vector2f(0, 0), col) {
    items = buttons;
    text = new TextBlock(
        sf::Vector2f(Settings::getResolution().first / 10, Settings::getResolution().second / 30), 
        sf::Vector2f((Settings::getResolution().first - Settings::getResolution().first / 10) / 2, 0), 
        col,
        title
    );
}

Button* Menu::getButton(int index) {
    return items[index];
}

void Menu::update(sf::Vector2i mousePos, bool pressed) {
    for (Button* button : items) {
        button->setColor(sf::Color::Blue);
    }
    for (Button* button : items) {
        if (button->underMouse(mousePos.x, mousePos.y)) {
            if (pressed) {
                button->sendEvent();
            } else {
                button->setColor(sf::Color::Cyan);
            }
        }
    }
}

void Menu::draw(sf::RenderWindow &target) {
    target.draw(*this);
    text->draw(target);
    for (Button* item : items) {
        item->draw(target);
    }
}

std::pair <Resolution, Resolution> Settings::resolution = {Resolution::WIDTH_BIG, Resolution::HEIGHT_BIG};
Difficulty Settings::difficulty = Difficulty::DF_MEDIUM;

Settings::Settings() {};

std::pair<Resolution, Resolution> Settings::getResolution() {
    return Settings::resolution;
}

Difficulty Settings::getDiff() {
    return Settings::difficulty;
}

void Settings::setDiff(Difficulty diff) {
    Settings::difficulty = diff;
}

void Settings::setResolution(std::string str) {
    if (str == "BIG") {
        Settings::resolution = {Resolution::WIDTH_BIG, Resolution::HEIGHT_BIG};
    } else if (str == "MEDIUM") {
        Settings::resolution = {Resolution::WIDTH_MEDIUM, Resolution::HEIGHT_MEDIUM};
    } else if (str == "SMALL") {
        Settings::resolution = {Resolution::WIDTH_SMALL, Resolution::HEIGHT_SMALL};
    }
}

std::string Settings::getDiffStr() {
    switch (Settings::difficulty) {
    case Difficulty::DF_EASY:
        return "Difficulty: Easy";
        break;
    case Difficulty::DF_MEDIUM:
        return "Difficulty: Medium";
        break;
    case Difficulty::DF_HARD:
        return "Difficulty: Hard";
        break;
    }
    return "";
}

std::string Settings::getResolutionStr() {
    switch (Settings::resolution.first) {
    case Resolution::WIDTH_SMALL:
        return "Resolution: Small";
        break;
    case Resolution::WIDTH_MEDIUM:
        return "Resolution: Medium";
        break;
    case Resolution::WIDTH_BIG:
        return "Resolution: Big";
        break;
    }
    return "";
}

GameField::GameField() : DisplayObject(sf::Vector2f(Settings::getResolution().first, Settings::getResolution().second), sf::Vector2f(0, 0), sf::Color::Black) {}

std::vector<DisplayObject*> GameField::getObjects() {
    return objects;
}

void GameField::draw(sf::RenderWindow &target) {
    target.draw(*this);
    for (DisplayObject* obj : objects) {
        obj->draw(target);
    }
}

void GameField::addItem(DisplayObject *obj) {
    objects.push_back(obj);
}

void GameField::addItem(Ball *obj) {
    move_objects.push_back(obj);
    objects.push_back(obj);
    balls.push_back(obj);
}

void GameField::addItem(Platform *obj) {
    move_objects.push_back(obj);
    objects.push_back(obj);
    platforms.push_back(obj);
}

void GameField::addItem(Statistics *obj) {
    data = obj;
}

void GameField::addItem(StatusBar *obj) {
    board = obj;
    objects.push_back(obj);
}

Statistics* GameField::getData() {
    return data;
}

void GameField::eventHandler(Event e) {
    int aliveObj = 0, aliveMoveObj = 0;
    switch (e.type) {
    case EventType::LIVES_DOWN:
        data->setLives(data->getLives() - 1);
        if (data->getLives() == 0) {
            EventDispatcher::setEvent({EventType::TO_MENU, nullptr});
        }
        break;
    case EventType::SCORE_UP:
        data->setScore(data->getScore() + 10);
        break;
    case EventType::COLLISION:
        for (DisplayObject* obj : objects) {
            if (obj->isVisible()) aliveObj++;
        }
        for (MovableObject* obj : move_objects) {
            if (obj->isVisible()) aliveMoveObj++;
        }
        if (aliveObj - 1 == aliveMoveObj) {
            EventDispatcher::setEvent({EventType::TO_MENU, nullptr});
        }
        break;
    }
}

void GameField::moveObjects() {
    for (Ball* ball : balls) {
        ball->move();
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
        for (Platform* platform : platforms) {
            platform->setVelocity(sf::Vector2f(-PlatformSpeed::PSP_MEDIUM, 0));
            platform->move();
        }
    } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
        for (Platform* platform : platforms) {
            platform->setVelocity(sf::Vector2f(PlatformSpeed::PSP_MEDIUM, 0));
            platform->move();
        }
    }
}

void GameField::checkCollisions() {
    for (MovableObject* obj1: move_objects) {
        for (DisplayObject* obj2: objects) {
            if (obj1 == obj2 
                || !obj1->isVisible() 
                || !obj2->isVisible() 
                || std::find(balls.begin(), balls.end(), obj2) != balls.end()) continue;
            obj1->checkCollision(obj2);
        }
        if (obj1->isVisible()) obj1->checkBounds();
    }
}

void GameField::update(sf::Vector2i mousePos, bool pressed) {
    moveObjects();
    checkCollisions();
    Event e;
    while (EventDispatcher::pollGameEvent(e)) {
        for (DisplayObject* obj : objects) {
            obj->eventHandler(e);
        }
        eventHandler(e);
    }
    board->update(data, mousePos, pressed);
}

Player::Player(Statistics* s, Platform* p, std::vector <Ball*> b) {
    stats = s;
    platform = p;
    balls = b;
}

Player::Player(std::string name) {
    float platformWidth = Settings::getResolution().first * PlatformSize::PS_MEDIUM / 1000;
    float platformHeight = Settings::getResolution().second * PlatformSize::PS_HEIGHT / 1000;
    platform = new Platform(
        sf::Vector2f(
            platformWidth,
            platformHeight
        ), 
        sf::Vector2f(
            (Settings::getResolution().first - platformWidth) / 2,
            Settings::getResolution().second - platformHeight
        ),
        sf::Color::Blue,
        PlatformSpeed::PSP_MEDIUM
    );
    float ballSize = Settings::getResolution().second * BallSize::BS_MEDIUM / 1000;
    float ballSpeed;
    switch (Settings::getResolution().first) {
    case Resolution::WIDTH_BIG:
        ballSpeed = (float)BallSpeed::BSP_FAST;
        break;
    case Resolution::WIDTH_MEDIUM:
        ballSpeed = (float)BallSpeed::BSP_MEDIUM;
        break;
    case Resolution::WIDTH_SMALL:
        ballSpeed = (float)BallSpeed::BSP_SLOW;
        break;
    }
    Ball *ball = new Ball(
        ballSize,
        sf::Vector2f(
            (Settings::getResolution().first - ballSize) / 2,
            (Settings::getResolution().second - ballSize) / 4 * 3
        ),
        sf::Color::Cyan,
        sf::Vector2f(sqrt(ballSpeed / 2.0), sqrt(ballSpeed / 2.0))
    );
    balls.push_back(ball);
    stats = new Statistics(3, 0, sf::seconds(0), name);
}

Platform* Player::getPlatform() {
    return platform;
}

std::vector <Ball*> Player::getBalls() {
    return balls;
}

Statistics* Player::getStatistics() {
    return stats;
} 

Players::Players() {}

void Players::addPlayer(Player *player){
    players.push_back(player);
}

std::vector<Player*> Players::getPlayers() { 
    return players; 
}

std::queue<Event> EventDispatcher::eventQueue, EventDispatcher::gameEventQueue;

void EventDispatcher::setEvent(Event e) {
    EventDispatcher::eventQueue.push(e);
}

void EventDispatcher::setGameEvent(Event e) {
    EventDispatcher::gameEventQueue.push(e);
}

bool EventDispatcher::pollEvent(Event &e) {
    if (EventDispatcher::eventQueue.empty()) return false;
    e = EventDispatcher::eventQueue.front();
    EventDispatcher::eventQueue.pop();
    return true;
}

bool EventDispatcher::pollGameEvent(Event &e) {
    if (EventDispatcher::gameEventQueue.empty()) return false;
    e = EventDispatcher::gameEventQueue.front();
    EventDispatcher::gameEventQueue.pop();
    return true;
}

Game::Game() {}

void Game::eventHandler(Event e) {
    switch(e.type) {
        case EventType::FRAME:
            update();
            timer.restart();
            break;
        case EventType::SAVE:
            history->save(this);
            break;
        case EventType::TO_GAME:
            state = Active::GAME;
            break;
        case EventType::TO_PAUSE:
            state = Active::PAUSE;
            break;
        case EventType::TO_MENU:
            state = Active::MENU;
            break;
        case EventType::CONTINUE:
            state = Active::GAME;
            history->load(this);
            window->close();
            create();
            break;
        case EventType::TO_SETTINGS:
            state = Active::SETTINGS;
            break;
        case EventType::NEW_GAME:
            init();
            state = Active::GAME;
            break;
        case EventType::QUIT:
            window->close();
            break;
        case EventType::SWITCH_DIFFICULTY:
            {
            Difficulty temp = Settings::getDiff();
            switch (temp) {
            case Difficulty::DF_EASY:
                settings->setDiff(DF_MEDIUM);   
                break;
            case Difficulty::DF_MEDIUM:
                settings->setDiff(DF_HARD);
                break;
            case Difficulty::DF_HARD:
                settings->setDiff(DF_EASY);
                break;
            }
            }
            init();
            state = Active::SETTINGS;
            break;
        case EventType::SWITCH_RESOLUTION:
            {
            Resolution width = Settings::getResolution().first;
            switch (width) {
            case Resolution::WIDTH_BIG:
                settings->setResolution("SMALL");
                break;
            case Resolution::WIDTH_MEDIUM:
                settings->setResolution("BIG");
                break;
            case Resolution::WIDTH_SMALL:
                settings->setResolution("MEDIUM");
                break;
            }
            window->close();
            create();
            init();
            state = Active::SETTINGS;
            break;
            }
        }
}

void Game::update() {
    sf::Event e;
    bool pressed = false; 
    while (window->pollEvent(e)) {
        if (e.type == sf::Event::Closed) {
            window->close();
        }
        if (e.type == sf::Event::MouseButtonReleased) {
            pressed = true;
        }
    }
    Event ev;
    while (EventDispatcher::pollEvent(ev)) {
        eventHandler(ev);
    }
    window->clear();
    sf::Vector2i mousePos = sf::Mouse::getPosition(*window);
    switch (state) {
        case Active::MENU:
            menu->update(mousePos, pressed);
            menu->draw(*window);
            break;
        case Active::SETTINGS:
            settingsMenu->update(mousePos, pressed);
            settingsMenu->draw(*window);
            break;
        case Active::GAME:
            gameField->update(mousePos, pressed);
            gameField->draw(*window);
            break;
        case Active::PAUSE:
            pauseMenu->update(mousePos, pressed);
            pauseMenu->draw(*window);
            break;
    }
    window->display();
}

void Game::create() {
    window = new sf::RenderWindow(
        sf::VideoMode(Settings::getResolution().first, Settings::getResolution().second), 
        "Arcanoid",
        sf::Style::Titlebar | sf::Style::Close
    );
    window->setKeyRepeatEnabled(false);
}

void Game::initMenus() {
    sf::Vector2f fullResolution = sf::Vector2f(Settings::getResolution().first, Settings::getResolution().second);
    sf::Vector2f buttonSize = sf::Vector2f(fullResolution.x / 6, fullResolution.y / 20);
    float xPos = (fullResolution.x - buttonSize.x) / 2;
    float yPos = fullResolution.y / 10;
    std::string diffStr = Settings::getDiffStr();
    std::string resolutionStr = Settings::getResolutionStr();

    std::vector<Button*> menu_buttons;
    menu_buttons.push_back(new Button(buttonSize, sf::Vector2f(xPos, yPos), sf::Color::Blue, "New Game", EventType::NEW_GAME));
    menu_buttons.push_back(new Button(buttonSize, sf::Vector2f(xPos, yPos * 2), sf::Color::Blue, "Continue", EventType::CONTINUE));
    menu_buttons.push_back(new Button(buttonSize, sf::Vector2f(xPos, yPos * 3), sf::Color::Blue, "Settings", EventType::TO_SETTINGS));
    menu_buttons.push_back(new Button(buttonSize, sf::Vector2f(xPos, yPos * 4), sf::Color::Blue, "Quit", EventType::QUIT));
    menu = new Menu(fullResolution, sf::Color::Black, menu_buttons, "Arcanoid");

    std::vector<Button*> settings_buttons;
    settings_buttons.push_back(new Button(buttonSize, sf::Vector2f(xPos, yPos), sf::Color::Blue, diffStr, EventType::SWITCH_DIFFICULTY));
    settings_buttons.push_back(new Button(buttonSize, sf::Vector2f(xPos, yPos * 2), sf::Color::Blue, resolutionStr, EventType::SWITCH_RESOLUTION));
    settings_buttons.push_back(new Button(buttonSize, sf::Vector2f(xPos, yPos * 3), sf::Color::Blue, "Return to menu", EventType::TO_MENU));
    settingsMenu = new Menu(fullResolution, sf::Color::Black, settings_buttons, "Settings");

    std::vector<Button*> pause_buttons;
    pause_buttons.push_back(new Button(buttonSize, sf::Vector2f(xPos, yPos), sf::Color::Blue, "Continue", EventType::TO_GAME));
    pause_buttons.push_back(new Button(buttonSize, sf::Vector2f(xPos, yPos * 2), sf::Color::Blue, "Restart", EventType::NEW_GAME));
    pause_buttons.push_back(new Button(buttonSize, sf::Vector2f(xPos, yPos * 3), sf::Color::Blue, "Save", EventType::SAVE));
    pause_buttons.push_back(new Button(buttonSize, sf::Vector2f(xPos, yPos * 4), sf::Color::Blue, "Load", EventType::CONTINUE));
    pause_buttons.push_back(new Button(buttonSize, sf::Vector2f(xPos, yPos * 5), sf::Color::Blue, "Return to menu", EventType::TO_MENU));
    pauseMenu = new Menu(sf::Vector2f(Settings::getResolution().first, Settings::getResolution().second), sf::Color::Black, pause_buttons, "Pause");
}

void Game::init() {
    state = Active::MENU;
    
    settings = new Settings();
    history = new History();

    sessionPlayers = new Players();
    sessionPlayers->addPlayer(new Player("Artur"));

    initMenus();
    
    gameField = new GameField();
    for (Player* player : sessionPlayers->getPlayers()) {
        gameField->addItem((Platform*)player->getPlatform());
        for (Ball* ball : player->getBalls()) {
             gameField->addItem((Ball*)ball);
        }
        gameField->addItem((Statistics*)player->getStatistics());
    }

    std::vector <Obstacle*> blocks;
    sf::Vector2f fullResolution = sf::Vector2f(Settings::getResolution().first, Settings::getResolution().second);
    int rowNum = ObstacleNum::OB_ROW / (4 - Settings::getDiff());
    int columnNum = ObstacleNum::OB_COLUMN / (4 - Settings::getDiff());
    float gapWidth = (float)fullResolution.x / columnNum / 20;
    float gapHeight = (float)(fullResolution.y - fullResolution.y / 20) / 2 / rowNum / 10;
    float obstacleWidth = (float)fullResolution.x / columnNum - gapWidth * 2;
    float obstacleHeight = (float)(fullResolution.y - fullResolution.y / 20) / 2 / rowNum - gapHeight * 2;
    sf::Vector2f obstacleSize = sf::Vector2f(obstacleWidth, obstacleHeight);
    for (int h = 0; h < rowNum; ++h) {
        for (int w = 0; w < columnNum; ++w) {
            blocks.push_back(new Obstacle(
                obstacleSize,
                sf::Vector2f((gapWidth + obstacleWidth) * w + gapWidth * (w + 1), (gapHeight + obstacleHeight) * h + gapHeight * (h + 1) + fullResolution.y / 20),
                sf::Color::Yellow
            ));
        }
    }
    for (Obstacle* block : blocks) {
        gameField->addItem((DisplayObject*)block);
    }

    gameField->addItem(new StatusBar(
        sf::Vector2f(fullResolution.x, fullResolution.y / 20), 
        gameField->getData()
    ));
}

void Game::process() {
    int tick = 16;
    while (window->isOpen()) {
        if (timer.getElapsedTime().asMilliseconds() > tick) {
            eventHandler({EventType::FRAME, nullptr});
        }
    }
}

void History::save(Game* game) {
    std::set <DisplayObject*> usedObjectPointers;
    outFile.open("save.txt", std::ifstream::out);
    outFile << "Difficulty\n" << std::to_string(Settings::getDiff()) << '\n';
    outFile << "Resolution\n" << std::to_string(Settings::getResolution().first) << " " << std::to_string(Settings::getResolution().second) << '\n';
    for (Player* player : game->sessionPlayers->getPlayers()) {
        outFile << "Statistics\n" << std::to_string(player->getStatistics()->getLives()) << " " << std::to_string(player->getStatistics()->getScore()) << " " << std::to_string(player->getStatistics()->getTime()) << " " << player->getStatistics()->getName() << '\n';
        outFile << "Platform\n" << std::to_string(player->getPlatform()->getBound().left) << " " << std::to_string(player->getPlatform()->getBound().top) << " " << std::to_string(player->getPlatform()->getVelocity().x) << '\n';
        usedObjectPointers.insert(player->getPlatform());
        for (Ball* ball : player->getBalls()) {
            outFile << "Ball\n" << std::to_string(ball->getBound().left) << " " << std::to_string(ball->getBound().top) << " " << std::to_string(ball->getVelocity().x) << " " << std::to_string(ball->getVelocity().y) << '\n';
            usedObjectPointers.insert(ball);
        }
    }
    for (int i = 0; i < game->gameField->getObjects().size() - 1; i++) {
        DisplayObject* obj = game->gameField->getObjects()[i];
        if (usedObjectPointers.find(obj) == usedObjectPointers.end()) {
            outFile << "Obstacle\n" << std::to_string(obj->getBound().left) << " " << std::to_string(obj->getBound().top) << " " << std::to_string(obj->getBound().width) << " " << std::to_string(obj->getBound().height) << " " << std::to_string(obj->isVisible()) << '\n';
            usedObjectPointers.insert(obj);
        }
    }
    outFile.close();

    outFile.open("save.json", std::ifstream::out);
    usedObjectPointers.clear();
    json serialization{};
    serialization["difficulty"] = Settings::getDiff();
    serialization["resolution"]["width"] = Settings::getResolution().first;
    serialization["resolution"]["height"] = Settings::getResolution().second;
    for (int i = 0; i < game->sessionPlayers->getPlayers().size(); i++) {
        Player* player = game->sessionPlayers->getPlayers()[i];
        serialization["player"][i]["statistics"]["lives"] = player->getStatistics()->getLives();
        serialization["player"][i]["statistics"]["score"] = player->getStatistics()->getScore();
        serialization["player"][i]["statistics"]["time"] = player->getStatistics()->getTime();
        serialization["player"][i]["statistics"]["name"] = player->getStatistics()->getName();
        serialization["player"][i]["platform"]["x"] = player->getPlatform()->getBound().left;
        serialization["player"][i]["platform"]["y"] = player->getPlatform()->getBound().top;
        serialization["player"][i]["platform"]["velocity"] = player->getPlatform()->getVelocity().x;
        usedObjectPointers.insert(player->getPlatform());
        for (int j = 0; j < player->getBalls().size(); j++) {
            Ball* ball = player->getBalls()[j];
            serialization["player"][i]["ball"][j]["x"] = ball->getBound().left;
            serialization["player"][i]["ball"][j]["y"] = ball->getBound().top;
            serialization["player"][i]["ball"][j]["velocity_x"] = ball->getVelocity().x;
            serialization["player"][i]["ball"][j]["velocity_y"] = ball->getVelocity().y;
            usedObjectPointers.insert(ball);
        }
    }
    int j = 0;
    for (int i = 0; i < game->gameField->getObjects().size() - 1; i++) {
        DisplayObject* obj = game->gameField->getObjects()[i];
        if (usedObjectPointers.find(obj) == usedObjectPointers.end()) {
            serialization["obstacle"][j]["x"] = obj->getBound().left;
            serialization["obstacle"][j]["y"] = obj->getBound().top;
            serialization["obstacle"][j]["width"] = obj->getBound().width;
            serialization["obstacle"][j]["height"] = obj->getBound().height;
            serialization["obstacle"][j]["visible"] = obj->isVisible();
            j++;
            usedObjectPointers.insert(obj);
        }
    }
    outFile << std::setw(4) << serialization << '\n';
    outFile.close();
}

void History::load(Game* game) {
    std::string type = "json";
    if (type == "txt") {
        std::string filename = "save.txt";
        inFile.open("save.txt");
        struct stat stat_buf;
        int rc = stat(filename.c_str(), &stat_buf);
        if (stat_buf.st_size == 0) {
            game->init();
            game->state = Active::GAME;
            return;
        }

        std::string name, diff;
        inFile >> name >> diff;
        game->settings->setDiff((Difficulty)stoi(diff));
        std::string width, height;
        inFile >> name >> width >> height;
        if ((Resolution)stoi(width) == Resolution::WIDTH_BIG) game->settings->setResolution("BIG");
        if ((Resolution)stoi(width) == Resolution::WIDTH_MEDIUM) game->settings->setResolution("MEDIUM");
        if ((Resolution)stoi(width) == Resolution::WIDTH_SMALL) game->settings->setResolution("SMALL");

        std::string l, s, d, n;
        inFile >> name >> l >> s >> d >> n;
        Statistics* stats = new Statistics(stoi(l), stoi(s), sf::seconds(0), n, stof(d));
        std::string top, left, vel;
        inFile >> name >> top >> left >> vel;
        float platformWidth = Settings::getResolution().first * PlatformSize::PS_MEDIUM / 1000;
        float platformHeight = Settings::getResolution().second * PlatformSize::PS_HEIGHT / 1000;
        Platform* platform = new Platform(sf::Vector2f(platformWidth, platformHeight), sf::Vector2f(stoi(top), stoi(left)), sf::Color::Blue, stoi(vel));
        std::string velY;
        inFile >> name >> top >> left >> vel >> velY;
        float ballSize = Settings::getResolution().second * BallSize::BS_MEDIUM / 1000;
        Ball *ball = new Ball(ballSize, sf::Vector2f(stoi(top), stoi(left)), sf::Color::Cyan, sf::Vector2f(stof(vel), stof(velY)));
        std::vector <Ball*> balls(1, ball);
        game->sessionPlayers = new Players();
        game->sessionPlayers->addPlayer(new Player(stats, platform, balls));
        
        game->initMenus();

        game->gameField = new GameField();
        for (Player* player : game->sessionPlayers->getPlayers()) {
            game->gameField->addItem((Platform*)player->getPlatform());
            for (Ball* ball : player->getBalls()) {
                game->gameField->addItem((Ball*)ball);
            }
            game->gameField->addItem((Statistics*)player->getStatistics());
        }
        std::string vis;
        while (!inFile.eof()) {
            inFile >> name >> top >> left >> width >> height >> vis;
            Obstacle* block = new Obstacle(sf::Vector2f(stof(width), stof(height)), sf::Vector2f(stof(top), stof(left)), sf::Color::Yellow);
            block->setVisible(stoi(vis));
            game->gameField->addItem((DisplayObject*)block);
        }
        

        game->gameField->addItem(new StatusBar(
            sf::Vector2f(Settings::getResolution().first, Settings::getResolution().second / 20), 
            game->gameField->getData()
        ));
        inFile.close();
    } else {
        std::string filename = "save.json";
        inFile.open(filename);
        struct stat stat_buf;
        int rc = stat(filename.c_str(), &stat_buf);
        if (stat_buf.st_size == 0) {
            game->init();
            game->state = Active::GAME;
            return;

        }
        json deserialization{};
        std::stringstream strStream;
        strStream << inFile.rdbuf();
        std::string str = strStream.str();
        deserialization = json::parse(str);

        Difficulty diff;
        diff = (Difficulty)deserialization["difficulty"].get<int>();
        game->settings->setDiff(diff);
        Resolution width, height;
        width = (Resolution)deserialization["resolution"]["width"].get<int>();
        height = (Resolution)deserialization["resolution"]["height"].get<int>();
        if (width == Resolution::WIDTH_BIG) game->settings->setResolution("BIG");
        if (width == Resolution::WIDTH_MEDIUM) game->settings->setResolution("MEDIUM");
        if (width == Resolution::WIDTH_SMALL) game->settings->setResolution("SMALL");

        int l, s;
        float d;
        std::string n;
        l = deserialization["player"][0]["statistics"]["lives"].get<int>();
        s = deserialization["player"][0]["statistics"]["score"].get<int>();
        d = deserialization["player"][0]["statistics"]["time"].get<float>();
        n = deserialization["player"][0]["statistics"]["name"].get<std::string>();
        Statistics* stats = new Statistics(l, s, sf::seconds(0), n, d);

        float top, left, vel;
        top = deserialization["player"][0]["platform"]["x"].get<float>();
        left = deserialization["player"][0]["platform"]["y"].get<float>();
        vel = deserialization["player"][0]["platform"]["velocity"].get<float>();
        float platformWidth = Settings::getResolution().first * PlatformSize::PS_MEDIUM / 1000;
        float platformHeight = Settings::getResolution().second * PlatformSize::PS_HEIGHT / 1000;
        Platform* platform = new Platform(sf::Vector2f(platformWidth, platformHeight), sf::Vector2f(top, left), sf::Color::Blue, vel);

        float velY;
        top = deserialization["player"][0]["ball"][0]["x"].get<float>();
        left = deserialization["player"][0]["ball"][0]["y"].get<float>();
        vel = deserialization["player"][0]["ball"][0]["velocity_x"].get<float>();
        velY = deserialization["player"][0]["ball"][0]["velocity_y"].get<float>();
        float ballSize = Settings::getResolution().second * BallSize::BS_MEDIUM / 1000;
        Ball *ball = new Ball(ballSize, sf::Vector2f(top, left), sf::Color::Cyan, sf::Vector2f(vel, velY));
        std::vector <Ball*> balls(1, ball);

        game->sessionPlayers = new Players();
        game->sessionPlayers->addPlayer(new Player(stats, platform, balls));

        game->initMenus();

        game->gameField = new GameField();
        for (Player* player : game->sessionPlayers->getPlayers()) {
            game->gameField->addItem((Platform*)player->getPlatform());
            for (Ball* ball : player->getBalls()) {
                game->gameField->addItem((Ball*)ball);
            }
            game->gameField->addItem((Statistics*)player->getStatistics());
        }
        bool vis;
        float wi, he;
        for (int i = 0; i < deserialization["obstacle"].size(); i++) {
            top = deserialization["obstacle"][i]["x"].get<float>();
            left = deserialization["obstacle"][i]["y"].get<float>();
            wi = deserialization["obstacle"][i]["width"].get<float>();
            he = deserialization["obstacle"][i]["height"].get<float>();
            vis = deserialization["obstacle"][i]["visible"].get<bool>();
            Obstacle* block = new Obstacle(sf::Vector2f(wi, he), sf::Vector2f(top, left), sf::Color::Yellow);
            block->setVisible(vis);
            game->gameField->addItem((DisplayObject*)block);
        }

        game->gameField->addItem(new StatusBar(
            sf::Vector2f(Settings::getResolution().first, Settings::getResolution().second / 20), 
            game->gameField->getData()
        ));
        inFile.close();
    }
}