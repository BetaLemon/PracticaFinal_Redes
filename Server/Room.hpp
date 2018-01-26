#pragma once
#include <string>

class Room
{
public:
    Room();
    Room(std::string msg, Room* n, Room* s, Room* e, Room* w);

    std::string GetMessage();
    Room* GetN();
    Room* GetS();
    Room* GetE();
    Room* GetW();

private:
    std::string message;
    Room* roomN;
    Room* roomS;
    Room* roomE;
    Room* roomW;
};
