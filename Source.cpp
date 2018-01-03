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

int main(){
    try{
        sql::Driver* driver = get_driver_instance();
        sql::Connection* con = driver->connect(HOST, USER, PASSWORD);
        con->setSchema(DATABASE);

        sql::Statement* stmt = con->createStatement();

        // El usuario introduce el usuario:
        std::string user, passwd;
        std::cout << "LOGIN" << std::endl << "Usuario: ";
        std::cin >> user;
        // Se comprueba si el nombre de usuario está en la base de datos:
        sql::ResultSet* res = stmt->executeQuery("SELECT PlayerName FROM Players WHERE PlayerName='" + user + "'");
        // Si está, entonces, que pregunte por la contraseña:
        if(res->next()){
            bool isCorrect = false; // Para controlar el bucle.
            while(!isCorrect){
                std::cout << "Contraseña: ";
                std::cin >> passwd;
                // Selecciona la entrada en la que el usuario y la contraseña coincidan.
                res = stmt->executeQuery("SELECT PlayerName, PlayerPassword FROM Players WHERE PlayerName='" + user +"' AND PlayerPassword='" + passwd + "'");
                if(res->next()){ isCorrect = true; }    // Si ha resultado, entonces es correcto.
            }
        }
        // Si no está, imprime que no existe.
        else{ std::cout << "El usuario " << user << " no existe." << std::endl; }


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
