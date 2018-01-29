#include "RoomManager.hpp"

RoomManager::RoomManager()
{

}
RoomManager::~RoomManager()
{

}

RoomManager* RoomManager::Instance()
{
    if(instance == 0)
        instance = new RoomManager();
    return instance;
}
