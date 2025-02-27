/**
 ***********************************************************************
 * @file   server.cpp 
 * @author Juraj Grabovac (jgrabovac2@gmail.com)
 * @date   26/2/2025
 * @brief  See server.h
 ***********************************************************************
*/


/*----- Includes -----*/
#include "server.h"

namespace {

constexpr auto kMaxClientNum = 5;

/*----- Enums and Structures -----*/
enum msg_type
{
    COMMAND,
    TOPIC,
    DATA
};

enum msg_command
{
    PUBLISH,
    SUBSCRIBE,
    UNSUBSCRIBE,
    INVALID_COMMAND
};

struct str_client
{
    SOCKET sock_handler;
    string subscribe_topic;
};

struct input_message
{
    string commandInput;
    string topicInput;
    string dataInput;
};

struct str_client _client_subscribe_list[kMaxClientNum];
struct input_message _received_input_message;
 
/*----- Helper Functions -----*/
/**
 * @brief Initialization of the client subscribe list to default values.
 */
void initialize_client_subscribe_list(void)
{
    int i;
    
    for(i = 0; i < kMaxClientNum; ++i)
    {
        _client_subscribe_list[i].sock_handler = INVALID_SOCKET;
        _client_subscribe_list[i].subscribe_topic = "NO_TOPIC";
    }
    
    cout << "Init Done" << endl;
}

/**
 * @brief Function add clients to subscribe list.
 * 
 * @param [in] client - socket
 */
void add_client_to_subscribe_list(SOCKET client)
{
    int i;
    
    for(i = 0; i < kMaxClientNum; ++i)
    {
        if(_client_subscribe_list[i].sock_handler == INVALID_SOCKET)
        {
            _client_subscribe_list[i].sock_handler = client;
            cout << "Client added to subscribe list" << endl;
            break;
        }
    }
    
    if (i == kMaxClientNum)
    {
        cout << "Client subscribe list full" << endl;
    }
}

/**
 * @brief Function remove clients from subscribe list.
 * 
 * @param [in] client - socket
 */
void remove_client_from_subscribe_list(SOCKET client)
{
    int i;
    
    for(i = 0; i < kMaxClientNum; ++i)
    {
        if (_client_subscribe_list[i].sock_handler == client)
        {
            _client_subscribe_list[i].sock_handler = INVALID_SOCKET;
            _client_subscribe_list[i].subscribe_topic = "NO_TOPIC";
            cout << "Client removed from subscribe list" << endl;
            
            break;
        }
    }
}

/**
 * @brief Function for parsing input message.
 * 
 * @param [in] buf - string buffer
 */
void parse_input_message(string buf)
{
    size_t pos = 0;
    int delimeterCount = 0;
    string delimiter = " ";
    string commandPart;
    
    cout << "Parse input message" << endl;
    
    while (((pos = buf.find(delimiter)) != string::npos) || (buf.empty() == 0)) 
    {
        if(pos != string::npos)
        {
            commandPart = buf.substr(0, pos);
        }
        else
        {
            commandPart = buf;
        }
        
        switch(delimeterCount)
        {
            case COMMAND:
            {
                _received_input_message.commandInput = commandPart;
                
                break;
            }
            case TOPIC:
            {
                _received_input_message.topicInput = commandPart;
                
                break;
            }
            case DATA:
            {
                _received_input_message.dataInput = commandPart;
                
                break;
            }
            default:
            {
                cout << "Invalid part of message" << endl;
            }
        }
        
        cout << "Client sent: " << commandPart << endl;
        
        if(pos != string::npos)
        {
            buf.erase(0, pos + delimiter.length());
        }
        else
        {
            buf.clear();
        }

        delimeterCount++;
    }
    
}

/**
 * @brief Function for subscribing client to specific topic.
 * 
 * @param [in] sock - socket
 * @param [in] topic - string topic
 */
void subscribe(SOCKET sock, string topic)
{
    int i;
    
    for(i = 0; i < kMaxClientNum; ++i)
    {
        if(_client_subscribe_list[i].sock_handler == sock)
        {
            _client_subscribe_list[i].subscribe_topic = topic;
            cout << "Topic Subscribed" << endl;
            
            break;
        }
    }
}

/**
 * @brief Function for unsubscribing client from specific topic.
 * 
 * @param [in] sock - socket
 * @param [in] topic - string topic
 */
void unsubscribe(SOCKET sock, string topic)
{
    int i;
    
    for(i = 0; i < kMaxClientNum; ++i)
    {
        if((_client_subscribe_list[i].sock_handler == sock) && (_client_subscribe_list[i].subscribe_topic == topic))
        {
            _client_subscribe_list[i].subscribe_topic = "NO_TOPIC";
            cout << "Topic Unsubscribed" << endl;
            
            break;
        }
    }
}

/**
 * @brief Function for checking is the client subscribed to specific topic.
 * 
 * @param [in] sock - socket
 * @param [in] topic - string topic
 *
 * @return int - 1 if client subscribed, otherwise 0.
 */
int is_client_subscribed_to_publish_topic(SOCKET sock, string topic)
{
    
    int retval = 0;
    int i;
    
    for(i = 0; i < kMaxClientNum; ++i)
    {
        if((_client_subscribe_list[i].sock_handler == sock) && (_client_subscribe_list[i].subscribe_topic.compare(topic) == 0))
        {
            retval = 1;
            cout << "Subscribe topic found" << endl;
        }
    }
    
    return retval;
}

/**
 * @brief Funtion converts input string to enum.
 * 
 * @param [in] input - string input
 *
 * @return msg_command - enum.
 */
msg_command resolveCommand(string input)
{
    if(input.compare("PUBLISH") == 0) return PUBLISH;
    if(input.compare("SUBSCRIBE") == 0) return SUBSCRIBE;
    if(input.compare("UNSUBSCRIBE") == 0) return UNSUBSCRIBE;
    
    return INVALID_COMMAND;
}

}  // namespace

