#include <bits/stdc++.h>
#include <SFML/Graphics.hpp>
#include "classes.hpp"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main() {
    Game *game = new Game();;
    game->create();
    game->init();
    game->process();
    return 0;
}