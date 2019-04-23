/* -----------------------------------------------------------------------------

  BrainBay  Version 2.3 (04/2019), contact: chris@shifz.org
  
  MODULE: OB_OSC_SENDER.H:  contains functions for sending OSC messages via UDP

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; See the
  GNU General Public License for more details.

-----------------------------------------------------------------------------*/



#include "brainBay.h"
#include <iostream>
#include "SDL_net.h"
#include <stdint.h>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

#define defaulthost "localhost"
#define OSCPORT 8000
#define LOCALPORT 9000
#define s_writebuflength 8192
#define s_readbuflength 128

#define sockettimeout 50



/*******************************************************************************************
Some things to note about the UDPConnection class.

Init this on both the client and/or server's excutable.  No extra work required. Unless your
planning on tracking the activity and data between multiple clients (eg: for multiplayer).
At which point you just memorize and communicate between different ip's manually.
HINT: look at packet->ip;

*******************************************************************************************/

class UDPConnection
{
    bool quit;
    UDPsocket ourSocket;
    IPaddress serverIP;
public:
    UDPpacket *packet;

    UDPConnection()
    {
        quit = false;
    }
    ~UDPConnection()
    {
        SDLNet_FreePacket(packet);
        SDLNet_Quit();
    }

	bool Init(const std::string &ip, int32_t remotePort, int32_t localPort)
    {
        std::cout << "Connecting to \n\tIP : " << ip << "\n\tPort : " << remotePort << std::endl;
        std::cout << "Local port : " << localPort << "\n\n";

        // Initialize SDL_net
//        if (!InitSDL_Net())
//            return false;

        if (!OpenPort(localPort))
            return false;

        if (!SetIPAndPort(ip, remotePort))
            return false;

        if (!CreatePacket(65536))
            return false;

        /* bind server address to channel 0 */
        if (SDLNet_UDP_Bind(ourSocket, 0, &serverIP) == -1)
        {
            printf("SDLNet_UDP_Bind: %s\n", SDLNet_GetError());
            return false;
        }

        return true;
    }
    bool InitServer(int32_t remotePort, int32_t localPort) {
        std::cout << "connecting to port" << remotePort << std::endl;
        std::cout << "Local port : " << localPort << "\n\n";

        // Initialize SDL_net
        if (!InitSDL_Net())
            return false;

        if (!OpenPort(localPort))
            return false;

        if (!SetPort(remotePort))
            return false;

        if (!CreatePacket(65536))
            return false;

        SDLNet_UDP_Unbind(ourSocket, 0);

        return true;
    }
    bool InitSDL_Net()
    {
        std::cout << "Initializing SDL_net...\n";

        if (SDLNet_Init() == -1)
        {
            std::cout << "\tSDLNet_Init failed : " << SDLNet_GetError() << std::endl;
            return false;
        }

        std::cout << "\tSuccess!\n\n";
        return true;
    }
    bool CreatePacket(int32_t packetSize)
    {
        std::cout << "Creating packet with size " << packetSize << "...\n";

        // Allocate memory for the packet
        packet = SDLNet_AllocPacket(packetSize);

        if (packet == nullptr)
        {
            std::cout << "\tSDLNet_AllocPacket failed : " << SDLNet_GetError() << std::endl;
            return false;
        }

        // Set the destination host and port
        // We got these from calling SetIPAndPort()
        packet->address.host = serverIP.host;
        packet->address.port = serverIP.port;

        std::cout << "\tSuccess!\n\n";
        return true;
    }
    bool OpenPort(int32_t port)
    {
        std::cout << "Opening port " << port << "...\n";

        // Sets our sovket with our local port
        ourSocket = SDLNet_UDP_Open(port);

        if (ourSocket == nullptr)
        {
            std::cout << "\tSDLNet_UDP_Open failed : " << SDLNet_GetError() << std::endl;
            return false;
        }

        std::cout << "\tSuccess!\n\n";
        return true;
    }
    bool SetIPAndPort(const std::string &ip, uint16_t port)
    {
        std::cout << "Setting IP ( " << ip << " ) " << "and port ( " << port << " )\n";

        // Set IP and port number with correct endianess
        if (SDLNet_ResolveHost(&serverIP, ip.c_str(), port) == -1)
        {
            std::cout << "\tSDLNet_ResolveHost failed : " << SDLNet_GetError() << std::endl;
            return false;
        }

        std::cout << "\tSuccess!\n\n";
        return true;
    }
    bool SetPort(uint16_t port)
    {
        std::cout << "Setting up port ( " << port << " )\n";

        // Set IP and port number with correct endianess
        if (SDLNet_ResolveHost(&serverIP, NULL, port) == -1)
        {
            std::cout << "\tSDLNet_ResolveHost failed : " << SDLNet_GetError() << std::endl;
            return false;
        }

        std::cout << "\tSuccess!\n\n";
        return true;
    }
    // Send data. 
    bool Send(const std::string &str)
    {
        // Set the data
        // UDPPacket::data is an Uint8, which is similar to char*
        // This means we can't set it directly.
        //
        // std::stringstreams let us add any data to it using << ( like std::cout ) 
        // We can extract any data from a std::stringstream using >> ( like std::cin )
        //
        //str

        memcpy(packet->data, str.c_str(), str.length());
        packet->len = str.length();

        // Send
        // SDLNet_UDP_Send returns number of packets sent. 0 means error
        //packet->channel = -1;
        if (SDLNet_UDP_Send(ourSocket, -1, packet) == 0)
        {
            std::cout << "\tSDLNet_UDP_Send failed : " << SDLNet_GetError() << "\n"
                << "==========================================================================================================\n";
            //msg.resize(0);
            return false;
        }
        std::cout << "sent to: " << packet->address.host << "\n";
        std::cout << "length is: " << packet->len << "\n";
    }
    inline UDPpacket* recievedData(){
        // Check to see if there is a packet wauting for us...
        if (SDLNet_UDP_Recv(ourSocket, packet))
        {
            /*for (int i = packet->len; i < 512; i++) {
                //may only be needed for local testing.
                packet->data[i] = 0;
            }*/
            //std::cout << "\tData received : " << packet->data << "\n";
            return packet;
        }return NULL;
    }
    inline bool WasQuit()
    {
        return quit;
    }
};


class OSC_SENDEROBJ : public BASE_CL
{
protected:
	DWORD dwRead,dwWritten;
	char szdata[300];
	int ppos;
  
public: 
	UDPConnection *udpConnection;
	UDPpacket *packet;

	char readbuf[s_readbuflength];
	char writebuf[s_writebuflength];
	char   host[101];
	LONGLONG timestamp;

	int  connect();
	void get_captions(void);

    OSC_SENDEROBJ(int num);
	void update_inports(void);
	void incoming_data(int, float);
	void work(void);
	void close_tcp(void);
	void make_dialog(void);
	void load(HANDLE hFile);
	void save(HANDLE hFile);
    ~OSC_SENDEROBJ();

};
