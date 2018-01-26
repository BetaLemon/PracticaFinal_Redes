#include "Room.hpp"

Room::Room(){
    message = "";
    roomN = nullptr;
    roomS = nullptr;
    roomE = nullptr;
    roomW = nullptr;
}
Room::Room(std::string msg, Room* n, Room* s, Room* e, Room* w):message(msg),roomN(n),roomS(s),roomE(e),roomW(w){}

std::string Room::GetMessage(){return message;}
Room* Room::GetN(){return roomN;}
Room* Room::GetS(){return roomS;}
Room* Room::GetE(){return roomE;}
Room* Room::GetW(){return roomW;}
