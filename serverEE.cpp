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
#include <vector>
#include <sstream>
using namespace std;


#define LOCAL_HOST "127.0.0.1"
#define MAXBUFSIZE 1024
#define PORT "23218"



int serverEE_udp_socket;
int addr_info_result, udp_bind_result, serverEE_recv_result, serverEE_send_result;
struct addrinfo hints;
struct addrinfo *servinfo;
struct sockaddr_storage their_addr;
socklen_t addr_len;
string course;
string category;
char buf[MAXBUFSIZE];
char result_buf[MAXBUFSIZE];




void create_serveree_udp_socket() {
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
    if((addr_info_result = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(addr_info_result));
        return;
    }
    

    if((serverEE_udp_socket = socket(servinfo -> ai_family, servinfo -> ai_socktype, servinfo -> ai_protocol)) == -1) {
        perror("server: socket");
        exit(1);
    }

    if((udp_bind_result = bind(serverEE_udp_socket, servinfo->ai_addr, servinfo->ai_addrlen)) == -1) {
        close(serverEE_udp_socket);
        perror("server: bind");
    }


    freeaddrinfo(servinfo);
}

string generate_meg(string course, string category, string info) {
    cout << "The course information has been found: The " << category << " of " << course << " is " << info << "." << endl;
    return "The " + category + " of " + course + " is " + info + ".";
}

string query(string course, string category) {
    fstream newfile;
    newfile.open("ee.txt",ios::in); //open a file to perform read operation using file object
    if (newfile.is_open()){ //checking whether the file is open
        string tp;
        while(getline(newfile, tp)){ //read data from file object and put it into string.
            // filter the control character at end of each line
            if(int(tp[tp.length() - 1]) <= 32) {
                tp = tp.substr(0, tp.length() - 1);
            }
            vector<string> v;
            stringstream ss(tp);
            while (ss.good()) {
                string substr;
                getline(ss, substr, ',');
                v.push_back(substr);
            }
            string curCourse = v[0];
            string curCredit = v[1];
            string curProf = v[2];
            string curDay = v[3];
            string curName = v[4];
            if (course == curCourse) {
                if(category == "Credit") {
                    return generate_meg(course, category, curCredit);
                    // result = "The Credits of " + course + " is " + curCredit + ".";
                    // cout << "The course information has been found: The " << category << " of " << course << " is " << curCredit << "." << endl;

                }
                else if(category == "Professor") {
                    return generate_meg(course, category, curProf);
                } 
                else if(category == "Days") {
                    return generate_meg(course, category, curDay);
                }
                else if(category == "CourseName") {
                    return generate_meg(course, category, curName);
                }
                break;
            }
            
        }
    newfile.close(); //close the file object.
    }
    cout << "Didn't find the course: " + course << endl;
    return "Didn't find the course: " + course;
}

int main() {
    
    create_serveree_udp_socket();
    cout << "The Server EE is up and running using UDP on port " << PORT << "." << endl;
    while(1) {
        // receive the query 
        addr_len = sizeof(their_addr);
        serverEE_recv_result = recvfrom(serverEE_udp_socket, buf, MAXBUFSIZE - 1, 0, (struct sockaddr *) &their_addr, &addr_len);
        string buf_string;
        buf_string.append(buf);
        int p = buf_string.find(" ");
        course = buf_string.substr(0, p);
        category = buf_string.substr(p + 1, buf_string.size() - 1);
        cout << "The Server EE received a request from the Main Server about the ";
        cout << category << " of " << course << "." << endl;
        // find the information
        string result = query(course, category);
        strncpy(result_buf, result.c_str(), MAXBUFSIZE);
        // send the result
        serverEE_send_result = sendto(serverEE_udp_socket, result_buf, MAXBUFSIZE - 1, 0, (struct sockaddr *) &their_addr, addr_len);
        cout << "The Server EE finished sending the response to the Main Server." << endl;
    }
}