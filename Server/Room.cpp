#include "Room.hpp"

Room::Room(){
    message = nullptr;
    roomN = nullptr;
    roomS = nullptr;
    roomE = nullptr;
    roomW = nullptr;
}
Room::Room(char* msg, Room* n, Room* s, Room* e, Room* w):message(msg),roomN(n),roomS(s),roomE(e),roomW(w){}

char* Room::GetMessage(){return message;}
Room* Room::GetN(){return roomN;}
Room* Room::GetS(){return roomS;}
Room* Room::GetE(){return roomE;}
Room* Room::GetW(){return roomW;}
