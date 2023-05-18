#include <SFML/Graphics.hpp>
#include <nlohmann/json.hpp>
#include <sys/stat.h>
#include <bits/stdc++.h>
#include <unistd.h>
#include <math.h>
#include "classes.hpp"

using json = nlohmann::json;

std::mt19937_64 rng;
std::uniform_real_distribution<double> unif(0, 1);

void DisplayObject::draw(sf::RenderWindow &target) {
    if (visible) target.draw(*shape);
}

void DisplayObject::setColor(sf::Color col) {
    shape->setFillColor(col);
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
    if (dynamic_cast<Bonus*>(obj)) return;
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
    shape->move(velocity);
    bounds = shape->getGlobalBounds();
    position = shape->getPosition();
}

void MovableObject::move(sf::Vector2f vel) {
    if (!this->isVisible()) return;
    shape->move(vel);
    bounds = shape->getGlobalBounds();
    position = shape->getPosition();
}

void MovableObject::setVelocity(sf::Vector2f vel) {
    velocity = vel;
}

sf::Vector2f MovableObject::getVelocity() {
    return velocity;
}

Statistics::Statistics(int l, int s, std::string n, float d = 0) {
    lives = l;
    score = s;
    name = n;
    delay = d;
    clock = new sf::Clock();
}

int Statistics::getLives() { 
    return lives; 
}

int Statistics::getScore() { 
    return score; 
}

