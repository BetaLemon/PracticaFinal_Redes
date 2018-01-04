#include <iostream>
#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <string>

#define HOST "tcp://127.0.0.1:3306"
#define USER "root"
#define PASSWORD "eucaliptus"
#define DATABASE "MUDGAMEDB"

bool compruebaUsuario(std::string usuario, sql::Statement* statem){
    sql::ResultSet* res = statem->executeQuery("SELECT PlayerName FROM Players WHERE PlayerName='" + usuario + "'");
    return(res->next());
}

bool compruebaUsuarioPassword(std::string usuario, std::string contras, sql::Statement* statem){
    // Selecciona la entrada en la que el usuario y la contraseña coincidan.
    sql::ResultSet* res = statem->executeQuery("SELECT PlayerName, PlayerPassword FROM Players WHERE PlayerName='" + usuario +"' AND PlayerPassword='" + contras + "'");
    return (res->next());
}

bool LOGIN(sql::Statement* stmt){
    bool isLoggedIn = false;    // Para el control del bucle. (si está Logged In o no).
    bool needsRegister = false; // Para hacer que el usuario se registre.
    std::cout << "LOGIN" << std::endl;
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
            isLoggedIn = true;
            needsRegister = false;
        }
        // Si no está, imprime que no existe.
        else{
            char input;
            std::cout << "El usuario " << user << " no existe." << std::endl;
            std::cout << "¿Quieres registrarte? Y/n: ";
            std::cin >> input;
            if(input == 'y' || input == 'Y'){
                needsRegister = true;
                break;
            }
            else if(input == 'n' || input == 'N'){
                needsRegister = false;
                break;
            }
            else{ std::cout << "Respuesta no válida." << std::endl; }
        }
    }
    return needsRegister;  // Devuelve si necesita registrar.
}

void REGISTER(sql::Statement* stmt){
    std::string user, passwd, checkPasswd;  // Almacena temporalmente la información del usuario.
    bool passwdMatches = false;             // Booleano para controlar si la contraseña ha coincidido. Está en false para que entre una vez al while.
    std::cout << "Vas a registrarte. Por favor, introduce la siguiente información:" << std::endl;
    std::cout << "Nombre de usuario: ";
    std::cin >> user;   // El usuario introduce su nombre de usuario
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
}

int main(){
    try{
        sql::Driver* driver = get_driver_instance();
        sql::Connection* con = driver->connect(HOST, USER, PASSWORD);
        con->setSchema(DATABASE);

        sql::Statement* stmt = con->createStatement();

        if(LOGIN(stmt)){ REGISTER(stmt); }

        std::cout << "LISTA DE PERSONAJES" << std::endl;


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
