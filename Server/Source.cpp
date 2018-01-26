#ifndef DEBUG
    #define PUGIXML HEADER ONLY
#endif  // DEBUG

#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pugixml.cpp>
#include <iostream>
#include <list>
#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <string>
#include <vector>
#include <sstream>
#include <SFML/Main.hpp>
#include <SFML/Network.hpp>
#include "RoomManager.hpp"

//#define HOST "tcp://127.0.0.1:3306"
#define HOST "tcp://127.0.0.1:3306"
#define USER "root"
#define PASSWORD "eucaliptus"
#define DATABASE "MUDGAMEDB"

#define MAX_BUFF_SIZE 100

#define DEBUG true

// Una función que permite convertir un int a string (no me funcionaba to_string()):
template <typename T>
  std::string NumToStr ( T Number )
  {
     std::ostringstream ss;
     ss << Number;
     return ss.str();
  }

template <typename T>
void debug(T txt){
    if(DEBUG){ std:: cout << txt << std::endl; }
}

pugi::xml_document doc; // Almacena la informacion relacionada al documento XML.
std::string usuario;    // Almacena el nombre de usuario, una vez se ha logueado.

// Estructuras de datos que almacenan información de manera similar a la base de datos respectiva:
struct Raza { int id; std::string nombre; int vida_base; int fuerza_base; int velocidad_base; };
struct Personaje { std::string nombre, playerID, raceID, vida, fuerza,  velocidad, oro; };

struct SharedData
{
    std::vector<Room*> rooms;
};

std::string Receive(sf::TcpSocket* s){
    char buffer[MAX_BUFF_SIZE];
    size_t receivedBytes;
    if(s->receive(buffer, MAX_BUFF_SIZE, receivedBytes) != sf::Socket::Done){ std::cout << "Error receiving data." << std::endl; }
    buffer[receivedBytes] = '\0';
    std::string tmp = buffer;
    debug(tmp + " was received.");
    return tmp;
}

void Send(sf::TcpSocket* s, std::string msg){
    s->send(msg.c_str(), msg.length());
    debug(msg);
    if(msg.at(0) == '!'){ Receive(s); }
}

// Una funcion con la que hemos encapsulado la comprobación de la existencia del nombre de usuario:
bool compruebaUsuario(std::string usuario, sql::Statement* statem){
    sql::ResultSet* res = statem->executeQuery("SELECT PlayerName FROM Players WHERE PlayerName='" + usuario + "'");
    return(res->next());
    delete(res);
}

// Otra función similar, pero que también comprueba la contraseña que se le pase (true si existe una
// combinación, o false si no existe):
bool compruebaUsuarioPassword(std::string usuario, std::string contras, sql::Statement* statem){
    // Selecciona la entrada en la que el usuario y la contraseña coincidan.
    sql::ResultSet* res = statem->executeQuery("SELECT PlayerName, PlayerPassword FROM Players WHERE PlayerName='" + usuario +"' AND PlayerPassword='" + contras + "'");
    return (res->next());
    delete(res);
}