float Statistics::getTime() {
    return float(floor((clock->getElapsedTime().asSeconds() + delay) * 100)) / 100;
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

void Statistics::to_string(std::stringstream &strStream) {
    strStream << "\t\tStatistics\n" << "\t\t\tLives " << lives << "\n\t\t\tScore " << score << "\n\t\t\tTime " << delay << "\n\t\t\tName " << name << '\n'; 
}

SaveloadObject* Statistics::from_string(std::stringstream &strStream) {
    std::string temp, name;
    int lives, score;
    float delay;
    strStream >> temp >> temp >> lives >> temp >> score >> temp >> delay >> temp >> name;
    Statistics* stats = new Statistics(lives, score, name, delay);
    return stats;
}

json Statistics::to_json() {
    json seri{};
    seri["lives"] = lives;
    seri["score"] = score;
    seri["time"] = delay;
    seri["name"] = name;
    return seri;
}

SaveloadObject* Statistics::from_json(json &deri) {
    int lives, score;
    float delay;
    std::string name;
    lives = deri["lives"].get<int>();
    score = deri["score"].get<int>();
    delay = deri["time"].get<float>();
    name = deri["name"].get<std::string>();
    Statistics* stats = new Statistics(lives, score, name, delay);
    return stats;
}

TextBlock::TextBlock(sf::Vector2f size, sf::Vector2f pos, sf::Color col, std::string title) : DisplayObject(size, pos, col) {
    sf::Font *font = new sf::Font();
    font->loadFromFile("Roboto-Light.ttf");
    text = new sf::Text(title, *font);
    text->setPosition(pos);
    text->setCharacterSize(floor(float(1) / 30 * Settings::getResolution().second));
}

void TextBlock::setText(std::string str) {
    text->setString(str);
}

void TextBlock::draw(sf::RenderWindow &target) {
    target.draw(*shape);
    target.draw(*text);
}

void Platform::eventHandler(Event e) {
    float platformWidth = Settings::getResolution().first * PlatformSize::PS_MEDIUM / 1000;
    float platformHeight = Settings::getResolution().second * PlatformSize::PS_HEIGHT / 1000;
    switch (e.type) {
    case EventType::FALL:
        shape->setPosition(sf::Vector2f(
            (Settings::getResolution().first - platformWidth) / 2,
            Settings::getResolution().second - platformHeight
        ));
        move(sf::Vector2f(0, 0));
        break;
    case EventType::VERTICAL_COLLISION:
        if (e.obj != this) return;
        velocity = -velocity;
        if (bounds.left < 0) {
            move(sf::Vector2f(-bounds.left, 0));
        } else {
            move(sf::Vector2f(Settings::getResolution().first - bounds.left - bounds.width, 0));
        }
        break;
    }   
}

void Platform::to_string(std::stringstream &strStream) {
    strStream << "\t\tPlatform" << "\n\t\t\tX " << bounds.left << "\n\t\t\tY " << bounds.top << "\n\t\t\tWidth " << bounds.width << "\n\t\t\tHeight " << bounds.height << "\n\t\t\tXVelocity " << velocity.x << "\n";
}

SaveloadObject* Platform::from_string(std::stringstream &strStream) {
    std::string temp;
    float x, y, w, h, v;
    strStream >> temp >> temp >> x >> temp >> y >> temp >> w >> temp >> h >> temp >> v;
    Platform* platform = new Platform(sf::Vector2f(w, h), sf::Vector2f(x, y), sf::Color::Blue, v);
    return platform;
}

json Platform::to_json() {
    json seri{};
    seri["x"] = bounds.left;
    seri["y"] = bounds.top;
    seri["width"] = bounds.width;
    seri["height"] = bounds.height;
    seri["x_velocity"] = velocity.x;
    seri["scale"] = scale_coef;
    return seri;
}

SaveloadObject* Platform::from_json(json &deri) {
    float x, y, w, h, v, s;
    x = deri["x"].get<float>();
    y = deri["y"].get<float>();
    w = deri["width"].get<float>();
    h = deri["height"].get<float>();
    v = deri["x_velocity"].get<float>();
    s = deri["scale"].get<float>();
    Platform* platform = new Platform(sf::Vector2f(w, h), sf::Vector2f(x, y), sf::Color::Blue, v);
    platform->scaleSpeed(s);
    platform->setScale();
    return platform;
}

float Platform::getBaseSpeedAbs() {
    float resolution_coef = (float)(Settings::getResolution().first) / (Resolution::W1);
    return PlatformSpeed::PSP_MEDIUM * resolution_coef;
}

int Ball::getBaseSpeedAbs(Difficulty diff) {
    float resolution_coef = (float)(Settings::getResolution().first * Settings::getResolution().second) / (Resolution::W1 * Resolution::H1);
    switch (diff) {
    case (Difficulty::DF_EASY):
        return ceil(resolution_coef * BallSpeed::BSP_SLOW);
    case (Difficulty::DF_MEDIUM):
        return ceil(resolution_coef * BallSpeed::BSP_MEDIUM);
    case (Difficulty::DF_HARD):
        return ceil(resolution_coef * BallSpeed::BSP_FAST);
    }
    return 0;
}
void Ball::eventHandler(Event e) {
    if (e.obj != this) return;
    float ballSpeed;
    float ballSize = Settings::getResolution().second * BallSize::BS_MEDIUM / 1000;
    switch (e.type) {
    case EventType::FALL:
        shape->setPosition(sf::Vector2f(
            (Settings::getResolution().first - ballSize) / 2,
            Settings::getResolution().second / 20 * 19
        ));
        move(sf::Vector2f(0, 0));
        EventDispatcher::setGameEvent({EventType::LIVES_DOWN, nullptr});
        break;
    case EventType::VERTICAL_COLLISION:
        move(-velocity);
        ballSpeed = getBaseSpeedAbs(Settings::getDiff());
        velocity.y /= fabs(velocity.y);
        velocity.x /= fabs(velocity.x);
        velocity.x = -velocity.x;
        velocity.y *= ((float)unif(rng) * 0.3 + 0.5) * sqrt(ballSpeed);
        velocity.x *= sqrt(ballSpeed - velocity.y * velocity.y);
        setScale();
        break;
    case EventType::HORIZONTAL_COLLISION:
        move(-velocity);
        ballSpeed = getBaseSpeedAbs(Settings::getDiff());
        velocity.y /= fabs(velocity.y);
        velocity.y = -velocity.y;
        velocity.x /= fabs(velocity.x);
        velocity.y *= ((float)unif(rng) * 0.3 + 0.5) * sqrt(ballSpeed);
        velocity.x *= sqrt(ballSpeed - velocity.y * velocity.y);
        setScale();
        break;
    }
}

void Ball::to_string(std::stringstream &strStream) {
    strStream << "\t\t\tBall" << "\n\t\t\t\tX " << bounds.left << "\n\t\t\t\tY " << bounds.top << "\n\t\t\t\tRadius " << ((sf::CircleShape*)shape)->getRadius() << "\n\t\t\t\tXVelocity " << velocity.x << "\n\t\t\t\tYVelocity " << velocity.y << "\n\t\t\t\tVisible " << visible << "\n";
}

SaveloadObject* Ball::from_string(std::stringstream &strStream) {
    std::string temp;
    float x, y, r, vx, vy, vis;
    strStream >> temp >> temp >> x >> temp >> y >> temp >> r >> temp >> vx >> temp >> vy >> temp >> vis;
    Ball* ball = new Ball(r, sf::Vector2f(x, y), sf::Color::Cyan, sf::Vector2f(vx, vy));
    ball->setVisible(vis);
    return ball;
}

json Ball::to_json() {
    json seri{};
    seri["ball"]["x"] = bounds.left;
    seri["ball"]["y"] = bounds.top;
    seri["ball"]["radius"] = ((sf::CircleShape*)shape)->getRadius();
    seri["ball"]["x_velocity"] = velocity.x;
    seri["ball"]["y_velocity"] = velocity.y;
    seri["ball"]["visible"] = visible;
    seri["ball"]["scale"] = scale_coef;
    return seri;
}

SaveloadObject* Ball::from_json(json &deri) {
    float x, y, r, vx, vy, vis, s;
    x = deri["ball"]["x"].get<float>();
    y = deri["ball"]["y"].get<float>();
    r = deri["ball"]["radius"].get<float>();
    vx = deri["ball"]["x_velocity"].get<float>();
    vy = deri["ball"]["y_velocity"].get<float>();
    vis = deri["ball"]["visible"].get<float>();
    s = deri["ball"]["scale"].get<float>();
    Ball* ball = new Ball(r, sf::Vector2f(x, y), sf::Color::Cyan, sf::Vector2f(vx, vy));
    ball->setVisible(vis);
    ball->scaleSpeed(s);
    ball->setScale();
    return ball;
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

Obstacle::Obstacle(sf::Vector2f size, sf::Vector2f pos, sf::Color col) : DisplayObject(size, pos, col) {
    if ((float)unif(rng) < 0.25) {
        bonuses.push_back(new Bonus(size, pos, float(BonusSpeed::BSSP_MEDIUM)));
        setColor(sf::Color::Green);
    }
}

void Obstacle::eventHandler(Event e) {
    if (e.obj != this) return;
    switch (e.type) {
    case EventType::COLLISION:
        visible = false;
        //srand(time(NULL));
        EventDispatcher::setGameEvent({EventType::SCORE_UP, nullptr});
        if (bonuses.size() != 0) {
            EventDispatcher::setGameEvent({EventType::BONUS, bonuses[ceil((float)unif(rng) * (int)bonuses.size()) - 1]});
        }
        break;
    }
}

void Obstacle::to_string(std::stringstream &strStream) {
    strStream << "\tObstacle" << "\n\t\tX " << bounds.left << "\n\t\tY " << bounds.top << "\n\t\tWidth " << bounds.width << "\n\t\tHeight " << bounds.height << "\n\t\tVisible " << visible << "\n\t\tBonusesNum " << bonuses.size() << '\n';
    for (int i = 0; i < bonuses.size(); ++i) {
        bonuses[i]->to_string(strStream);
    }
}

SaveloadObject* Obstacle::from_string(std::stringstream &strStream) {
    std::string temp;
    float x, y, w, h, vis, size;
    strStream >> temp >> temp >> x >> temp >> y >> temp >> w >> temp >> h >> temp >> vis >> temp >> size;
    Obstacle* obstacle = new Obstacle(sf::Vector2f(w, h), sf::Vector2f(x, y), sf::Color::Yellow);
    obstacle->clearBonuses();
    obstacle->setColor(sf::Color::Yellow);
    if (size > 0) obstacle->setColor(sf::Color::Green);
    for (int i = 0; i < size; ++i) {
        Bonus* bonus = new Bonus(sf::Vector2f(0, 0), sf::Vector2f(0, 0), 0);
        bonus = (Bonus*)bonus->from_string(strStream);
        obstacle->addBonus(bonus);
    }
    obstacle->setVisible(vis);
    return obstacle;
}

json Obstacle::to_json() {
    json seri{};
    seri["obstacle"]["x"] = bounds.left;
    seri["obstacle"]["y"] = bounds.top;
    seri["obstacle"]["width"] = bounds.width;
    seri["obstacle"]["height"] = bounds.height;
    seri["obstacle"]["visible"] = visible;
    seri["obstacle"]["bonuses_num"] = bonuses.size();
    for (int i = 0; i < bonuses.size(); ++i) {
        seri["obstacle"]["bonuses"][i] = bonuses[i]->to_json();
    }
    return seri;
}

SaveloadObject* Obstacle::from_json(json &deri) {
    float x, y, w, h, vis;
    x = deri["obstacle"]["x"].get<float>();
    y = deri["obstacle"]["y"].get<float>();
    w = deri["obstacle"]["width"].get<float>();
    h = deri["obstacle"]["height"].get<float>();
    vis = deri["obstacle"]["visible"].get<float>();
    Obstacle* obstacle = new Obstacle(sf::Vector2f(w, h), sf::Vector2f(x, y), sf::Color::Yellow);
    obstacle->clearBonuses();
    obstacle->setColor(sf::Color::Yellow);
    int size = deri["obstacle"]["bonuses_num"].get<int>();
    if (size > 0) obstacle->setColor(sf::Color::Green);
    for (int i = 0; i < size; ++i) {
        Bonus* bonus = new Bonus(sf::Vector2f(0, 0), sf::Vector2f(0, 0), 0);
        bonus = (Bonus*)bonus->from_json(deri["obstacle"]["bonuses"][i]);
        obstacle->addBonus(bonus);
    }
    obstacle->setVisible(vis);
    return obstacle;
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
    target.draw(*shape);
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
        EventType::TO_MENU // Changed event
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
    std::ostringstream out;
    out << std::fixed << std::setprecision(2) << stats->getTime();
    std::string time = out.str();
    bar.push_back(new TextBlock(
        sf::Vector2f(Settings::getResolution().first / 10, Settings::getResolution().second / 20),
        sf::Vector2f(Settings::getResolution().first / 15 + (Settings::getResolution().first / 20 + Settings::getResolution().first / 10) * 2, 0),
        sf::Color::Black,
        "Time: " + time
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
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
        menu->sendEvent();
    }
    if (menu->underMouse(mousePos.x, mousePos.y)) {
        menu->setColor(sf::Color::Cyan);
        if (pressed) menu->sendEvent();
    }
    bar[0]->setText("Lives: " + std::to_string(stats->getLives()));
    bar[1]->setText("Score: " + std::to_string(stats->getScore()));
    std::ostringstream out;
    out << std::fixed << std::setprecision(2) << stats->getTime();
    std::string time = out.str();
    bar[2]->setText("Time: " + time);
    bar[3]->setText("Name: " + stats->getName());
}

void StatusBar::draw(sf::RenderWindow &target) {
    target.draw(*shape);
    menu->draw(target);
    for (TextBlock* text : bar) {
        text->draw(target);
    }
}

Menu::Menu(sf::Vector2f size, sf::Color col, std::vector<Button*> buttons, std::string title) : DisplayObject(size, sf::Vector2f((Settings::getResolution().first - size.x) / 2, (Settings::getResolution().second - size.y) / 2), col) {
    items = buttons;
    text = new TextBlock(
        sf::Vector2f(Settings::getResolution().first / 10, Settings::getResolution().second / 20), 
        sf::Vector2f((Settings::getResolution().first - Settings::getResolution().first / 10) / 2, Settings::getResolution().second / 20), 
        sf::Color::Red,
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
    target.draw(*shape);
    text->draw(target);
    for (Button* item : items) {
        item->draw(target);
    }
}

std::pair <Resolution, Resolution> Settings::resolution = {Resolution::W0, Resolution::H0};
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
    if (str == "1920x1000") {
        Settings::resolution = {Resolution::W0, Resolution::H0};
    } else if (str == "1600x900") {
        Settings::resolution = {Resolution::W1, Resolution::H1};
    } else if (str == "1560x877") {
        Settings::resolution = {Resolution::W2, Resolution::H2};
    } else if (str == "1520x720") {
        Settings::resolution = {Resolution::W3, Resolution::H3};
    } else if (str == "1480x720") {
        Settings::resolution = {Resolution::W4, Resolution::H4};
    } else if (str == "1366x768") {
        Settings::resolution = {Resolution::W5, Resolution::H5};
    } else if (str == "1280x720") {
        Settings::resolution = {Resolution::W6, Resolution::H6};
    } else if (str == "800x450") {
        Settings::resolution = {Resolution::W7, Resolution::H7};
    }
}

void Settings::to_string(std::stringstream &strStream) {
    strStream << "Settings\n" << "\tDifficulty " << difficulty << "\n\tResolution\n" << "\t\tWidth " << resolution.first << "\n\t\tHeight " << resolution.second << '\n';
}
    
SaveloadObject* Settings::from_string(std::stringstream &strStream) {
    Settings* settings = new Settings();
    std::string temp;
    int diff;
    std::pair <int, int> res;
    strStream >> temp >> temp >> diff >> temp >> temp >> res.first >> temp >> res.second;
    settings->setDiff((Difficulty)diff);
    settings->setResolution(std::pair<Resolution, Resolution>((Resolution)res.first, (Resolution)res.second));
    return settings;
}

json Settings::to_json() {
    json seri{};
    seri["settings"]["difficulty"] = difficulty;
    seri["settings"]["resolution"]["width"] = resolution.first;
    seri["settings"]["resolution"]["height"] = resolution.second;
    return seri;
}

SaveloadObject* Settings::from_json(json &deri) {
    int diff;
    std::pair <int, int> res;
    diff = deri["settings"]["difficulty"].get<int>();
    res.first = deri["settings"]["resolution"]["width"].get<int>();
    res.second = deri["settings"]["resolution"]["height"].get<int>();
    Settings* settings = new Settings();
    settings->setDiff((Difficulty)diff);
    settings->setResolution(std::pair<Resolution, Resolution>((Resolution)res.first, (Resolution)res.second));
    return settings;
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
    case (Resolution::W0):
        return "Resolution: 1920x1000";
        break;
    case (Resolution::W1):
        return "Resolution: 1600x900";
        break;
    case (Resolution::W2):
        return "Resolution: 1560x877";
        break;
    case (Resolution::W3):
        return "Resolution: 1520x720";
        break;
    case (Resolution::W4):
        return "Resolution: 1480x720";
        break;
    case (Resolution::W5):
        return "Resolution: 1366x768";
        break;
    case (Resolution::W6):
        return "Resolution: 1280x720";
        break;
    case (Resolution::W7):
        return "Resolution: 800x450";
        break;
    }
    return "";
}

GameField::GameField() : DisplayObject(sf::Vector2f(Settings::getResolution().first, Settings::getResolution().second), sf::Vector2f(0, 0), sf::Color::Black) {}

std::vector<DisplayObject*> GameField::getObjects() {
    return objects;
}

void GameField::to_string(std::stringstream &strStream) {
    strStream << "Gamefield\n\tObstaclesNum " << objects.size() - move_objects.size() - 1 << "\n\tTimersNum " << bonus_timers.size();
    for (std::map<sf::Clock*, std::pair<float, EventType>> :: iterator it = bonus_timers.begin(); it != bonus_timers.end(); it++) {
        strStream << "\n\tTimer\n\t\tTimeLeft " << it->second.first - it->first->getElapsedTime().asSeconds() << "\n\t\tEvent " << it->second.second << '\n';
    }
    for (DisplayObject* obj : objects) {
        if (dynamic_cast<Obstacle*>(obj)) {
            obj->to_string(strStream);
        }
    }
}

SaveloadObject* GameField::from_string(std::stringstream &strStream) {
    std::string temp;
    int sizeO, sizeT;
    strStream >> temp >> temp >> sizeO >> temp >> sizeT;
    GameField* field = new GameField();
    for (int i = 0; i < sizeT; ++i) {
        float time;
        int event;
        strStream >> temp >> temp >> time >> temp >> event;
        sf::Clock* clock = new sf::Clock();
        field->addTimer({time, (EventType)event});
    } 
    for (int i = 0; i < sizeO; ++i) {
        Obstacle* obstacle = new Obstacle(sf::Vector2f(0, 0), sf::Vector2f(0, 0), sf::Color::Black);
        obstacle = (Obstacle*)obstacle->from_string(strStream);
        field->addItem(obstacle);
    }
    return field;
}

json GameField::to_json() {
    json seri{};
    int j = 0;
    for (std::map<sf::Clock*, std::pair<float, EventType>> :: iterator it = bonus_timers.begin(); it != bonus_timers.end(); it++) {
        seri["gamefield"]["timers"][j]["time_left"] = it->second.first - it->first->getElapsedTime().asSeconds();
        seri["gamefield"]["timers"][j]["event"] = it->second.second;
        j++; 
    }
    j = 0;
    for (int i = 0; i < objects.size(); ++i) {
        if (dynamic_cast<Obstacle*>(objects[i])) {
            seri["gamefield"]["obstacles"][j] = objects[i]->to_json();
            j++;
        }
    }
    seri["gamefield_timers_num"] = bonus_timers.size();
    seri["gamefield_obstacles_num"] = j;
    return seri;
}

SaveloadObject* GameField::from_json(json &deri) {
    GameField* field = new GameField();
    int size = deri["gamefield_timers_num"].get<int>();
    for (int i = 0; i < size; ++i) {
        sf::Clock* clock = new sf::Clock();
        field->addTimer({deri["gamefield"]["timers"][i]["time_left"].get<float>(), (EventType)deri["gamefield"]["timers"][i]["event"].get<int>()});
    } 
    size = deri["gamefield_obstacles_num"].get<int>();
    for (int i = 0; i < size; ++i) {
        Obstacle* obstacle = new Obstacle(sf::Vector2f(0, 0), sf::Vector2f(0, 0), sf::Color::Black);
        obstacle = (Obstacle*)obstacle->from_json(deri["gamefield"]["obstacles"][i]);
        field->addItem(obstacle);
    }
    return field;
}

void GameField::freezeTimers() {
    for (std::map<sf::Clock*, std::pair<float, EventType>> :: iterator it = bonus_timers.begin(); it != bonus_timers.end(); it++) {
        it->second.first -= it->first->restart().asSeconds();
    }
}

void GameField::startTimers() {
    for (std::map<sf::Clock*, std::pair<float, EventType>> :: iterator it = bonus_timers.begin(); it != bonus_timers.end(); it++) {
        it->first->restart();
    }
}

void GameField::draw(sf::RenderWindow &target) {
    target.draw(*shape);
    for (DisplayObject* obj : objects) {
        obj->draw(target);
    }
}

void GameField::addItem(DisplayObject *obj) {
    objects.push_back(obj);
    if (dynamic_cast<Obstacle*>(obj) && !obj->isVisible()) {
        for (Bonus* bonus : ((Obstacle*)obj)->getBonuses()) {
            bonuses.push_back(bonus);
            move_objects.push_back(bonus);
            objects.push_back(bonus);
        }
    }
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
    case EventType::BALL_FASTEN:
        bonus_timers.insert({new sf::Clock(), {(float)10, EventType::BALL_FASTEN_DECLINE}});
        for (Ball* ball : balls) {
            ball->scaleSpeed(Coefficients::BALL_COEF * 1.3333);
            ball->setScale();
        }
        break;
    case EventType::BALL_FASTEN_DECLINE:
        for (Ball* ball : balls) {
            ball->scaleSpeed((float)1 / (Coefficients::BALL_COEF * 1.3333));
            ball->setScale();
        }
        break;
    case EventType::BALL_SLOWEN:
        bonus_timers.insert({new sf::Clock(), {(float)10, EventType::BALL_SLOWEN_DECLINE}});
        for (Ball* ball : balls) {
            ball->scaleSpeed((float)1 / (Coefficients::BALL_COEF * 1.5));
            ball->setScale();
        }
        break;
    case EventType::BALL_SLOWEN_DECLINE:
        for (Ball* ball : balls) {
            ball->scaleSpeed(Coefficients::BALL_COEF * 1.5);
            ball->setScale();
        }
        break;
    case EventType::PLATFORM_FASTEN:
        bonus_timers.insert({new sf::Clock(), {(float)10, EventType::PLATFORM_FASTEN_DECLINE}});
        for (Platform* platform : platforms) {
            platform->scaleSpeed(Coefficients::PLATFORM_COEF * 1.5);
            platform->setScale();
        }
        break;
    case EventType::PLATFORM_FASTEN_DECLINE:
        for (Platform* platform : platforms) {
            platform->scaleSpeed((float)1 / (Coefficients::PLATFORM_COEF * 1.5));
            platform->setScale();
        }
        break;
    case EventType::PLATFORM_SLOWEN:
        bonus_timers.insert({new sf::Clock(), {(float)10, EventType::PLATFORM_SLOWEN_DECLINE}});
        for (Platform* platform : platforms) {
            platform->scaleSpeed((float)1 / (Coefficients::PLATFORM_COEF * 1.3333));
            platform->setScale();
        }
        break;
    case EventType::PLATFORM_SLOWEN_DECLINE:
        for (Platform* platform : platforms) {
            platform->scaleSpeed(Coefficients::PLATFORM_COEF * 1.3333);
            platform->setScale();
        }
        break;
    case EventType::PLATFORM_LONGEN:
        bonus_timers.insert({new sf::Clock(), {(float)10, EventType::PLATFORM_LONGEN_DECLINE}});
        for (Platform* platform : platforms) {
            platform->scale(Coefficients::PLATFORM_COEF * 1.5);
        }
        break;
    case EventType::PLATFORM_LONGEN_DECLINE:
        for (Platform* platform : platforms) {
            platform->scale((float)1 / (Coefficients::PLATFORM_COEF * 1.5));
        }
        break;
    case EventType::PLATFORM_SHORTEN:
        bonus_timers.insert({new sf::Clock(), {(float)10, EventType::PLATFORM_SHORTEN_DECLINE}});
        for (Platform* platform : platforms) {
            platform->scale((float)1 / (Coefficients::PLATFORM_COEF * 1.5));
        }
        break;
    case EventType::PLATFORM_SHORTEN_DECLINE:
        for (Platform* platform : platforms) {
            platform->scale(Coefficients::PLATFORM_COEF * 1.5);
        }
        break;
    case EventType::BONUS:
        bonuses.push_back((Bonus*)e.obj);
        move_objects.push_back((MovableObject*)e.obj);
        objects.push_back(e.obj);
        break;
    case EventType::LIVES_DOWN:
        data->setLives(data->getLives() - 1);
        if (data->getLives() <= 0) {
            EventDispatcher::setEvent({EventType::LOSE, nullptr});
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
            EventDispatcher::setEvent({EventType::WIN, nullptr});
        }
        break;
    }
}

void GameField::moveObjects() {
    for (MovableObject* obj : move_objects) {
        if (dynamic_cast<Platform*>(obj)) continue;
        obj->move();
    }
    float platformSpeed = Platform(sf::Vector2f(0, 0)).getBaseSpeedAbs();
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
        for (Platform* platform : platforms) {
            platform->setVelocity(sf::Vector2f(-platformSpeed, 0));
            platform->setScale();
            platform->move();
        }
    } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
        for (Platform* platform : platforms) {
            platform->setVelocity(sf::Vector2f(platformSpeed, 0));
            platform->setScale();
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
    std::vector<sf::Clock*> to_delete;
    for (std::map<sf::Clock*, std::pair<float, EventType>> :: iterator it = bonus_timers.begin(); it != bonus_timers.end(); it++) {
        if (it->first->getElapsedTime().asSeconds() > it->second.first) {
            EventDispatcher::setGameEvent({it->second.second, nullptr});
            to_delete.push_back(it->first);
        }
    }
    for (int i = 0; i < to_delete.size(); i++) {
        bonus_timers.erase(to_delete[i]);
    }
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
    float platformSpeed = Platform(sf::Vector2f(0, 0)).getBaseSpeedAbs();
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
        platformSpeed
    );
    float ballSize = Settings::getResolution().second * BallSize::BS_MEDIUM / 1000;
    float ballSpeed = Ball(0).getBaseSpeedAbs(Settings::getDiff());
    Ball *ball = new Ball(
        ballSize,
        sf::Vector2f(
            (Settings::getResolution().first - ballSize) / 2,
            Settings::getResolution().second / 20 * 19
        ),
        sf::Color::Cyan,
        sf::Vector2f(sqrt(ballSpeed / 2.0), sqrt(ballSpeed / 2.0))
    );
    balls.push_back(ball);
    stats = new Statistics(3, 0, name);
}

