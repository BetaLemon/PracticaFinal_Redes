#ifndef DEBUG
    #define PUGIXML HEADER ONLY
#endif  // DEBUG

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
#include <SFML/Network.hpp>

#define HOST "127.0.0.1"
#define PORT 50000

// Una función que permite convertir un int a string (no me funcionaba to_string()):
template <typename T>
std::string NumToStr ( T Number )
{
 std::ostringstream ss;
 ss << Number;
 return ss.str();
}

int main(){
    sf::Socket::Status stat;
    sf::TcpSocket sock;
    stat = sock.connect(HOST, PORT, sf::seconds(15.f));

    std::cout << stat;


    return 0;
}