using namespace std;

namespace server_handler {

ServerHandler::~ServerHandler() {
    _server_thread.join();

    // Cleanup winsock
	WSACleanup();
}

bool ServerHandler::Init(int port_number){
    _port_num = port_number;

    if(!InitializeWinSock()){
        return false;
    }

    if(!CreateListeningSocket()){
        return false;
    }

    _server_thread = std::thread(ServerThread, this);

    return true;
}

bool ServerHandler::InitializeWinSock(){
    bool ret = true;

	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);

	int wsOk = WSAStartup(ver, &wsData);
	if (wsOk != 0)
	{
		cerr << "Can't Initialize winsock!" << endl;
		ret = false;
	}

    return ret;
}

bool ServerHandler::CreateListeningSocket(){
    bool ret = true;

	// Create a listening socket
	_listening = socket(AF_INET, SOCK_STREAM, 0);
	if (_listening == INVALID_SOCKET)
	{
		cerr << "Can't create a socket" << endl;
		return 0;
	}

    return ret;
}

void ServerHandler::ServerThread(){
    // Bind the ip address and port to a socket
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(_port_num);
	hint.sin_addr.S_un.S_addr = INADDR_ANY; 
	
	bind(_listening, (sockaddr*)&hint, sizeof(hint));

	// Set socket for listening 
	listen(_listening, SOMAXCONN);

	// Create file descriptor and zero it
	fd_set master;
	FD_ZERO(&master);

	// Add listening socket to file descriptor
	FD_SET(_listening, &master);
    
    // Initialize client subscribe list
    initialize_client_subscribe_list();

	while (1)
	{
		// Make a copy of descriptor file because select() call is destructive
		fd_set copy = master;
        
        // Call select() 
		int socketCount = select(0, &copy, nullptr, nullptr, nullptr);

		// Loop through all the current connections
		for (int i = 0; i < socketCount; i++)
		{
			SOCKET sock = copy.fd_array[i];

			// Check is it a listening or a client socket
			if (sock == _listening)
			{
				// Accept new connection
				SOCKET client = accept(_listening, nullptr, nullptr);

				// Add the new connection to the list of connected clients
				FD_SET(client, &master);
                
                // Add the new client to the subscribe list
                add_client_to_subscribe_list(client);

				// Send a message to the connected client
				string ConnectMsg = "CLIENT CONNECTED\n";
				send(client, ConnectMsg.c_str(), ConnectMsg.size() + 1, 0);
			}
			else
			{
				char bufInput[4096];
				ZeroMemory(bufInput, 4096);
				
				// Receive message
				int bytesIn = recv(sock, bufInput, 4096, 0);
                
				if (bytesIn <= 0)
				{
					// Close client
					closesocket(sock);
					FD_CLR(sock, &master);
				}
                else if(strcmp(bufInput, "DISCONNECT") == 0)
                {
                    // Send a message to the disconnected client
                    string DisconnectMsg = "CLIENT DISCONNECTED\n";
                    send(sock, DisconnectMsg.c_str(), DisconnectMsg.size() + 1, 0);
                    
                    // Remove client from subscribe list
                    remove_client_from_subscribe_list(sock);
            
                    // Disconnect the client
                    closesocket(sock);
                    FD_CLR(sock, &master);
                }
				else
				{
                    string buf = bufInput;
                    string publish_topic;
                    int publish_flag = 0;
                    
                    // Parse input message before checking commands
                    parse_input_message(buf);
                    
                    // Convert string to enum so that switch-case could be performed 
                    switch(resolveCommand(_received_input_message.commandInput))
                    {
                        case PUBLISH:
                        {
                            cout << "Publish command received" << endl;
                            
                            // Set publish flag and add publish topic 
                            publish_flag = 1;
                            publish_topic = _received_input_message.topicInput;
                            
                            break;
                        }
                        case SUBSCRIBE:
                        {
                            cout << "Subscribe command received" << endl;
                            
                            // Subscribe client to specific topic 
                            subscribe(sock, _received_input_message.topicInput);
                            
                            break;
                        }
                        case UNSUBSCRIBE:
                        {
                            cout << "Unsubscribe command received" << endl;
                            
                            // Unsubscribe client from specific topic
                            unsubscribe(sock, _received_input_message.topicInput);
                            
                            break;
                        }
                        default:
                        {
                            cout << "Unknown command" << endl;
                        }
                    }

					// Check if there are messages waiting to be published to a specific topic and socket
					for (int i = 0; i < (int)master.fd_count; i++)
					{
						SOCKET outSock = master.fd_array[i];

                        // Check publish flag status and whether any client subscribed to publish topic
                        if (is_client_subscribed_to_publish_topic(outSock, publish_topic) && publish_flag)
						{
							ostringstream ss;
                            ss << "[Message] Topic: " << publish_topic << " Data: " << _received_input_message.dataInput << endl;
                            
							string strOut = ss.str();
							send(outSock, strOut.c_str(), strOut.size() + 1, 0);
                            
                            // Clear publish flag and topic
                            publish_flag = 0;
                            publish_topic.clear();
						}
					}
				}
			}
		}
	}

	while (master.fd_count > 0)
	{
		// Get the socket
		SOCKET sock = master.fd_array[0];

		// Remove socket from the master file list and close the socket
		FD_CLR(sock, &master);
		closesocket(sock);
	}
}

} // namespace server_handler