Platform* Player::getPlatform() {
    return platform;
}

std::vector <Ball*> Player::getBalls() {
    return balls;
}

void Player::to_string(std::stringstream &strStream) {
    strStream << "\tPlayer\n";
    stats->to_string(strStream);
    platform->to_string(strStream);
    strStream << "\t\tBalls\n\t\t\tBallsNum " << balls.size() << "\n";
    for (Ball* ball : balls) {
        ball->to_string(strStream);
    }
}

SaveloadObject* Player::from_string(std::stringstream &strStream) {
    std::string temp;
    strStream >> temp;
    Statistics* stats = new Statistics(0, 0, "");
    stats = (Statistics*)stats->from_string(strStream);
    Platform* platform = new Platform(sf::Vector2f(0, 0), sf::Vector2f(0, 0));
    platform = (Platform*)platform->from_string(strStream);
    int size;
    strStream >> temp >> temp >> size;
    std::vector <Ball*> balls;
    for (int i = 0; i < size; ++i) {
        Ball* ball = new Ball(0, sf::Vector2f(0, 0), sf::Color::Black);
        ball = (Ball*)ball->from_string(strStream);
        balls.push_back(ball);
    }
    Player* player = new Player(stats, platform, balls);
    return player;
}

json Player::to_json() {
    json seri{};
    seri["player"]["statistics"] = stats->to_json();
    seri["player"]["platform"] = platform->to_json();
    seri["player"]["balls_num"] = balls.size();
    for (int i = 0; i < balls.size(); ++i) {
        seri["player"]["balls"][i] = balls[i]->to_json();
    } 
    return seri;
}

