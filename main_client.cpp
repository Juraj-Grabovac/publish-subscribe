/**
 * @file main_client.cpp
 *
 * @brief Client application entry point.
 * 
 */

#include "client.h"

namespace {

client_handler::ClientHandler cl_handler;

} // namespace

using namespace std;

int main(){
    if(!cl_handler.Init()){
        cout << "Unable to initialize client handler" << endl;
    }

    while(1){
        this_thread::sleep_for(chrono::seconds(1));
    }

    return 0;
}