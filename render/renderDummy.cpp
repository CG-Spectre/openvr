//
// Created by Aidan Anderson on 4/4/25.
//

#include "renderDummy.h"

#include <iostream>
#include <ostream>

renderDummy::renderDummy(std::string message) {
    this->message = message;
}

void renderDummy::render(SDL_Renderer *renderer) {
    std::cout << "Dummy message: " << message << std::endl;
}