SaveloadObject* Player::from_json(json &deri) {
    Statistics* stats = new Statistics(0, 0, "");
    Platform* platform = new Platform(sf::Vector2f(0, 0), sf::Vector2f(0, 0));
    std::vector <Ball*> balls;
    int size = deri["player"]["balls_num"].get<int>();

    stats = (Statistics*)stats->from_json(deri["player"]["statistics"]);
    platform = (Platform*)platform->from_json(deri["player"]["platform"]);
    for (int i = 0; i < size; ++i) {
        Ball* ball = new Ball(0, sf::Vector2f(0, 0), sf::Color::Black);
        ball = (Ball*)ball->from_json(deri["player"]["balls"][i]);
        balls.push_back(ball);
    }
    Player* player = new Player(stats, platform, balls);
    return player;
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

void Players::to_string(std::stringstream &strStream) {
    strStream << "Players\n\tPlayersNum " << players.size() << "\n";
    for (Player* player : players) {
        player->to_string(strStream);
    }
}

SaveloadObject* Players::from_string(std::stringstream &strStream) {
    std::string temp;
    int size;
    strStream >> temp >> temp >> size;
    Players* players = new Players();
    for (int i = 0; i < size; ++i) {
        Player* player = new Player("");
        player = (Player*)player->from_string(strStream);
        players->addPlayer(player);
    }
    return players;
}

json Players::to_json() {
    json seri{};
    seri["players_num"] = players.size();
    for (int i = 0; i < players.size(); ++i) {
        seri["players"][i] = players[i]->to_json();
    } 
    return seri;
}

SaveloadObject* Players::from_json(json &deri) {
    int size = deri["players_num"].get<int>();
    Players* players = new Players();
    for (int i = 0; i < size; ++i) {
        Player* player = new Player("");
        player = (Player*)player->from_json(deri["players"][i]);
        players->addPlayer(player);
    }
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
            break;
        case EventType::SAVE:
            save(toSave);
            // save_json(toSave);
            break;
        case EventType::WIN:
            init();
            state = Active::MESSAGE_WIN;
            break;
        case EventType::LOSE:
            init();
            state = Active::MESSAGE_LOSE;
            break;
        /**
        case EventType::END_GAME:
            init();
            state = Active::MENU;
            break;
        */
        case EventType::START:
            state = Active::MESSAGE_START;
            break;
        case EventType::TO_GAME:
            gameField->getData()->getClock()->restart();
            gameField->startTimers();
            state = Active::GAME;
            break;
        /**
        case EventType::TO_PAUSE:
            state = Active::PAUSE;
            break;
        */
        case EventType::TO_MENU:
            if (state == Active::GAME) {
                gameField->getData()->setDelay(gameField->getData()->getClock()->getElapsedTime().asSeconds());
                gameField->freezeTimers();
            }
            state = Active::MENU;
            break;
        case EventType::LOAD:      
            load();
            // load_json();
            window->close();
            create();
            break;
        case EventType::TO_SETTINGS:
            state = Active::SETTINGS;
            break;
        case EventType::NEW_GAME:
            init();
            state = Active::MESSAGE_START;
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
            case Resolution::W0:
                settings->setResolution("1600x900");
                break;
            case Resolution::W1:
                settings->setResolution("1560x877");
                break;
            case Resolution::W2:
                settings->setResolution("1520x720");
                break;
            case Resolution::W3:
                settings->setResolution("1480x720");
                break;
            case Resolution::W4:
                settings->setResolution("1366x768");
                break;
            case Resolution::W5:
                settings->setResolution("1280x720");
                break;
            case Resolution::W6:
                settings->setResolution("800x450");
                break;
            case Resolution::W7:
                settings->setResolution("1920x1000");
                break;
            }
            window->close();
            create();
            reinit();
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
    // window->clear();
    sf::Vector2i mousePos = sf::Mouse::getPosition(*window);
    switch (state) {
        case Active::MESSAGE_LOSE:
            lose->update(mousePos, pressed);
            gameField->draw(*window);
            lose->draw(*window);
            break;
        case Active::MESSAGE_WIN:
            win->update(mousePos, pressed);
            gameField->draw(*window);
            win->draw(*window);
            break;
        case Active::MESSAGE_START:
            start->update(mousePos, pressed);
            gameField->draw(*window);
            start->draw(*window);
            break;
        case Active::MENU:
            pauseMenu->update(mousePos, pressed);
            gameField->draw(*window);
            pauseMenu->draw(*window);
            // menu->update(mousePos, pressed);
            // menu->draw(*window);
            break;
        case Active::SETTINGS:
            settingsMenu->update(mousePos, pressed);
            gameField->draw(*window);
            settingsMenu->draw(*window);
            break;
        case Active::GAME:
            gameField->update(mousePos, pressed);
            gameField->draw(*window);
            break;
        /**
        case Active::PAUSE:
            pauseMenu->update(mousePos, pressed);
            pauseMenu->draw(*window);
            break;
        */
    }
    window->display();
}

