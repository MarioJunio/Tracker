//
// Created by MarioJ on 25/06/15.
//

#ifndef SERVER_TRACKER_SERVER_H
#define SERVER_TRACKER_SERVER_H

#include <iostream>
#include <list>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>
#include <pthread.h>
#include "Tokenizer.h"

#define MAX_DATA_SIZE 1024
#define NET_EOM "\r\n\r\n"

#define NEW_NODE "-new"
#define GET_NODES "-nodes"
#define LIST_SEPARATOR ","

using namespace std;

class Server {
    
private:
    list<string> nodes;
    
public:
    void start(string, string);
    static void* start_handler_request(void*);
    void send(int, string);
    void end_message(int);
    string receive(int);
    void process(int, string);
    
    void add_node(string);
    bool node_exists(string);
    string get_nodes();
};

typedef struct {
    Server* server;
    int socket;
    string addr;
} Helper;


#endif //SERVER_TRACKER_SERVER_H
