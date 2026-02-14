/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software
    MODULE: OB_GENERICUDP_RECEIVER.H:  generic UDP receiver

-----------------------------------------------------------------------------*/

#include "brainBay.h"
#include <iostream>
#include "SDL_net.h"

using namespace std;

#define DEFAULT_UDP_PORT 5005

class GENERIC_UDP_RECEIVEROBJ : public BASE_CL
{
public:
    UDPsocket sock;
    UDPpacket * packet;
    IPaddress remote_ip;
    char host[101];
    int port;
    int opened;
    int num_channels;
    int num_samples;
    // Thread control - per-instance to avoid cross-instance races
    volatile int udpThreadDone;
    volatile int udpThreadRunning;
    HANDLE udpThreadHandle;
    uint32_t updSampleCount;

    GENERIC_UDP_RECEIVEROBJ(int num);
    ~GENERIC_UDP_RECEIVEROBJ();

    void make_dialog(void);
    void load(HANDLE hFile);
    void save(HANDLE hFile);
    void work(void);
    int open_socket();
    void close_socket();
    void get_captions(void);
    void session_start(void);
};