void Game::create() {
    uint64_t timeSeed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::seed_seq ss{uint32_t(timeSeed & 0xffffffff), uint32_t(timeSeed>>32)};
    rng.seed(ss);

    window = new sf::RenderWindow(
        sf::VideoMode(Settings::getResolution().first, Settings::getResolution().second), 
        "Arcanoid",
        sf::Style::Titlebar | sf::Style::Close
    );
    window->setKeyRepeatEnabled(false);
}

void Game::initMenus() {
    sf::Vector2f fullResolution = sf::Vector2f(Settings::getResolution().first, Settings::getResolution().second);
    sf::Vector2f buttonSize = sf::Vector2f(fullResolution.x / 5, fullResolution.y / 20);
    float delta = fullResolution.y / 20;
    sf::Vector2f menuSize = sf::Vector2f(fullResolution.x / 3, fullResolution.y - 2 * delta);
    sf::Vector2f boxSize = sf::Vector2f(fullResolution.x / 3 * 2, fullResolution.y / 8);
    float xPos = (fullResolution.x - buttonSize.x) / 2;
    float yPos = fullResolution.y / 10;
    std::string diffStr = Settings::getDiffStr();
    std::string resolutionStr = Settings::getResolutionStr();
    /**
    std::vector<Button*> menu_buttons;
    menu_buttons.push_back(new Button(buttonSize, sf::Vector2f(xPos, yPos), sf::Color::Blue, "New Game", EventType::NEW_GAME));
    menu_buttons.push_back(new Button(buttonSize, sf::Vector2f(xPos, yPos * 2), sf::Color::Blue, "Continue", EventType::CONTINUE));
    menu_buttons.push_back(new Button(buttonSize, sf::Vector2f(xPos, yPos * 3), sf::Color::Blue, "Settings", EventType::TO_SETTINGS));
    menu_buttons.push_back(new Button(buttonSize, sf::Vector2f(xPos, yPos * 4), sf::Color::Blue, "Quit", EventType::QUIT));
    menu = new Menu(fullResolution, sf::Color::Black, menu_buttons, "Arcanoid");
    */
    
    std::vector<Button*> settings_buttons;
    settings_buttons.push_back(new Button(buttonSize, sf::Vector2f(xPos, delta + yPos), sf::Color::Blue, diffStr, EventType::SWITCH_DIFFICULTY));
    settings_buttons.push_back(new Button(buttonSize, sf::Vector2f(xPos, delta + yPos * 2), sf::Color::Blue, resolutionStr, EventType::SWITCH_RESOLUTION));
    settings_buttons.push_back(new Button(buttonSize, sf::Vector2f(xPos, delta + yPos * 3), sf::Color::Blue, "Return to menu", EventType::TO_MENU));
    settingsMenu = new Menu(menuSize, sf::Color(0, 0, 0, 0), settings_buttons, "Settings"); // Changed sizes

    std::vector<Button*> pause_buttons;
    pause_buttons.push_back(new Button(buttonSize, sf::Vector2f(xPos, delta + yPos), sf::Color::Blue, "Continue", EventType::START));
    pause_buttons.push_back(new Button(buttonSize, sf::Vector2f(xPos, delta + yPos * 2), sf::Color::Blue, "New Game", EventType::NEW_GAME)); // Name changed
    pause_buttons.push_back(new Button(buttonSize, sf::Vector2f(xPos, delta + yPos * 3), sf::Color::Blue, "Save", EventType::SAVE));
    pause_buttons.push_back(new Button(buttonSize, sf::Vector2f(xPos, delta + yPos * 4), sf::Color::Blue, "Load", EventType::LOAD));
    pause_buttons.push_back(new Button(buttonSize, sf::Vector2f(xPos, delta + yPos * 5), sf::Color::Blue, "Settings", EventType::TO_SETTINGS)); // Added new line
    pause_buttons.push_back(new Button(buttonSize, sf::Vector2f(xPos, delta + yPos * 6), sf::Color::Blue, "Exit", EventType::QUIT)); // Name and event changed
    pauseMenu = new Menu(menuSize, sf::Color(0, 0, 0, 0), pause_buttons, "Menu"); // Changed sizes and name

    start = new MessageBox(EventType::TO_GAME, "Press the button to start/continue the game session", boxSize);
    win = new MessageBox(EventType::TO_MENU, "You destroyed all obstacles on your way! You won!", boxSize);
    lose = new MessageBox(EventType::TO_MENU, "You lost too many lives ans you died. You lost", boxSize);
}

