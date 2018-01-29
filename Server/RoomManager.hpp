#include "Room.hpp"
#include <vector>

#define RM RoomManager::Instance()

class RoomManager
{
public:
    static RoomManager* Instance();
    ~RoomManager();

    inline Room* GetRoomAtIndex(int i){return rooms[i];}

private:
    RoomManager();
    static RoomManager* instance;
    std::vector<Room*> rooms;

};
