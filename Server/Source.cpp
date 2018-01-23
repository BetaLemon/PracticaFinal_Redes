#ifndef DEBUG
    #define PUGIXML HEADER ONLY
#endif  // DEBUG

#include <pugixml.cpp>
#include <iostream>
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

#define HOST "tcp://127.0.0.1:3306"
#define USER "root"
#define PASSWORD "eucaliptus"
#define DATABASE "MUDGAMEDB"

// Una función que permite convertir un int a string (no me funcionaba to_string()):
template <typename T>
  std::string NumToStr ( T Number )
  {
     std::ostringstream ss;
     ss << Number;
     return ss.str();
  }

pugi::xml_document doc; // Almacena la informacion relacionada al documento XML.
std::string usuario;    // Almacena el nombre de usuario, una vez se ha logueado.

// Estructuras de datos que almacenan información de manera similar a la base de datos respectiva:
struct Raza { int id; std::string nombre; int vida_base; int fuerza_base; int velocidad_base; };
struct Personaje { std::string nombre, playerID, raceID, vida, fuerza,  velocidad, oro; };

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
void CrearPersonaje(sql::Statement* stmt){
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

    std::cout << "Elige una raza para tu nuevo personaje:" << std::endl;
    std::cout << "#   RAZA\t\t\tVIDA\tFUERZA\tVELOCIDAD" << std::endl;
    // Imprime todas las razas almacenadas en el vector de STL:
    for(int i = 0; i < razas.size(); i++){
        std::cout << i+1 << " - " << razas[i].nombre << "\t\t\t" << razas[i].vida_base << "\t" << razas[i].fuerza_base << "\t" << razas[i].velocidad_base << std::endl;
    }
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
bool LOGIN(sql::Statement* stmt){
    bool isLoggedIn = false;    // Para el control del bucle. (si está Logged In o no).
    bool needsRegister = false; // Para hacer que el usuario se registre.
    std::cout << "LOGIN" << std::endl;
    // Mientras no haya iniciado su sesión:
    while(!isLoggedIn){
        // El usuario introduce el usuario:
        std::string user, passwd;
        std::cout << "Usuario: ";
        std::cin >> user;
        // Se comprueba si el nombre de usuario está en la base de datos:
        if(compruebaUsuario(user, stmt)){
        // Si está, entonces, que pregunte por la contraseña:
            bool isCorrect = false; // Para controlar el bucle.
            while(!isCorrect){
                std::cout << "Contraseña: ";
                std::cin >> passwd;
                isCorrect = compruebaUsuarioPassword(user, passwd, stmt);    // Si ha resultado, entonces es correcto.
            }
            isLoggedIn = true;      // Se ha logueado...
            needsRegister = false;  // ...por lo tanto no necesita registrarse.
            usuario = user;         // Almacenamos su nombre de usuario.
        }
        // Si no está, imprime que no existe.
        else{
            char input;
            std::cout << "El usuario " << user << " no existe." << std::endl;
            // Le preguntamos si quiere registrarse:
            std::cout << "¿Quieres registrarte? Y/n: ";
            std::cin >> input;
            if(input == 'y' || input == 'Y'){
                needsRegister = true;   // Si dice que sí, entonces necesita registrarse.
                break;
            }
            else if(input == 'n' || input == 'N'){
                needsRegister = false;  // No necesita registrarse, porque no quiere.
                break;
            }
            else{ std::cout << "Respuesta no válida." << std::endl; }
        }
    }
    return needsRegister;  // Devuelve si necesita registrar.
}

// Función que se encarga del Registro:
void REGISTER(sql::Statement* stmt){
    std::string user, passwd, checkPasswd;  // Almacena temporalmente la información del usuario.
    bool passwdMatches = false;             // Booleano para controlar si la contraseña ha coincidido. Está en false para que entre una vez al while.
    bool nameIsTaken = true;                // Booleano para controlar si el nombre de usuario ya existe. Esta en false para que entre una vez al while.
    std::cout << "Vas a registrarte. Por favor, introduce la siguiente información:" << std::endl;
    while(nameIsTaken){
        std::cout << "Nombre de usuario: ";
        std::cin >> user;   // El usuario introduce su nombre de usuario
        nameIsTaken = compruebaUsuario(user, stmt);
        if(nameIsTaken){ std::cout << "El nombre de usuario " << user << " ya existe. Prueba otro:" << std::endl;}
    }
    while(!passwdMatches){
        std::cout << "Contraseña: ";
        std::cin >> passwd;
        std::cout << "Repite la contraseña: ";
        std::cin >> checkPasswd;
        if(checkPasswd == passwd){ passwdMatches = true;}   // Si son iguales, significa que coinciden.
        else { std::cout << "Las contraseñas no coinciden. Vuelve a intentar:" << std::endl;}
    }
    // El usuario ha usado un nombre de usuario y contraseña válidos. Lo insertamos en la base de datos.
    int inserted = stmt->executeUpdate("INSERT into Players(PlayerName,PlayerPassword) values ('" + user + "', '" + passwd + "')");
    if(inserted == 1){ std::cout << "Te has registrado, " << user << ".\n"; }

    usuario = user; // Almacenamos su nombre de usuario.

    CrearPersonaje(stmt);   // Iniciamos el proceso de creación de personaje.
}

void CargarXML(){
    //pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file("jugadores.xml");
    std::cout << "Se ha cargado: " << result.description() << std::endl;
}

void recorrerNodosJugadores(){
    pugi::xml_node nodoJugadores = doc.child("jugadores");
    for(pugi::xml_node nodoJugador = nodoJugadores.child("jugador"); nodoJugador; nodoJugador = nodoJugador.next_sibling("jugador")){
        std::cout << "Se detecta un nodo 'jugador' dentro de la raiz 'jugadores'" << std::endl;
    }
}

void GestionarCliente(sf::TcpSocket *socket){
    bool playerExit = false;
    //while(!playerExit && socket->)
}

// LISTA DE DUDAS PARA JUEVES: wait(),

int main(){
    try{
        // Inicialización para MySQL:
        sql::Driver* driver = get_driver_instance();
        sql::Connection* con = driver->connect(HOST, USER, PASSWORD);
        con->setSchema(DATABASE);

        sql::Statement* stmt = con->createStatement();

        // Lógica del servidor:
        sf::TcpListener listener;
        listener.listen(50000);
        bool gameRunning = true;
        std::vector<sf::TcpSocket*> sockets;

        while(gameRunning){
            sf::TcpSocket socket;
            listener.accept(socket);
            sockets.push_back(&socket);
            if(fork() == 0){
                GestionarCliente(&socket);
                exit(0);
            }
        }


        if(LOGIN(stmt)){ REGISTER(stmt); }  // Si el usuario ha elegido registrarse, LOGIN devuelve true, para ejecutar REGISTER().

        std::cout << "Empieza el juego" << std::endl;

        //CargarXML();
        //recorrerNodosJugadores();

        delete(stmt);
        delete(con);

        system("pause");

        /* EJEMPLOS DE SELECT Y SELECT COUNT

        sql::ResultSet* res = stmt->executeQuery("SELECT PlayerName, PlayerPassword from Players");
        std::cout << "USERNAME      |       USERPASSWORD" << std::endl;
        while(res->next()){
            std::cout << res->getString("PlayerName") << "    |    " << res->getString("PlayerPassword") << std::endl;
        }
        delete(res);
        res = stmt->executeQuery("SELECT count(*) FROM Players WHERE PlayerName='player1' and PlayerPassword = '1234'");
        if(res->next()){
            int existe = res->getInt(1);
            if(existe == 1){ std::cout << "El usuario existe en la BD. La autenticación es correcta." << std::endl;}
            else{ std::cout << "El usuario NO existe en la BD." << std::endl;}
        }
        delete(res);
        delete(stmt);
        delete(con);*/

        /* EJEMPLO de INSERT Y UPDATE
        int numRows = stmt->executeUpdate("INSERT into Players(PlayerName,PlayerPassword) values ('player3', '1234')");
        if(numRows == 1){ std::cout << "Se ha insertado un nuevo usuario\n"; }
        delete(stmt);
        delete(con);*/
    }
    catch(sql::SQLException &e){
        std::cout << "Se produce el error " << e.getErrorCode() << std::endl;
    }
    return 0;
}