void Game::reinit() {
    GameField* newGameField = new GameField();

    sessionPlayers = new Players();
    sessionPlayers->addPlayer(new Player("Artur"));

    initMenus();
    for (Player* player : sessionPlayers->getPlayers()) {
        newGameField->addItem((Platform*)player->getPlatform());
        for (Ball* ball : player->getBalls()) {
             newGameField->addItem((Ball*)ball);
        }
        newGameField->addItem((Statistics*)player->getStatistics());
    }

    std::vector <Obstacle*> blocks;
    sf::Vector2f fullResolution = sf::Vector2f(Settings::getResolution().first, Settings::getResolution().second);
    int rowNum = ObstacleNum::OB_ROW / (4.5 - Settings::getDiff());
    int columnNum = ObstacleNum::OB_COLUMN / (4.5 - Settings::getDiff());
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
    int i = 0;
    for (Obstacle* block : blocks) {
        block->clearBonuses();
        block->setColor(sf::Color::Yellow);
        while (!dynamic_cast<Obstacle*>(gameField->getObjects()[i])) i++;
        std::vector<Bonus*> bonuses = ((Obstacle*)(gameField->getObjects()[i]))->getBonuses();
        i++;
        if (bonuses.size() > 0) {
            block->setColor(sf::Color::Green);
            for (Bonus* bonus : bonuses) {
                Bonus* resizedBonus = new Bonus(sf::Vector2f(block->getBound().width, block->getBound().height), sf::Vector2f(block->getBound().left, block->getBound().top), (float)BonusSpeed::BSSP_MEDIUM);
                resizedBonus->setBonus(bonus->getBonus());
                block->addBonus(resizedBonus);
            }
        }
        newGameField->addItem((DisplayObject*)block);
    }

    newGameField->addItem(new StatusBar(
        sf::Vector2f(fullResolution.x, fullResolution.y / 20), 
        newGameField->getData()
    ));

    gameField = newGameField;

    toSave.clear();
    toSave.push_back(settings);
    toSave.push_back(sessionPlayers);
    toSave.push_back(gameField);
}

