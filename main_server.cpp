/**
 * @file main_server.cpp
 *
 * @brief Server application entry point.
 * 
 */

#include "server.h"

namespace {

server_handler::ServerHandler ser_handler;

} // namespace

using namespace std;

int main(int argc, char **argv){
    int i = 0, j = 0, arg_size = 0;
    int port_num = 0, temp_port_num = 0;
    
    // Check input arguments
    if (argc != 2)
    {
        // Default port
        port_num = 54000;
    }
    else
    {
        // Calculate port - convert input argument to port
        while(argv[1][i] != '\0')
        {
            ++i;
        }
    
        arg_size = i; 
    
        while(i > 0)
        {
            j = i;
        
            --i;
            temp_port_num = argv[1][i] - '0';
        
            while(j < arg_size)
            {
                temp_port_num *= 10;
                ++j;
            }
        
            port_num += temp_port_num;
        } 
    } 
    
    cout << "Port: " << port_num << endl;

    if(!ser_handler.Init(port_num)){
        cout << "Unable to initialize server handler" << endl;
    }

    while(1){
        this_thread::sleep_for(chrono::seconds(1));
    }

    return 0;
}