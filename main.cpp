#include <bits/stdc++.h>
#include <SFML/Graphics.hpp>
#include "classes.hpp"

int main() {
    Game *game = new Game();
    game->create();
    game->init();
    game->process();
    return 0;
}