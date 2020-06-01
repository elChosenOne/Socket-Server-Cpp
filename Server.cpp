#define _WIN32_WINNT 0x601
#include <iostream>
#include <Ws2tcpip.h>
#include <string.h>
#include <chrono>
#include <future>
using namespace std;

int Recivir(SOCKET);    //Funcion que recive datos del cliente
string ConsLinea();     //Funcion que recive un string de parte del usuario y lo retorna

int main(){
    //Iniciando el Winsock
    WSADATA wsData;
    int a = 0;
    unsigned long Modo = 1;
    WORD ver = MAKEWORD(2, 2);

    cout<<"Iniciando winsock...";
    int ws0k = WSAStartup(ver, &wsData);
    if (ws0k != 0){
        cerr << "No se puede iniciar winsock"<<endl;
        return 0;
    }
    cout<<"winsock iniciado"<<endl;

    //Creando el socket para escuchar
    cout<<"Creando el socket para escuchar...";
    SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);

    if (listening == INVALID_SOCKET){
        cerr<<"No se puede crear el socket"<<endl;
        return 0;
    }
    cout<<"El socket se creo correctamente"<<endl;

    cout<<"Servidor iniciado"<<endl;

    //Se utiliza el socket creado para recivir una coneccion via ip
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(5555);
    hint.sin_addr.S_un.S_addr = INADDR_ANY;

    bind(listening, (sockaddr*)&hint, sizeof(hint));

    //Se declara el socket para escuchar
    listen(listening, SOMAXCONN);

    //Esperando por una conexion
    sockaddr_in Client;
    int ClientSize = sizeof(Client);
    SOCKET clientSocket;
    char host[NI_MAXHOST];          //Nombre del cliente
    char service[NI_MAXSERV];       //Puerto

    do{
        //Se acepta al cliente que este en fila
        clientSocket = accept(listening, (sockaddr*)&Client, &ClientSize);

        ZeroMemory(host, NI_MAXHOST);
        ZeroMemory(service, NI_MAXSERV);

        //Se obtiene el nombre del cliente y el puerto en que queda. Luego se pregunta al usuario si lo admite
        if (getnameinfo((sockaddr*)&Client, sizeof(Client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0){
            cout<<host<<" intentando conectarse en el puerto "<<service<<" permitir? (1. Si)"<<endl;
            cin>>a;
        }
    }while(a != 1);
    cout<<"Usuario conectado, chat listo"<<endl;
    //Se cierra el socket que escucha
    closesocket(listening);

    //Se le avisa al socket del cliente que no debe pausarse para enviar o recivir.
    ioctlsocket(clientSocket, FIONBIO, &Modo);

    //loop while. Se utiliza a para verificar si el cliente esta conectado (-1 en caso de que se desconecte)
    string Envio;
    auto fut = async(launch::async, ConsLinea);
    while(a != -1){
        a = Recivir(clientSocket);

        Envio.clear();

        //Este bloque se activa al momento en que la funcion "ConsLinea" retorne un valor
        if(fut.wait_for(chrono::seconds(0)) == future_status::ready){
            //Se obtiene el valor del retorno
            Envio = fut.get();
            if (strcmp(Envio.c_str(), "exit") == 0){
                cout<<"Desconectando..."<<endl;
                break;
            }
            if (Envio.length() > 0)
                send(clientSocket, Envio.c_str(), Envio.length()+1, 0);
            //Se vuelve a pedir un string para enviar
            fut = async(launch::async, ConsLinea);
        }
    }

    cout<<"Cerrando..."<<endl;

    //Se cierra el socket del cliente
    closesocket(clientSocket);

    //Se limpia winsock
    WSACleanup();

    return 0;
}

int Recivir(SOCKET clientSocket){
    char buf[4096];
    int bytesReceived;
    ZeroMemory(buf, 4096);
    //Se verifica que el cliente reciva datos
    bytesReceived = recv(clientSocket, buf, 4096, 0);

    //Si no se reciven datos el cliente esta desconectado
    if(bytesReceived == 0){
        cout<<"Cliente desconectado"<<endl;
        return -1;
    }

    //En caso de que el cliente envie datos estos se imprimen
    if (bytesReceived != -1)
        cout<<">"<<buf<<endl;

    return 0;
}

string ConsLinea(){
    string lin;
    getline(cin, lin);
    return lin;
}
