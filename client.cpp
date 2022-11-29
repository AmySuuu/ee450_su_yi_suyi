// reference from Beej's tutorial
// https://beej.us/guide/bgnet/html/#a-simple-stream-client

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
#include <string>

using namespace std;


#define LOCAL_HOST "127.0.0.1"
#define TCP_PORT "25218"
#define MAXDATASIZE 1024

#define INVALID_USER "0"
#define INVALID_PASS "1"
#define VALID "2"


int addr_info, send_result, recv_result, tcp_socket, tcp_port;
struct addrinfo hints;
struct addrinfo *servinfo;
char cred_send_buf[MAXDATASIZE], cred_recv_buf[MAXDATASIZE], course_send_buf[MAXDATASIZE], course_recv_buf[MAXDATASIZE];
string username;



void create_connection() {
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    addr_info = getaddrinfo(LOCAL_HOST, TCP_PORT, &hints, &servinfo);

    if(addr_info != 0) {
        cout << "ERROR: serverM fails to get address info" << endl;
        return;
    }

    if((tcp_socket = socket(servinfo -> ai_family, servinfo -> ai_socktype, servinfo -> ai_protocol)) == -1) {
        perror("server: socket");
        exit(1);
    }

    if (connect(tcp_socket, servinfo ->ai_addr, servinfo ->ai_addrlen) == -1) {
        close(tcp_socket);
        perror("client: connect");
        exit(1);
    }    
    tcp_port = ((struct sockaddr_in*) (void*) servinfo->ai_addr)->sin_port;

}

string getUsername() {
    string username;
    bool usercheck = true;
    //check invalid username
    while(usercheck) {
        cout << "Please enter the username: ";
        cin >> username;
        if (username.length() < 5 || username.length() > 50) {
            cout << "username is suppose to have a length of 5~50 characters, please re-enter your username." << endl;
            username = "";
        } else {
            for (int i = 0; i < username.length(); i++) {
                if ((username[i] >= 48 && username[i] <= 57) || (username[i] >= 97 && username[i] <= 122)) {
                    usercheck = false;
                } else {
                    cout << "The username should be of lower case characters, please re-enter your username." << endl;
                    usercheck = true;
                    username = "";
                    break;
                }
            }
        }   
    }
    return username;
}
string getPassword(){
    
    string password;
    
    bool passcheck = true;
    
    // check invalid password
    while(passcheck) {
        cout << "Please enter the password: ";
        cin >> password;
        if (password.length() < 5 || password.length() > 50) {
            cout << "password is suppose to have a length of 5~50 characters, please re-enter your password." << endl;
        } else {
            passcheck = false;
        }
    }
    return password;
}

string getCourseInfo() {
    string course, query;
    while(1) {
        cout << "Please enter the course code to query: ";
        cin >> course;
        if (course.substr(0,2) == "CS" || course.substr(0,2) == "EE") {
            break;
        } else {
            cout << "Please enter course code start with 'CS' or 'EE" << endl;
        }
    }
    
    cout << "Please enter the category (Credit / Professor / Days / CourseName): ";
    cin >> query;
    return course + " " + query;
}


int main(int argc, char *argv[]) {
    int attempt = 3;
    create_connection();
    cout << "The client is up and running." << endl;
    string username = getUsername();
    string password = getPassword();
    string user_info = username + ' ' + password;
    strncpy(cred_send_buf, user_info.c_str(), MAXDATASIZE);
    cout << username << " sent an authentication request to the main server." << endl;
    // Phase 1A: Client sends the authentication request to the main server over TCP connection
    while(attempt >= 0) {
        attempt --;
        if(attempt == -1) {
            cout << "Authentication Failed for 3 attempts. Client will shut down." << endl;
            exit(1);
        } else if (attempt < 2){
            user_info = getUsername() + ' ' + getPassword();
            strncpy(cred_send_buf, user_info.c_str(), MAXDATASIZE);
        }
        if ((send_result = send(tcp_socket, cred_send_buf, sizeof(cred_send_buf), 0)) == -1) {
            perror("send");
            exit(1);
        }
        if ((recv_result = recv(tcp_socket, cred_recv_buf, MAXDATASIZE, 0)) == -1) {
            perror("recv");
            exit(1);
        }
        if(strcmp(cred_recv_buf, "0") == 0) { // if the credential is authenticate successfully
            cout << username << " received the result of authentication using TCP over port " << tcp_port << ". Authentication is successful" << endl;
            while(1) {
                string courseInfo = getCourseInfo();  // get course info
                strncpy(course_send_buf, courseInfo.c_str(), MAXDATASIZE);
                send(tcp_socket, course_send_buf, sizeof(cred_send_buf), 0);
                cout << username << " sent a request to the main server." << endl;
                recv(tcp_socket, course_recv_buf, MAXDATASIZE, 0);
                cout << "The client received the response from the Main server using TCP over port " << tcp_port << "." << endl;
                cout << course_recv_buf << endl;
                cout << "\n\n";
                cout << "-----Start a new request-----" << endl;
            }
        } else {
            if (strcmp(cred_recv_buf, "1") == 0) {
                cout << username << " received the result of authentication using TCP over port ";
                cout << tcp_port << ". Authentication failed: ";
                cout << "Password Does not exist\n" << endl;
            } else {
                cout << username << " received the result of authentication using TCP over port ";
                cout << tcp_port << ". Authentication failed: ";
                cout << "Username does not match\n" << endl;
            }
        }
        cout << "Attempts remaining:" << attempt << "\n" << endl;
        memset(&cred_recv_buf, 0, sizeof(cred_recv_buf));
        
    }
}

