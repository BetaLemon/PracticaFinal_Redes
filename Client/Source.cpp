#ifndef DEBUG
    #define PUGIXML HEADER ONLY
#endif  // DEBUG

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <SFML/Network.hpp>

#define HOST "10.38.0.163"
#define PORT 50000
#define MAX_BUFF_SIZE 100

// Una función que permite convertir un int a string (no me funcionaba to_string()):
template <typename T>
std::string NumToStr ( T Number )
{
 std::ostringstream ss;
 ss << Number;
 return ss.str();
}

std::string Receive(sf::TcpSocket* sock)
{
    sf::Socket::Status status;
    size_t received;
    char buffer[MAX_BUFF_SIZE];
    status = sock->receive(buffer, MAX_BUFF_SIZE, received);
    buffer[received]='\0';
    std::string tmp;
    if(status != sf::Socket::Done)
    {
        tmp = NumToStr(status);
        return tmp;
    }

    else
        return buffer;

}

void Send(sf::TcpSocket* s, std::string msg)
{
    s->send(msg.c_str(), msg.length());
}

int main(){
    sf::Socket::Status status;
    sf::TcpSocket sock;

    status = sock.connect(HOST, PORT, sf::seconds(15.f));

    if (status != sf::Socket::Done)
    {
        std::cout << "Failed to connect!"<<std::endl;
    }
    else{
        std::cout << "Connected with status: "<<status<<std::endl;
    }

    std::size_t receivedSize;
    std::string str;


    std::string in;
    //sock.send(str.c_str(), str.length());

    while(true)
    {
        std::string rec = Receive(&sock);
        //std::cout << rec;

        if(rec[0] == '!'){
            rec.erase(0,1);
            std::cout << rec;
            Send(&sock, "!");
        }
        else{
            std::cout << rec;
            std::cin >> in;
            Send(&sock, in);
        }

/*

        if(rec.at(0) != '!'){
            std::cout << rec;
            std::cin >> in;
            Send(&sock, in);
        }
        else{
            rec.erase(0,1);
            std::cout << rec;
        }*/

    }

/*
    char buffer[MAX_BUFF_SIZE];
    sock.receive(buffer, MAX_BUFF_SIZE, receivedSize);
    buffer[MAX_BUFF_SIZE]='\0';
    str = buffer;

    std::cout << status;

    while(true);*/
    return 0;
}
