#include "Server.h"

using namespace std;

int main() {
    
    Server server;
    server.start("127.0.0.1", "65000");
    
    return 0;
}