// Función grande que se encarga de todo el proceso de creación de personaje:
void CrearPersonaje(sql::Statement* stmt, sf::TcpSocket* s){
    std::vector<Raza> razas;    // Almacenará todas las razas disponibles (obtenidas de la BD).
    int enteredRace;            // Almacena la raza que ha seleccionado el jugador.
    std::string enteredName;    // Almacena el nombre del personaje introducido por el jugador.
    bool validRace = false;     // Booleano que controla el while, y que sólo será true cuando la raza introducida exista.
    bool validName = false;     // Booleano que controla el while, para que el usuario introduzca un nombre que no existe aún.
    Personaje pers;             // Almacena toda la información del personaje nuevo. Se traduce 1 a 1 a la entrada de la BD.

    sql::ResultSet* res = stmt->executeQuery("SELECT * FROM Races");    // Queremos todas las entradas que hay en la tabla de razas.
    while(res->next()){ // Siempre que exista el siguiente (y avancemos hacia este)...
        // ... añade al vector STL de razas toda la información de la raza a la que accedemos:
        razas.push_back({res->getInt("RaceID"), res->getString("RaceName"), res->getInt("RaceBaseLife"), res->getInt("RaceBaseStrength"), res->getInt("RaceBaseSpeed")});
    }
    delete(res);    // Ya no necesitamos el resultado.

    Send(s,"!Elige una raza para tu nuevo personaje:\n"); Receive(s);
    Send(s,"!#   RAZA\t\t\tVIDA\tFUERZA\tVELOCIDAD\n");  Receive(s);
    // Imprime todas las razas almacenadas en el vector de STL:
    std::string sendStr = "";
    for(int i = 0; i < razas.size(); i++){
        sendStr += NumToStr(i+1) + " - " + razas[i].nombre + "\t\t\t" + NumToStr(razas[i].vida_base) + "\t" + NumToStr(razas[i].fuerza_base) + "\t" + NumToStr(razas[i].velocidad_base + "\n");
    }
    Send(s, "!" + sendStr);
    // Mientras no tengamos una raza válida, pide una raza al usuario:
    while(!validRace){
        std::cout << "Introduce la raza que hayas elegido (#): ";
        std::cin >> enteredRace;
        if(enteredRace <= razas.size() && enteredRace > 0){ validRace = true; } // Si la raza está dentro del rango, entonces es válida.
        else{ std::cout << "Raza no válida." << std::endl; }
    }
    enteredRace -= 1;   // Como el número que introduce el usuario está en base a lo impreso por pantalla, tenemos que rectificarlo,
                        // para que coincida con el valor respectivo del vector de razas (vector->(0,n); impreso pantalla->(1,n)).
    // Rellenamos el nuevo personaje con la información que tenemos:
    pers.raceID = NumToStr(razas[enteredRace].id);                  // la raza del personaje.
    pers.vida = NumToStr(razas[enteredRace].vida_base);             // la vida inicial.
    pers.fuerza = NumToStr(razas[enteredRace].fuerza_base);         // su fuerza inicial.
    pers.velocidad = NumToStr(razas[enteredRace].velocidad_base);   // su velocidad inicial.
    pers.oro = NumToStr(0);                                         // y su oro, que no tiene.

    // Le indicamos al usuario la raza que ha introducido:
    std::cout << "Has elejido " << razas[enteredRace].nombre << ".\n";

    // Mientras no tengamos un nombre de personaje válido, pidele uno al usuario:
    while(!validName){
        std::cout << "Introduce el nombre del personaje: ";
        std::cin >> enteredName;
        // Comprobamos si el nombre introducido existe:
        res = stmt->executeQuery("SELECT count(*) FROM Characters WHERE CharacterName='" + enteredName + "'");
        // SI hemos obtenido un resultado...
        if(res->next()){
            // ... y existe el nombre, dile al usuario que no sirve.
            if(res->getInt(1) == 1){
                std::cout << "El nombre ya existe, por favor, prueba con otro." << std::endl;
            }
            // ... o si no existe, es válido.
            else{ validName = true; }
        }
        delete(res);    // Ya no necesitamos el resultado.
    }
    // Le indicamos al usuario el nombre que ha escogido:
    std::cout << "Tu personaje se llamará " << enteredName << ".\n";
    pers.nombre = enteredName;  // Almacenamos en el personaje el nombre escogido.

    // Cuando el usuario se logueó o se registró almacenamos su nombre de usuario. Buscamos en la DB,
    // su PlayerID correspondiente.
    res = stmt->executeQuery("SELECT PlayerID FROM Players WHERE PlayerName='" + usuario + "'");
    res->next();
    pers.playerID = NumToStr(res->getInt("PlayerID"));  //Ahora que tenemos el PlayerID, lo almacenamos en el personaje.
    delete(res);    // Ya no necesitamos el resultado.

    // Esta lorza de código introduce cada una de las entradas del struct Personaje en una nueva entrada en la DB (en la tabla Characters):
    int e = stmt->executeUpdate("INSERT INTO Characters(CharacterName, PlayerID, RaceID, Life, Strength, Speed, Gold) values('"
    + pers.nombre + "','" + pers.playerID + "','" + pers.raceID + "','" + pers.vida + "','" + pers.fuerza + "','" + pers.velocidad + "','" + pers.oro + "')");

    // El personaje se ha creado correctamente. Hemos finalizado el proceso de creación.
}