void Game::load() {
    std::string filename = "save.txt";
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    if (stat_buf.st_size == 0) {
        init();
        state = Active::MENU;
        return;
    }
    std::stringstream strStream;
    inFile.open(filename);
    strStream << inFile.rdbuf();
    inFile.close();

    history->from_string(toSave, strStream);

    settings = (Settings*)toSave[0];
    history = new SerializeProxy();

    sessionPlayers = (Players*)toSave[1];

    initMenus();
    
    gameField = (GameField*)toSave[2];
    for (Player* player : sessionPlayers->getPlayers()) {
        gameField->addItem((Platform*)player->getPlatform());
        for (Ball* ball : player->getBalls()) {
             gameField->addItem((Ball*)ball);
        }
        gameField->addItem((Statistics*)player->getStatistics());
    }

    gameField->addItem(new StatusBar(
        sf::Vector2f(Settings::getResolution().first, Settings::getResolution().second / 20), 
        gameField->getData()
    ));

    toSave.clear();
    toSave.push_back(settings);
    toSave.push_back(sessionPlayers);
    toSave.push_back(gameField);
}

void Game::load_json() {
    std::string filename = "save.json";
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    if (stat_buf.st_size == 0) {
        init();
        state = Active::MENU;
        return;
    }
    json deri{};
    std::stringstream strStream;
    inFile.open(filename);
    strStream << inFile.rdbuf();
    inFile.close();
    std::string str = strStream.str();
    deri = json::parse(str);
    history->from_json(toSave, deri);

    settings = (Settings*)toSave[0];
    history = new SerializeProxy();

    sessionPlayers = (Players*)toSave[1];

    initMenus();
    
    gameField = (GameField*)toSave[2];
    for (Player* player : sessionPlayers->getPlayers()) {
        gameField->addItem((Platform*)player->getPlatform());
        for (Ball* ball : player->getBalls()) {
             gameField->addItem((Ball*)ball);
        }
        gameField->addItem((Statistics*)player->getStatistics());
    }

    gameField->addItem(new StatusBar(
        sf::Vector2f(Settings::getResolution().first, Settings::getResolution().second / 20), 
        gameField->getData()
    ));

    toSave.clear();
    toSave.push_back(settings);
    toSave.push_back(sessionPlayers);
    toSave.push_back(gameField);
}

void Game::save(std::vector<SaveloadObject *> toSave) {
    std::string seri = history->to_string(toSave);
    outFile.open("save.txt");
    outFile << seri << '\n';
    outFile.close();
}

void Game::save_json(std::vector<SaveloadObject *> toSave) {
    json seri = history->to_json(toSave);
    outFile.open("save.json");
    outFile << std::setw(4) << seri << '\n';
    outFile.close();
}

void Game::init() {
    state = Active::MENU;
    
    settings = new Settings();
    history = new SerializeProxy();

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
    int rowNum = ObstacleNum::OB_ROW / (4.5 - Settings::getDiff());
    int columnNum = ObstacleNum::OB_COLUMN / (4.5 - Settings::getDiff());
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

    toSave.clear();
    toSave.push_back(settings);
    toSave.push_back(sessionPlayers);
    toSave.push_back(gameField);
}

void Game::process() {
    int tick = 16000;
    while (window->isOpen()) {
        timer.restart();
        eventHandler({EventType::FRAME, nullptr});
        int slp = tick - timer.getElapsedTime().asMicroseconds();
        usleep(std::max(slp, 0));
    }
}

std::string SerializeProxy::to_string(std::vector <SaveloadObject*> &toSave) {
    std::stringstream strStream;
    for (int i = 0; i < toSave.size(); ++i) {
        toSave[i]->to_string(strStream);
    }
    std::string seri = strStream.str();
    return seri;
}

json SerializeProxy::to_json(std::vector <SaveloadObject*> &toSave) {
    json seri;
    for (int i = 0; i < toSave.size(); ++i) {
        seri[i] = toSave[i]->to_json();
    }
    return seri;
}

