class Room{
public:
    Room();
    Room(char* msg, Room* n, Room* s, Room* e, Room* w);

    char* GetMessage();
    Room* GetN();
    Room* GetS();
    Room* GetE();
    Room* GetW();

private:
    char* message;
    Room* roomN;
    Room* roomS;
    Room* roomE;
    Room* roomW;
}