// Función que se encarga del Login: ( devuelve true si el usuario necesita registrarse! )
bool LOGIN(sql::Statement* stmt, sf::TcpSocket* s){
    bool isLoggedIn = false;    // Para el control del bucle. (si está Logged In o no).
    bool needsRegister = false; // Para hacer que el usuario se registre.
    Send(s, "!LOGIN\n");
    // Mientras no haya iniciado su sesión:
    while(!isLoggedIn){
        // El usuario introduce el usuario:
        std::string user, passwd;
        Send(s,"Usuario: ");
        user = Receive(s);
        // Se comprueba si el nombre de usuario está en la base de datos:
        if(compruebaUsuario(user, stmt)){
        // Si está, entonces, que pregunte por la contraseña:
            bool isCorrect = false; // Para controlar el bucle.
            while(!isCorrect){
                Send(s, "Contraseña: ");
                passwd = Receive(s);
                isCorrect = compruebaUsuarioPassword(user, passwd, stmt);    // Si ha resultado, entonces es correcto.
            }
            isLoggedIn = true;      // Se ha logueado...
            needsRegister = false;  // ...por lo tanto no necesita registrarse.
            usuario = user;         // Almacenamos su nombre de usuario.
        }
        // Si no está, imprime que no existe.
        else{
            char input;
            // Le preguntamos si quiere registrarse:
            Send(s, "El usuario " + user + " no existe.\n¿Quieres registrarte? y/N: ");
            input = Receive(s).at(0);
            if(input == 'y' || input == 'Y'){
                needsRegister = true;   // Si dice que sí, entonces necesita registrarse.
                break;
            }
            else if(input == 'n' || input == 'N'){
                needsRegister = false;  // No necesita registrarse, porque no quiere.
                break;
            }
            else{ Send(s,"!Respuesta no válida.\n");}   // El receive sólo es para cumplir con el ciclo.
        }
    }
    return needsRegister;  // Devuelve si necesita registrar.
}

// Función que se encarga del Registro:
void REGISTER(sql::Statement* stmt, sf::TcpSocket* s){
    std::string user, passwd, checkPasswd;  // Almacena temporalmente la información del usuario.
    bool passwdMatches = false;             // Booleano para controlar si la contraseña ha coincidido. Está en false para que entre una vez al while.
    bool nameIsTaken = true;                // Booleano para controlar si el nombre de usuario ya existe. Esta en false para que entre una vez al while.
    //std::cout << "Vas a registrarte. Por favor, introduce la siguiente información:" << std::endl;
    while(nameIsTaken){
        Send(s,"Vas a registrarte. Por favor, introduce la siguiente información:\nNombre de usuario: ");
        user = Receive(s);   // El usuario introduce su nombre de usuario
        nameIsTaken = compruebaUsuario(user, stmt);
        if(nameIsTaken){ Send(s, "!El nombre de usuario " + user + " ya existe. Prueba otro:\n"); Receive(s); }
    }
    while(!passwdMatches){
        Send(s, "Contraseña: ");
        passwd = Receive(s);
        Send(s, "Repite la contraseña: ");
        checkPasswd = Receive(s);
        if(checkPasswd == passwd){ passwdMatches = true;}   // Si son iguales, significa que coinciden.
        else { Send(s, "!Las contraseñas no coinciden. Vuelve a intentar:\n"); Receive(s);}  // ATENCIÓN, ROMPEMOS CICLO?
    }
    // El usuario ha usado un nombre de usuario y contraseña válidos. Lo insertamos en la base de datos.
    int inserted = stmt->executeUpdate("INSERT into Players(PlayerName,PlayerPassword) values ('" + user + "', '" + passwd + "')");
    if(inserted == 1){ Send(s, "!Te has registrado, " + user + ".\n"); Receive(s); }

    usuario = user; // Almacenamos su nombre de usuario.

   // CrearPersonaje(stmt);   // Iniciamos el proceso de creación de personaje.
}

std::vector<Room*> LoadRoomsMap(){
    pugi::xml_parse_result result = doc.load_file("worldmap.xml");
    pugi::xml_node roomsNode = doc.child("rooms");
    std::vector<Room*> rooms;
    for(pugi::xml_node roomNode = roomsNode.child("room"); roomNode; roomNode = roomNode.next_sibling("room")){
        std::cout << "Se han detectado las rooms" << std::endl;
        rooms.push_back(new Room());

    }
    for(pugi::xml_node roomNode = roomsNode.child("room"); roomNode; roomNode = roomNode.next_sibling("room")){
        std::cout << roomNode.attribute("ID").as_int() << std::endl;
        Room* tmp = rooms[roomNode.attribute("ID").as_int()];
        //debug(roomNode.attribute("ID").as_int());
        std::string msg = roomNode.child_value("MSG");
        int n = atoi(roomNode.child_value("N"));
        int s = atoi(roomNode.child_value("S"));
        int e = atoi(roomNode.child_value("E"));
        int w = atoi(roomNode.child_value("W"));

        tmp->SetMSG(msg);
        n == -1 ? tmp->SetN(nullptr) : tmp->SetN(rooms[n]);
        s == -1 ? tmp->SetS(nullptr) : tmp->SetS(rooms[s]);
        e == -1 ? tmp->SetE(nullptr) : tmp->SetE(rooms[e]);
        w == -1 ? tmp->SetW(nullptr) : tmp->SetW(rooms[w]);
        //for(pugi::xml_node RoomNodeDir = RoomNodeDir.first_child(); RoomNodeDir; RoomNodeDir.next_sibling()){
        std::cout << "MSG: ";
        debug(tmp->GetMessage());
        std::cout << "N: ";
        debug(tmp->GetN());
        std::cout << "S: ";
        debug(tmp->GetS());
        std::cout << "E: ";
        debug(tmp->GetE());
        std::cout << "W: ";
        debug(tmp->GetW());

    }



    return rooms;
}

