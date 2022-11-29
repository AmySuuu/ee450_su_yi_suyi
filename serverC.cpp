// /*
// ** server.c -- a stream socket server demo
// ** reference from Beej's tutorial: https://beej.us/guide/bgnet/html/#a-simple-stream-server
// */
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <errno.h> 
#include <string.h> 
#include <netdb.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <sys/wait.h>
#include <string>
#include <signal.h>
#include <sys/wait.h>
#include <fstream>
#include <iostream>
using namespace std;


#define LOCAL_HOST "127.0.0.1"
#define MAXDATASIZE 1024
#define PORT "21218"

#define INVALID_USER '2'
#define INVALID_PASS '1'
#define VALID '0'



int cred_socket;
int addr_info_result;
struct addrinfo hints, *serveinfo;
struct sockaddr_storage their_addr;

char buf[MAXDATASIZE];



void create_socket() {
    memset(&hints, 0, sizeof hints); // make sure struct is empty
    hints.ai_family = AF_UNSPEC; // accept both IPv4 and IPv6
    hints.ai_socktype = SOCK_DGRAM; // UDP stream sockets
    hints.ai_flags = AI_PASSIVE; // help filling IP 
    if ((addr_info_result = getaddrinfo(NULL, PORT, &hints, &serveinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(addr_info_result));
        return;
    }
    

    if((cred_socket = socket(serveinfo -> ai_family, serveinfo -> ai_socktype, serveinfo -> ai_protocol)) == -1) {
        perror("server: socket");
        exit(1);
    }


    if (bind(cred_socket, serveinfo->ai_addr, serveinfo->ai_addrlen) == -1) {
        close(cred_socket);
        perror("server: bind");
    }


    freeaddrinfo(serveinfo);
    
}
char authenticate(char* buf) {
    fstream newfile;
    string buf_string;
    buf_string.append(buf);
    int p = buf_string.find(" ");
    string username = buf_string.substr(0, p);
    string password = buf_string.substr(p + 1, buf_string.size() - 1);
    newfile.open("cred.txt",ios::in); //open a file to perform read operation using file object
    if (newfile.is_open()) { //checking whether the file is open
        string tp;
        while(getline(newfile, tp)){ //read data from file object and put it into string.
            // filter the control character at end of each line
            if(int(tp[tp.length() - 1]) <= 32) {
                tp = tp.substr(0, tp.length() - 1);
            }
            int del = tp.find(",");
            string cur_username = tp.substr(0, del);
            string cur_password = tp.substr(del + 1, tp.length() - 1);
            if (cur_username == username) {
                if (cur_password == password) {
                    return VALID;
                } else {
                    return INVALID_PASS;
                }
            }
        }
    newfile.close(); //close the file object.
    }
    return INVALID_USER;
}





int main() {
    socklen_t addr_len;
    char auth_result[1];
    

    create_socket(); // create socket
    cout << "The ServerC is up and running using UDP on port " << PORT << "." << endl;
    addr_len = sizeof(their_addr);

    while(1) {
        // receive query from main server
        if(recvfrom(cred_socket, buf, MAXDATASIZE - 1, 0, (struct sockaddr *) &their_addr, &addr_len) == -1) {
            perror("recvfrom");
            exit(1);
        }
        cout << "The ServerC received an authentication request from the Main Server." << endl;
        // authenticate the result
        auth_result[0] = authenticate(buf);
        // send the authenticate result to others
        if (sendto(cred_socket, auth_result, 1, 0, (struct sockaddr *) &their_addr, addr_len) == -1) {
            perror("sendto");
            exit(1);
        }
        cout << "The ServerC finished sending the response to the Main Server." << endl;
    }

    close(cred_socket);
    return 0;

}