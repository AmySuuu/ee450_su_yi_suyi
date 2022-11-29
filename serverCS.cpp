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
#define MAXDATASIZE 1024
#define PORT "22218"


int cs_socket, addr_info_result;
struct addrinfo hints, *servinfo;
struct sockaddr_storage their_addr;
socklen_t addr_len;
string course, category;
char buf[MAXDATASIZE];
char result_buf[MAXDATASIZE];




void create_socket() {
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
    if((addr_info_result = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(addr_info_result));
        return;
    }
    
    if((cs_socket = socket(servinfo -> ai_family, servinfo -> ai_socktype, servinfo -> ai_protocol)) == -1) {
        perror("server: socket");
    }


    if(bind(cs_socket, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
        close(cs_socket);
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
    newfile.open("cs.txt",ios::in); //open a file to perform read operation using file object
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
    create_socket();
    cout << "The Server CS is up and running using UDP on port " << PORT << "." << endl;
    while(1) {
        // getting query from main server
        addr_len = sizeof(their_addr);
        if (recvfrom(cs_socket, buf, MAXDATASIZE - 1, 0, (struct sockaddr *) &their_addr, &addr_len) == -1) {
            perror("recvfrom");
            exit(1);
        }
        string buf_string;
        buf_string.append(buf);
        int p = buf_string.find(" ");
        course = buf_string.substr(0, p);
        category = buf_string.substr(p + 1, buf_string.size() - 1);
        cout << "The Server CS received a request from the Main Server about the " << category << " of " << course << "." << endl;
        // respond the query
        string result = query(course, category);
        strncpy(result_buf, result.c_str(), MAXDATASIZE);
        // send the query to main server
        if (sendto(cs_socket, result_buf, MAXDATASIZE - 1, 0, (struct sockaddr *) &their_addr, addr_len) == -1) {
            perror("sendto");
            exit(1);
        }
        cout << "The Server CS finished sending the response to the Main Server." << endl;
    }
}