// /*
// ** server.c -- a stream socket server demo
// ** reference from Beej's tutorial: https://beej.us/guide/bgnet/html/#a-simple-stream-server
// */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <string>
#include <iostream>
using namespace std;


#define LOCAL_HOST "127.0.0.1"
#define MAXDATASIZE 1024
#define BACKLOG 10

#define C_PORT "21218"
#define CS_PORT "22218"
#define EE_PORT "23218"
#define TCP_PORT "25218"


string course;
string category;
char *username;
int client_socket, cred_socket, cs_socket, ee_socket, child_socket, addr_info_result;



struct addrinfo hints_client, hints_c, hints_cs, hints_ee;
struct addrinfo *servinfo_client, *servinfo_c, *servinfo_cs, *servinfo_ee; 

struct sockaddr_storage client_addr, serverc_addr;
socklen_t client_addr_size, cred_addr_size;
char buf[MAXDATASIZE];
char servercs_buf[MAXDATASIZE];
char serveree_buf[MAXDATASIZE];


char serverc_buf[MAXDATASIZE];





void create_client_socket() {
    memset(&hints_client, 0, sizeof hints_client);
    hints_client.ai_family = AF_UNSPEC;
    hints_client.ai_socktype = SOCK_STREAM;
    hints_client.ai_flags = AI_PASSIVE;

    if ((addr_info_result = getaddrinfo(NULL, TCP_PORT, &hints_client, &servinfo_client)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(addr_info_result));
        return;
    }

    if ((client_socket = socket(servinfo_client -> ai_family, servinfo_client -> ai_socktype, servinfo_client -> ai_protocol)) == -1) {
        perror("server: socket");
        return;
    }

    if (bind(client_socket, servinfo_client->ai_addr, servinfo_client->ai_addrlen) == -1) {
        close(client_socket);
        perror("server: bind");
    }
    freeaddrinfo(servinfo_client);
}


void create_cred_socket() {
    memset(&hints_c, 0, sizeof hints_c);
    hints_c.ai_family = AF_UNSPEC;
    hints_c.ai_socktype = SOCK_DGRAM;
    if ((addr_info_result = getaddrinfo(LOCAL_HOST, C_PORT, &hints_c, &servinfo_c)) != 0) {
        fprintf(stderr, "cred getaddrinfo: %s\n", gai_strerror(addr_info_result));
        return;
    }
    cred_socket = socket(servinfo_c -> ai_family, servinfo_c -> ai_socktype, servinfo_c -> ai_protocol);
}

void create_cs_socket() {
    memset(&hints_cs, 0, sizeof hints_cs);
    hints_cs.ai_family = AF_UNSPEC;
    hints_cs.ai_socktype = SOCK_DGRAM;
    if ((addr_info_result = getaddrinfo(LOCAL_HOST, CS_PORT, &hints_cs, &servinfo_cs))!=0) {
        fprintf(stderr, "cs getaddrinfo: %s\n", gai_strerror(addr_info_result));
        return;
    }
    cs_socket = socket(servinfo_cs -> ai_family, servinfo_cs -> ai_socktype, servinfo_cs -> ai_protocol);
}

void create_ee_socket() {
    memset(&hints_ee, 0, sizeof hints_ee);
    hints_ee.ai_family = AF_UNSPEC;
    hints_ee.ai_socktype = SOCK_DGRAM;
    if ((addr_info_result = getaddrinfo(LOCAL_HOST, EE_PORT, &hints_ee, &servinfo_ee))!=0) {
        fprintf(stderr, "ee getaddrinfo: %s\n", gai_strerror(addr_info_result));
        return;
    }
    ee_socket = socket(servinfo_ee -> ai_family, servinfo_ee -> ai_socktype, servinfo_ee -> ai_protocol);
}
void encrypt_cred(char input[]) {
    for (int i = 0; i < strlen(input); i++) {
        int curr = int(input[i]);
        // encrypt number
        if (curr >= 48 && curr <= 57) {
            curr = curr + 4;
            curr = (curr - 48) % 10 + 48;
            input[i] = (char) curr;
        }
        // encrypt capital letter 
        else if (curr >= 65 && curr <= 90) {
            curr = curr + 4;
            curr = (curr- 65) % 26 + 65;
            input[i] = (char) curr;
        } else if (curr >= 97 && curr <= 122) {
            curr = curr + 4;
            curr = (curr - 97) % 26 + 97;
            input[i] = (char) curr;
        }
    }
}



