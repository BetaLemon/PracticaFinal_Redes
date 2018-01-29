#ifndef DEBUG
    #define PUGIXML HEADER ONLY
#endif  // DEBUG

#include <iostream> // Para cin y cout.
#include <string>   // Para tratar con el texto, tanto para imprimir, como para input.
#include <vector>  
#include <sstream>  // Para NumToStr().
#include <SFML/Network.hpp> // Para el protocolo TCP y la conexión al servidor.

#define HOST "10.38.0.163"  // Dirección IP del servidor.
#define PORT 50000          // Puerto a través del cual escucha el servidor.
#define MAX_BUFF_SIZE 1024  // Tamaño en bytes máximo con el que recibe información del servidor.

// El cliene es muy sencillo. Simplemente imprime por pantalla lo que recibe del servidor
// y envía aquello que el usuario introduce. Todo esto funciona de manera ordenada.

// Una función que permite convertir un int a string (no me funcionaba to_string()):
template <typename T>
std::string NumToStr ( T Number )
{
 std::ostringstream ss;
 ss << Number;
 return ss.str();
}

// Esta función encapsula la recepción de información:
std::string Receive(sf::TcpSocket* sock)    // Le pasamos el socket como puntero, pues no es copiable.
{
    sf::Socket::Status status;  // Variable que acogerá el estado del socket.
    size_t received;            // Almacenaremos el tamaño en bytes de información recibida.
    char buffer[MAX_BUFF_SIZE]; // Buffer que almacenará la información recibida.
    status = sock->receive(buffer, MAX_BUFF_SIZE, received);    // A través del socket recibimos la información.
    buffer[received]='\0';  // Añadimos al final el '\0' para que se acabe el texto allí, y que el string no se llene de basura.
    std::string tmp;        // Sólo se usa para imprimir el error...
    if(status != sf::Socket::Done)
    {
        tmp = NumToStr(status); // ... que se produce si se desconecta del servidor.
        return tmp;
    }

    else
        return buffer;  // Si no hay error, convertimos el buffer a string, para posteriormente imprimirlo.

}

// Esta función encapsula el envío de información al servidor:
void Send(sf::TcpSocket* s, std::string msg)  // Le pasamos el socket y el mensaje que queremos enviar a través de éste.
{
    s->send(msg.c_str(), msg.length()); // Envía el string, convertido a array de chars, indicando su tamaño en bytes.
}

int main(){
    sf::Socket::Status status;  // Status almacena el estado del socket.
    sf::TcpSocket sock;         // El socket a través del cual nos conectaremos al servidor.

    status = sock.connect(HOST, PORT, sf::seconds(15.f));   // Intentamos conectar, con un time-out de 15s. Recogemos el estado.

    if (status != sf::Socket::Done) // Si el estado es diferente de Conectado...
    {
        std::cout << "Failed to connect!"<<std::endl;   // ... imprimimos que no nos hemos podido conectar.
    }
    else{   // Sino (ergo, está conectado)...
        std::cout << "Connected with status: "<<status<<std::endl;  // Imprimimos que nos hemos conectado con el status que será 0.
    }

    std::string in; // String que almacena el input del jugador.

    while(true)
    {
        std::string rec = Receive(&sock);   // Almacenamos en rec el string recibido (a través de sock).
        
        /* A continuación tenemos el sistema que hemos montado para poder enviar mensajes al cliente que
        no esperen una respuesta, de manera que no rompemos el ciclo, pero tampoco obligamos al jugador a teclear
        algo para enviar algo irrelevante.
        
        Funciona de la siguiente manera: en el servidor hemos añadido '!' delante de los mensajes que no requieran
        de respuesta. En el servidor, si detecta que ha enviado un mensaje de este tipo, hará un Receive de manera
        automática, dentro de la misma función Send, para recoger la respuesta del cliente.
        Así pues, en el cliente, se suprime el carácter '!' para que no se imprima, imprimimos el texto, y luego
        hacemos un Send de cualquier cosa, sin que el usuario haga ningún input. El servidor lo recibirá y pasará 
        al siguiente paso.
        
        De esta manera podemos enviar efectivamente mensajes sin respuesta del servidor al cliente. (No viceversa!)*/
        
        if(rec[0] == '!'){  // Si el string empieza por '!'...
            rec.erase(0,1); // ... suprime el primer carácter (que es '!').
            std::cout << rec;   // Imprime el texto (que ya no tendrá '!').
            Send(&sock, "!");   // Enviamos al servidor el carácter '!' que está esperando, para que se cumpla el ciclo.
                                // (Se podría enviar cualquier otro carácter).
        }
        else{   // Si el string no empieza por '!', y por tanto SÍ espera una respuesta...
            std::cout << rec;   // ...imprimimos el texto, y...
            std::cin >> in;     // ...esperamos el input del jugador.


            if(in == "exit" || in == "EXIT" || in == "quit" || in == "QUIT")    // Si el jugador ha introducido "exit"/...
            {
                break;  // ... Entonces que salga del bucle.
            }
            else Send(&sock, in);   // Sino, enviamos el input al servidor.
        }

    sock.disconnect();  // Finalmente nos desconectamos del servidor.
        
    return 0; 
}