void SerializeProxy::from_string(std::vector <SaveloadObject*> &toLoad, std::stringstream &strStream) {
    for (int i = 0; i < toLoad.size(); ++i) {
        toLoad[i] = toLoad[i]->from_string(strStream);
    }
}

void SerializeProxy::from_json(std::vector <SaveloadObject*> &toLoad, json &deri) {
    for (int i = 0; i < toLoad.size(); ++i) {
        toLoad[i] = toLoad[i]->from_json(deri[i]);
    }
}

MessageBox::MessageBox(EventType event, std::string str, sf::Vector2f size) : DisplayObject(size, sf::Vector2f((Settings::getResolution().first - size.x) / 2, (Settings::getResolution().second - size.y) / 2), sf::Color(0, 0, 0, 0)) {
    sf::Vector2f nullPoint = sf::Vector2f((Settings::getResolution().first - size.x) / 2, (Settings::getResolution().second - size.y) / 2);
    text = new TextBlock(sf::Vector2f(size.x, size.y * 5 / 6), nullPoint, sf::Color::Red, str);
    button = new Button(sf::Vector2f(size.x / 2, size.y / 3), sf::Vector2f(nullPoint.x + size.x / 4, nullPoint.y + size.y * 5 / 6), sf::Color::Blue, "OK", event);
}

void MessageBox::draw(sf::RenderWindow &target)
{
    text->draw(target);
    button->draw(target);
}

void MessageBox::setText(std::string str) {
    text->setText(str);
}

void MessageBox::update(sf::Vector2i mousePos, bool pressed) {
    button->setColor(sf::Color::Blue);
    if (button->underMouse(mousePos.x, mousePos.y)) {
        if (pressed) {
            button->sendEvent();
        } else {
            button->setColor(sf::Color::Cyan);
        }
    }
}

Bonus::Bonus(sf::Vector2f size, sf::Vector2f pos, float vel) : MovableObject(size, pos, sf::Color::White, sf::Vector2f(0, vel)) {
    bonus = (EventType)(ceil((float)unif(rng) * 6) + 100);
    std::string filepath = "";
    switch (bonus) {
    case EventType::BALL_FASTEN:
        filepath = "BSPU.png";
        break;
    case EventType::BALL_SLOWEN:
        filepath = "BSPD.png";
        break;
    case EventType::PLATFORM_FASTEN:
        filepath = "PSPU.png";
        break;
    case EventType::PLATFORM_SLOWEN:
        filepath = "PSPD.png";
        break;
    case EventType::PLATFORM_LONGEN:
        filepath = "PSZU.png";
        break;
    case EventType::PLATFORM_SHORTEN:
        filepath = "PSZD.png";
        break;
    }
    sf::Texture* texture = new sf::Texture();
    texture->loadFromFile(filepath);
    shape->setTexture(texture, true);
}

void Bonus::setBonus(EventType e) { 
    bonus = e; 
    std::string filepath = "";
    switch (bonus) {
    case EventType::BALL_FASTEN:
        filepath = "BSPU.png";
        break;
    case EventType::BALL_SLOWEN:
        filepath = "BSPD.png";
        break;
    case EventType::PLATFORM_FASTEN:
        filepath = "PSPU.png";
        break;
    case EventType::PLATFORM_SLOWEN:
        filepath = "PSPD.png";
        break;
    case EventType::PLATFORM_LONGEN:
        filepath = "PSZU.png";
        break;
    case EventType::PLATFORM_SHORTEN:
        filepath = "PSZD.png";
        break;
    }
    sf::Texture* texture = new sf::Texture();
    texture->loadFromFile(filepath);
    shape->setTexture(texture, true);
}

void Bonus::checkCollision(DisplayObject *obj)
{
    if (!dynamic_cast<Platform*>(obj)) return;
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

void Bonus::checkBounds() {
    int w = Settings::getResolution().first, h = Settings::getResolution().second;
    if (bounds.left < 0 || bounds.left + bounds.width > w) EventDispatcher::setGameEvent({EventType::VERTICAL_COLLISION, this});
    if (bounds.top + bounds.height > h) EventDispatcher::setGameEvent({EventType::BONUS_FALL, this});
}

void Bonus::eventHandler(Event e) {
    if (e.obj != this) return;
    switch (e.type) {
    case EventType::BONUS_FALL:
        setVisible(false);
        break;
    case EventType::HORIZONTAL_COLLISION:
        setVisible(false);
        EventDispatcher::setGameEvent({bonus, nullptr});
        break;
    }
}

void Bonus::to_string(std::stringstream &strStream) {
    strStream << "\t\tBonus\n\t\t\tType " << bonus << "\n\t\t\tX " << bounds.left << "\n\t\t\tY " << bounds.top << "\n\t\t\tWidth " << bounds.width << "\n\t\t\tHeight " << bounds.height << "\n\t\t\tYVelocity " << velocity.y << "\n\t\t\tVisible " << visible << '\n';
}

SaveloadObject *Bonus::from_string(std::stringstream &strStream) {
    float x, y, w, h, vis, vel;
    int event;
    std::string temp;
    strStream >> temp >> temp >> event >> temp >> x >> temp >> y >> temp >> w >> temp >> h >> temp >> vel >> temp >> vis;
    Bonus* bon = new Bonus(sf::Vector2f(w, h), sf::Vector2f(x, y), vel);
    bon->setBonus((EventType)event);
    bon->setVisible(vis);
    return bon;
}

json Bonus::to_json() {
    json seri{};
    seri["bonus"]["type"] = bonus;
    seri["bonus"]["x"] = bounds.left;
    seri["bonus"]["y"] = bounds.top;
    seri["bonus"]["width"] = bounds.width;
    seri["bonus"]["height"] = bounds.height;
    seri["bonus"]["y_velocity"] = velocity.y;
    seri["bonus"]["visible"] = visible;
    return seri;
}

SaveloadObject* Bonus::from_json(json &deri) {
    float x, y, w, h, vis, vel;
    int event;
    event = deri["bonus"]["type"].get<int>();
    x = deri["bonus"]["x"].get<float>();
    y = deri["bonus"]["y"].get<float>();
    w = deri["bonus"]["width"].get<float>();
    h = deri["bonus"]["height"].get<float>();
    vel = deri["bonus"]["y_velocity"].get<float>();
    vis = deri["bonus"]["visible"].get<bool>();
    Bonus* bon = new Bonus(sf::Vector2f(w, h), sf::Vector2f(x, y), vel);
    bon->setBonus((EventType)event);
    bon->setVisible(vis);
    return bon;
}