void get_query_info(char* buf) {
    string buf_string;
    buf_string.append(buf);
    int p = buf_string.find(" ");
    course = buf_string.substr(0, p);
    category = buf_string.substr(p + 1, buf_string.size() - 1);
}

int main(void) {
    char encrypted_buf[MAXDATASIZE];
    create_client_socket();
    create_cred_socket();
    create_cs_socket();
    create_ee_socket();
    cout << "The main server is up and running." << endl;

    if (listen(client_socket, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }
    cred_addr_size = sizeof(serverc_addr);
    if ((child_socket = accept(client_socket, (struct sockaddr *)&client_addr, &client_addr_size)) == -1) {
        perror("accept");
        exit(1);
    }
    int cnt = 0;

    while(1) {
        if (cnt == 3) {
            cnt = 0;
            if ((child_socket = accept(client_socket, (struct sockaddr *)&client_addr, &client_addr_size)) == -1) {
                perror("accept");
                exit(1);
            }
        }
        // receive from client
        cnt++;
        if (recv(child_socket, buf, MAXDATASIZE, 0) == -1) {
            perror("recv");
            exit(1);
        }

        strncpy(encrypted_buf, buf, sizeof(buf));
        encrypt_cred(encrypted_buf);
        char copy[MAXDATASIZE];
        strncpy(copy, buf, sizeof(buf));
        username = strtok(copy, " ");
        
        cout << "The main server received the authentication for " << username << " using TCP over port "<< TCP_PORT << "." << endl;
        
        // send authentication request to server C
        cred_addr_size = sizeof(serverc_addr);
        if (sendto(cred_socket, encrypted_buf, MAXDATASIZE, 0, servinfo_c -> ai_addr, servinfo_c -> ai_addrlen) == -1) {
            perror("sendto");
            exit(1);
        }
        cout << "The main server sent an authentication request to serverC." << endl;

        // receive result from server C
        if(recvfrom(cred_socket, serverc_buf, MAXDATASIZE, 0, servinfo_c -> ai_addr, &(servinfo_c -> ai_addrlen)) == -1) {
            perror("recvfrom");
            exit(1);
        }
        cout << "The main server received the result of the authentication request from ServerC using UDP over port " << C_PORT << "." << endl;

        // send the authentication from serverC to client
        if(send(child_socket, serverc_buf, sizeof(serverc_buf), 0) == -1) {
            perror("serverc: send");
            exit(1);
        }
        cout << "The main server sent the authentication result to the client." << endl;
        // if authentication successful, go to course query part
        if(strcmp(serverc_buf, "0") == 0) {
            break;
        }
    }
    // begin to query about course
    while(1) {
        // getting query from client
        recv(child_socket, buf, MAXDATASIZE, 0);
        get_query_info(buf);
        cout << "The main server received from " << username << " to query course " << course << " about " << category;
        cout << "using TCP over port" <<  TCP_PORT << endl;
        // deal with query for EE
        if(buf[0] == 'E') {
            sendto(ee_socket, buf, MAXDATASIZE, 0, servinfo_ee -> ai_addr, servinfo_ee -> ai_addrlen);
            cout << "The main server sent a request to serverEE." << endl;
            recvfrom(ee_socket, serveree_buf, MAXDATASIZE, 0, servinfo_ee -> ai_addr, &(servinfo_ee -> ai_addrlen));
            cout << "The main server received the response from serverEE using UDP over port " << EE_PORT "." << endl;
            send(child_socket, serveree_buf, sizeof(serveree_buf), 0);
            cout << "The main server sent the query information to the client." << endl;
        } else { // deal with query for CS
            sendto(cs_socket, buf, MAXDATASIZE, 0, servinfo_cs -> ai_addr, servinfo_cs -> ai_addrlen);
            cout << "The main server sent a request to serverCS." << endl;
            recvfrom(cs_socket, servercs_buf, MAXDATASIZE, 0, servinfo_cs -> ai_addr, &(servinfo_cs -> ai_addrlen));
            cout << "The main server received the response from serverCS using UDP over port " << CS_PORT "." << endl;
            send(child_socket, servercs_buf, sizeof(servercs_buf), 0);
            cout << "The main server sent the query information to the client." << endl;       
        }
    }

}