/**
 ***********************************************************************
 * @file   client.cpp 
 * @author Juraj Grabovac (jgrabovac2@gmail.com)
 * @date   26/2/2025
 * @brief  See client.h
 ***********************************************************************
*/


/*----- Includes -----*/
#include "client.h"

namespace {

constexpr auto kENDL = 13;
constexpr auto kBS = 8;

}  // namespace

using namespace std;

namespace client_handler {

ClientHandler::~ClientHandler() {
    _client_thread.join();

    // Close socket
	closesocket(_sock);
    
    // Cleanup winsock
	WSACleanup();
}

bool ClientHandler::Init(){
    if(!InitializeWinSock()){
        return false;
    }

    if(!CreateSocket()){
        return false;
    }

    ConnectToServer();

    _client_thread = std::thread(ClientThread, this);

    return true;
}

bool ClientHandler::InitializeWinSock(){
    bool ret = true;

	WSAData data;
	WORD ver = MAKEWORD(2, 2);
	int wsResult = WSAStartup(ver, &data);
	if (wsResult != 0)
	{
		cerr << "Can't start Winsock, Err #" << wsResult << endl;
		ret = false;
	}

    return ret;
}

bool ClientHandler::CreateSocket(){
    bool ret = true;

	_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (_sock == INVALID_SOCKET)
	{
		cerr << "Can't create socket, Err #" << WSAGetLastError() << endl;
		WSACleanup();
		ret = false;
	}

    return ret;
}
void ClientHandler::ConnectToServer(){
    // Fill hint structure
	sockaddr_in hint;
	hint.sin_family = AF_INET;
    hint.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    string command, client_name, userInput;
    int connResult = SOCKET_ERROR;
    int recv_port;

    // Connection to server
    do
    {
        cout << ">>> ";
        
        getline(cin, userInput);
        istringstream iss(userInput);

        // Parse input stream to command, port and client name
        iss >> command >> recv_port >> client_name;
        
        // Check command
        if(command.compare("CONNECT") != 0)
        {
            cout << "Invalid command" << endl;
        }
        else if(client_name.empty())
        {
            cout << "Client name empty" << endl;
        }
        else
        {
            // Set port
            hint.sin_port = htons(recv_port);
            
            // Connect to server
            connResult = connect(_sock, (sockaddr*)&hint, sizeof(hint));
            if (connResult == SOCKET_ERROR)
            {
                cerr << "Can't connect to server, try another port" << endl;
            }
        }
    } while(connResult != 0);
}

void ClientHandler::ClientThread(){
    // Set non-blocking(mode 1) client socket
    // Enabled send/recv without blocking while waiting for input 
    int mode = 1;
    ioctlsocket(_sock, FIONBIO, (unsigned long *)&mode);
    
    // Create file descriptors for read and write
    fd_set read_flags, write_flags;
    
    // Wait time for an event
    struct timeval waitd = {10, 0};
  
    long unsigned int br;
    int sel, numRead;
    int endl_flag = 0;   

    char in[255];
    memset(&in, 0, 255);

    // Definition of std input handler and record
    HANDLE hIn;
    INPUT_RECORD input;
    
    string userInput;

    while(1) 
    {
        FD_ZERO(&read_flags);
        FD_ZERO(&write_flags);
        FD_SET(_sock, &read_flags);
        FD_SET(_sock, &write_flags);

        sel = select(_sock+1, &read_flags, &write_flags, nullptr, &waitd);
        
        // Check select error
        if(sel < 0)
        {
            cerr << "Last Select Error, Err #" << WSAGetLastError() << endl;
            continue;
        }
        
        // If socket ready for reading
        if(FD_ISSET(_sock, &read_flags)) 
        {
            // Clear read set
            FD_CLR(_sock, &read_flags);

            // Read message from server
            memset(&in, 0, 255);
            numRead = recv(_sock, in, 255, 0);
            
            if(numRead <= 0) 
            {
                printf("\nClosing socket");
                closesocket(_sock);
                break;
            }
            else if(in[0] != '\0')
            {
                cout << in;
            }
        }

        /* ----- Receiving User Input ----- */
        // Get std input handler and check is there any input
        hIn = GetStdHandle(STD_INPUT_HANDLE);
        GetNumberOfConsoleInputEvents(hIn, &br);
        
        if(br)
        {
            // If there is console input, read it
            ReadConsoleInput(hIn, &input, 1, &br);
            
            // Check that input is a key event
            if(input.EventType == KEY_EVENT && input.Event.KeyEvent.bKeyDown == true)
            {
                // Discard input buffer and reset br - input already read and saved to string input
                FlushConsoleInputBuffer(hIn);
                br = 0;

                // Print input to console to keep read input visible
                cout << (char)input.Event.KeyEvent.wVirtualKeyCode;


                // Create user input and save to string userInput
                if ((input.Event.KeyEvent.wVirtualKeyCode != kENDL) && (input.Event.KeyEvent.wVirtualKeyCode != kBS))
                {
                    userInput += (char)input.Event.KeyEvent.wVirtualKeyCode;
                }
                else if (input.Event.KeyEvent.wVirtualKeyCode == kBS)
                {
                    if (userInput.empty() == 0)
                    {
                        userInput.pop_back();
                    }
                }
                else
                {
                    cout << endl;
                    endl_flag = 1;
                }
            }
        }
 
        // If socket ready for writing
        if(FD_ISSET(_sock, &write_flags) && endl_flag) 
        {
            // Clear write set
            FD_CLR(_sock, &write_flags);
            
            // Send message to server
            send(_sock, userInput.c_str(), 255, 0);
            userInput.clear();
            
            // Reset end line flag to enable receiving new user input
            endl_flag = 0;
        }
    }
}

} // namespace client_handler