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
#include <SFML/Network.hpp>

#define HOST "tcp://127.0.0.1:3306"
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
    sf::TcpSocket sock;
    sock.connect(HOST, PORT, sf::seconds(15.f));


    return 0;
}
