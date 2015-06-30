//
// Created by MarioJ on 25/06/15.
//

#include "Server.h"

void *get_in_addr(struct sockaddr *sa) {
    
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void Server::start(string host, string port) {
    
    int sockfd = 0, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    int rv;
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP
    
    if ((rv = getaddrinfo(NULL, port.c_str(), &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return;
    }
    
    // loop through all the results and bind to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next) {
        
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }
        
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                       sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }
        
        if (::bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }
        
        break;
    }
    
    if (p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        return;
    }
    
    freeaddrinfo(servinfo); // all done with this structure
    
    if (listen(sockfd, 20) == -1) {
        perror("listen");
        exit(1);
    }
    
    sin_size = sizeof their_addr;
    
    pthread_t t_handler_client;
    
    cout << "> Tracker started" << endl;
    cout << "> Waiting connections" << endl;
    
    while (1) {  // main accept() loop
        
        new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &sin_size);
        
        if (new_fd == -1) {
            perror("accept");
            continue;
        }
        
        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *) &their_addr), s, sizeof s);
        string addr(s);
        
        Helper helper;
        helper.server = this;
        helper.socket = new_fd;
        helper.addr = addr;
        
        pthread_create(&t_handler_client, NULL, &Server::start_handler_request, &helper);
        
    }
    
}

void *Server::start_handler_request(void *o) {
    
    cout << endl;
    
    Helper* helper = (Helper*) o;
    
    Server* server = helper->server;
    int socket = helper->socket;
    string addr = helper->addr;
    
    // get data from client
    string request_data = server->receive(socket);
    
    cout << "> [Node]: " << request_data << endl;
    
    // process request
    server->process(socket, request_data);
    
    // close socket client, work done !
    close(helper->socket);
    
    return NULL;
}

void Server::send(int socket, string message) {
    
    if (::send(socket, message.c_str(), message.length(), 0) == -1) {
        cout << "error to send " << message << endl << endl;
    }
}

void Server::end_message(int socket) {
    send(socket, NET_EOM);
}

string Server::receive(int socket) {
    
    string message("");
    long numbytes;
    char buffer[MAX_DATA_SIZE];
    
    while ((numbytes = recv(socket, buffer, MAX_DATA_SIZE - 1, 0)) > 0) {
        
        message.append(buffer, numbytes);
        
        if (message.find(NET_EOM) != string::npos)
            break;
        
    }
    
    return message.substr(0, message.length() - strlen(NET_EOM));
    
}

void Server::process(int socket, string data) {
    
    Tokenizer str(data);
    string token = str.next();
    
    if (token.compare(NEW_NODE) == 0) {
        
        string addr = str.next();
        addr.append(" ").append(str.next());
        
        if (!node_exists(addr)) {
            add_node(addr);
        } else {
            cout << "> node exists " << addr << endl;
        }
        
        // make response from client with all nodes active on tracker
        string nodes_current("-nodes ");
        nodes_current.append(get_nodes()).append(NET_EOM);
        
        // send to client the current nodes in tracket
        send(socket, nodes_current);
        
    } else if (token.compare(GET_NODES) == 0) {
        // TODO GET NODES
    }
    
    cout.flush();
    
}

bool Server::node_exists(string node) {
    
    list<string>::iterator it;
    
    for (it = nodes.begin(); it != nodes.end(); it++) {
        
        if ((*it).compare(node) == 0)
            return true;
        
    }
    
    return false;
}

void Server::add_node(string node) {
    nodes.push_back(node);
}

string Server::get_nodes() {
    
    string nodes_str("");
    list<string>::iterator it;
    
    for (it = nodes.begin(); it != nodes.end(); it++)
        nodes_str.append(*it).append(LIST_SEPARATOR);
    
    if (nodes_str.length() > 0)
        return nodes_str.substr(0, nodes_str.length() - 1);
    else
        return nodes_str;
}