void GestionarCliente(int shmID, sql::Statement * stmt, sf::TcpSocket *socket){
    bool playerExit = false;
    SharedData* shd = (struct SharedData*)shmat(shmID, NULL, 0);
    if(shd < 0) std::cout << "[ERROR ATTACHING SHARED MEMORY]"<<std::endl;
    Room* currentRoom = shd->rooms[0];

    if(LOGIN(stmt, socket)){ REGISTER(stmt, socket); }
    debug("Game Start!");
    while(!playerExit)
    {
        //LOGIN(stmt, socket);

        while(true)
        {

            Send(socket, currentRoom->GetMessage());
            std::string tmp = Receive(socket);

            Room* prevRoom = currentRoom;

            if(tmp == "N" || tmp == "n")
            {
                currentRoom = currentRoom->GetN();
            }
            else if(tmp == "S" || tmp == "s")
            {
                currentRoom = currentRoom->GetS();
            }
            else if(tmp == "E" || tmp == "e")
            {
                currentRoom = currentRoom->GetE();
            }
            else if(tmp == "W" || tmp == "w")
            {
                currentRoom = currentRoom->GetW();
            }
            else
            {
                Send(socket, "!You can only go N, S, E or W!\n");
            }

            if(currentRoom == nullptr)
            {
                Send(socket, "!You shall not pass!\n");
                currentRoom = prevRoom;
            }

        }



    }

    shmdt(shd);
    socket->disconnect();
    //while(!playerExit && socket->)
}


// LISTA DE DUDAS PARA JUEVES: wait(),

int main(){
    try{
        //Shared Memory Reserved
        int shmID = shmget(IPC_PRIVATE, sizeof(SharedData), IPC_CREAT | 0666);
        if(shmID < 0) std::cout << "[FAILED RESERVING SHARED MEMORY]"<< std::endl;
        SharedData* shd = (struct SharedData*)shmat(shmID, NULL, 0);
        if(shd < 0) std::cout << "[ERROR ATTACHING SHARED MEMORY]"<< std::endl;
        shd->rooms = LoadRoomsMap();
        // Inicialización para MySQL:
        sql::Driver* driver = get_driver_instance();
        sql::Connection* con = driver->connect(HOST, USER, PASSWORD);
        con->setSchema(DATABASE);

        sql::Statement* stmt = con->createStatement();

        //RoomManager::Instance()->
        // Lógica del servidor:
        std::cout << "Listening..." << std::endl;
        sf::TcpListener listener;
        listener.listen(50000);
        bool gameRunning = true;

        //SharedData shd;
        std::vector<sf::TcpSocket*> sockets;

        while(gameRunning){
            sf::TcpSocket socket;
            std::cout << "Waiting for new connection..." << std::endl;
            listener.accept(socket);
            std::cout << "New Socket Accepted from: "<< socket.getRemoteAddress() <<":"<<socket.getRemotePort()<<std::endl;
            sockets.push_back(&socket);
            if(fork() == 0){
                GestionarCliente(shmID, stmt, &socket);
                exit(0);
            }
        }

        for(int i = 0; i < sockets.size(); i++)
        {
            sockets[i]->disconnect();
        }
        listener.close();
        //if(LOGIN(stmt)){ REGISTER(stmt); }  // Si el usuario ha elegido registrarse, LOGIN devuelve true, para ejecutar REGISTER().

        //std::cout << "Empieza el juego" << std::endl;

        //CargarXML();
        //recorrerNodosJugadores();

        delete(stmt);
        delete(con);

        system("pause");

    }
    catch(sql::SQLException &e){
        std::cout << "Se produce el error " << e.getErrorCode() << std::endl;
    }
    return 0;
}
