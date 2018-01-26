#pragma once
#include <string>

class Room
{
public:
    Room();

    std::string GetMessage();
    Room* GetN();
    Room* GetS();
    Room* GetE();
    Room* GetW();

    inline void SetMSG(std::string msg){message = msg;};
    inline void SetN(Room* n){roomN = n;};
    inline void SetS(Room* s){roomS = s;};
    inline void SetE(Room* e){roomE = e;};
    inline void SetW(Room* w){roomW = w;};

private:
    std::string message;
    Room* roomN;
    Room* roomS;
    Room* roomE;
    Room* roomW;
};
