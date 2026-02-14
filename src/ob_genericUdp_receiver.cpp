/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software
    MODULE: OB_GENERICUDP_RECEIVER.CPP

    A simple UDP receiver that interprets incoming packets as IEEE-754
    single-precision floats (4 bytes each). The number of floats in the
    packet defines the number of channels (len/4). If more than 16 channels
    are received the packet is discarded. 
-----------------------------------------------------------------------------*/

#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#include <iphlpapi.h>
#pragma comment(lib, "iphlpapi.lib")  // link with iphlpapi for adapter enumeration
#include "brainBay.h"
#include "ob_genericUdp_receiver.h"

#define DISCOVERY_PORT 5555

#define MAX_CHANNELS 16

// Shared buffer for received values (one float per channel)
float udp_values[MAX_CHANNELS] = { 0 };

// Thread control
int udpThreadDone = 1;
int udpThreadRunning = 0;
DWORD udpStatId = 0;
uint32_t udpPacketCount = 0;

LRESULT CALLBACK GenericUdpDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

DWORD WINAPI UdpProc(LPVOID lpv)
{
    udpThreadRunning = 1;
    udpThreadDone = 0;
    GENERIC_UDP_RECEIVEROBJ* st = (GENERIC_UDP_RECEIVEROBJ*)lpv;
    if (!st) return 0;
    while (!udpThreadDone) {
        if (st->sock && st->packet) {
            if (SDLNet_UDP_Recv(st->sock, st->packet)) {
                int len = st->packet->len;
                int total_floats = len / sizeof(float);
                if (total_floats <= 0) continue;

                int channels = st->num_channels;
                if (channels <= 0) channels = 1;
                if (channels > MAX_CHANNELS) channels = MAX_CHANNELS;

                int samples = total_floats / channels;
                if (samples <= 0) continue;

                if (st->num_samples > 0 && samples > st->num_samples) samples = st->num_samples;

                for (int s = 0; s < samples && !udpThreadDone; s++) {
                    for (int c = 0; c < channels; c++) {
                        float v = 0.0f;
                        memcpy(&v, st->packet->data + (s * channels + c) * sizeof(float), sizeof(float));
                        if (c < MAX_CHANNELS) udp_values[c] = v;
                    }
                    process_packets();
                    udpPacketCount++;
                }
            }
        }
        Sleep(1);
    }
    udpThreadRunning = 0;
    return 0;
}

GENERIC_UDP_RECEIVEROBJ::GENERIC_UDP_RECEIVEROBJ(int num) : BASE_CL()
{
    int i;
    inports = 0;
    outports = 16; // default 16 output ports
    width = 100; height = CON_START + outports * CON_HEIGHT + 5;
    sock = NULL;
    packet = NULL;
    port = DEFAULT_UDP_PORT;
    opened = 0;
    num_channels = 1;  // default to 1 channel 
    num_samples = 10;  // 10 samples per channel
    strcpy(host, "");

    // determine local IP address and store in host
    // prefer wireless adapter (IF_TYPE_IEEE80211) then any private IPv4
    ULONG flags = GAA_FLAG_INCLUDE_PREFIX;
    ULONG outBufLen = 15000;
    PIP_ADAPTER_ADDRESSES pAddresses = (PIP_ADAPTER_ADDRESSES)malloc(outBufLen);
    if (pAddresses) {
        DWORD dwRet = GetAdaptersAddresses(AF_INET, flags, NULL, pAddresses, &outBufLen);
        if (dwRet == ERROR_BUFFER_OVERFLOW) {
            free(pAddresses);
            pAddresses = (PIP_ADAPTER_ADDRESSES)malloc(outBufLen);
            if (pAddresses) dwRet = GetAdaptersAddresses(AF_INET, flags, NULL, pAddresses, &outBufLen);
        }
        if (dwRet == NO_ERROR) {
            PIP_ADAPTER_ADDRESSES pCurr = pAddresses;
            char best_private[64] = "";
            char first_ipv4[64] = "";
            while (pCurr) {
                if (pCurr->OperStatus == IfOperStatusUp) {
                    PIP_ADAPTER_UNICAST_ADDRESS pUnicast = pCurr->FirstUnicastAddress;
                    while (pUnicast) {
                        SOCKADDR *sa = pUnicast->Address.lpSockaddr;
                        if (sa && sa->sa_family == AF_INET) {
                            struct sockaddr_in *sin = (struct sockaddr_in*)sa;
                            const char *ipstr = inet_ntoa(sin->sin_addr);
                            if (!ipstr) { pUnicast = pUnicast->Next; continue; }

                            // skip link-local 169.254.x.x and loopback
                            uint32_t addr = ntohl(sin->sin_addr.s_addr);
                            if ((addr & 0xFFFF0000) == 0xA9FE0000) { pUnicast = pUnicast->Next; continue; }
                            if ((addr >> 24) == 127) { pUnicast = pUnicast->Next; continue; }

                            // prefer wireless adapters
                            if (pCurr->IfType == IF_TYPE_IEEE80211) {
                                strncpy(host, ipstr, sizeof(host)-1);
                                host[sizeof(host)-1]=0;
                                printf("got wireless IP: %s\n", host);
                                goto adapter_done;
                            }

                            // record first private IPv4 (10., 172.16/12, 192.168.)
                            if ( ((addr & 0xFF000000) == 0x0A000000) || ((addr & 0xFFF00000) == 0xAC100000) || ((addr & 0xFFFF0000) == 0xC0A80000) ) {
                                if (best_private[0] == '\0') strncpy(best_private, ipstr, sizeof(best_private)-1);
                            }

                            if (first_ipv4[0] == '\0') strncpy(first_ipv4, ipstr, sizeof(first_ipv4)-1);
                        }
                        pUnicast = pUnicast->Next;
                    }
                }
                pCurr = pCurr->Next;
            }
            if (best_private[0]) {
                strncpy(host, best_private, sizeof(host)-1);
                host[sizeof(host)-1]=0;
                printf("got private IP: %s\n", host);
            }
            else if (first_ipv4[0]) {
                strncpy(host, first_ipv4, sizeof(host)-1);
                host[sizeof(host)-1]=0;
                printf("got first IPv4: %s\n", host);
            }
        } else {
            printf("GetAdaptersAddresses failed: %lu\n", dwRet);
        }
    adapter_done:
        if (pAddresses) free(pAddresses);
    } else {
        printf("Out of memory for adapter enumeration\n");
    }


    for (i = 0; i < outports; i++) {
        sprintf(out_ports[i].out_name, "ch%d", i + 1);
        sprintf(out_ports[i].out_desc, "channel %d", i + 1);
    }
}

GENERIC_UDP_RECEIVEROBJ::~GENERIC_UDP_RECEIVEROBJ()
{
    close_socket();
    printf("UDP-socket closed!\n");
    WSACleanup();
}

int GENERIC_UDP_RECEIVEROBJ::open_socket()
{
    if (sock) return 1;
    sock = SDLNet_UDP_Open(port);
    if (!sock) {
        printf("Could not create UDP socket\n");
        char msg[256]; strcpy(msg, "SDLNet_UDP_Open: "); strcat(msg, SDLNet_GetError());
        report_error(msg);
        return 0;
    }
    packet = SDLNet_AllocPacket(65536);
    if (!packet) {
        report_error((char*)"SDLNet_AllocPacket failed");
        SDLNet_UDP_Close(sock);
        sock = NULL;
        return 0;
    }
    printf("UDP-socket created!\n");
    opened = 1;
    GLOBAL.udpReceiver_available = 1;
    // start receiver thread
    if (!udpThreadRunning) {
        udpThreadDone = 0;
        udpPacketCount = 0;
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)UdpProc, this, 0, &udpStatId);
    }
    return 1;
}

void GENERIC_UDP_RECEIVEROBJ::close_socket()
{
    // signal thread to stop and wait briefly
    udpThreadDone = 1;
    int timeout = 0;
    while (udpThreadRunning && timeout < 50) { Sleep(20); timeout++; }

    if (packet) { SDLNet_FreePacket(packet); packet = NULL; }
    if (sock) { SDLNet_UDP_Close(sock); sock = NULL; }
    opened = 0;
    GLOBAL.udpReceiver_available = 0;
}

void GENERIC_UDP_RECEIVEROBJ::get_captions(void)
{
    int i;
    char tmp[20];
    for (i = 0; i < outports; i++) {
        sprintf(tmp, "udp%d", i + 1);
        strcpy(out_ports[i].out_name, tmp);
        sprintf(out_ports[i].out_desc, "UDP ch %d", i + 1);
        out_ports[i].out_min = -1.0f;
        out_ports[i].out_max = 1.0f;
    }
    printf("udp-receiver created\n");
}

void GENERIC_UDP_RECEIVEROBJ::make_dialog(void)
{
    display_toolbox(hDlg = CreateDialog(hInst, (LPCTSTR)IDD_GENERICUDPBOX, ghWndStatusbox, (DLGPROC)GenericUdpDlgHandler));
}

void GENERIC_UDP_RECEIVEROBJ::load(HANDLE hFile)
{
    load_object_basics(this);
    // load_property("host", P_STRING, &host);
    load_property("port", P_INT, &port);
    load_property("opened", P_INT, &opened);
    load_property("num_channels", P_INT, &num_channels);
    load_property("num_samples", P_INT, &num_samples);


    if (opened) {
        if (!open_socket()) {
            opened = 0;
            report_error((char*)"Could not open UDP socket on load");
        }
    }
}

void GENERIC_UDP_RECEIVEROBJ::save(HANDLE hFile)
{
    save_object_basics(hFile, this);
    save_property(hFile, "host", P_STRING, &host);
    save_property(hFile, "port", P_INT, &port);
    save_property(hFile, "opened", P_INT, &opened);
    save_property(hFile, "num_channels", P_INT, &num_channels);
    save_property(hFile, "num_samples", P_INT, &num_samples);
}

void GENERIC_UDP_RECEIVEROBJ::work(void)
{
    // simply forward latest values populated by receiver thread
    if ((udpPacketCount % 100 == 0) && (hDlg == ghWndToolbox)) {
        char szdata[100];
        sprintf(szdata, "%u UDP Packets read\n", udpPacketCount);
        add_to_listbox(hDlg, IDC_LIST, szdata);
    }
    
    for (int i = 0; i < num_channels && i < outports && i < MAX_CHANNELS; i++) {
        pass_values(i, udp_values[i]);
    }
}

void GENERIC_UDP_RECEIVEROBJ::session_start(void)
{
    // flush UDP packet buffer at session start
    if (sock && packet) {
        while (SDLNet_UDP_Recv(sock, packet)) {
            // discard all pending packets 
        }
    }

    // receiver thread is started on open_socket()

    /*
    // send UDP broadcast message to announce this receiver
    if (sock) {
        // Use a temporary Winsock UDP socket for broadcasting so we can enable SO_BROADCAST.
        SOCKET bsock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (bsock == INVALID_SOCKET) {
            printf("Failed to create broadcast socket\n");
        }
        else {
            char opt = 1;
            if (setsockopt(bsock, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt)) == SOCKET_ERROR) {
                printf("Failed to set SO_BROADCAST on socket\n");
            }
            else {
                struct sockaddr_in baddr;
                memset(&baddr, 0, sizeof(baddr));
                baddr.sin_family = AF_INET;
                baddr.sin_port = htons(DISCOVERY_PORT);
                baddr.sin_addr.s_addr = inet_addr("255.255.255.255");

                char msg[256];
                snprintf(msg, sizeof(msg), "BRAINBAY_UDP_RECEIVER:%s:%d", host, port);

                int sent = sendto(bsock, msg, (int)strlen(msg), 0, (struct sockaddr*)&baddr, sizeof(baddr));
                if (sent == SOCKET_ERROR) {
                    printf("Failed to send UDP broadcast announcement (winsock): %d\n", WSAGetLastError());
                }
                else {
                    printf("UDP broadcast announcement sent from %s:%d\n", host, port);
                }
            }
            closesocket(bsock);
        }
    } */
}


// Dialog Handler
LRESULT CALLBACK GenericUdpDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    GENERIC_UDP_RECEIVEROBJ* st;
    st = (GENERIC_UDP_RECEIVEROBJ*)actobject;
    if ((st == NULL) || (st->type != OB_GENERIC_UDP_RECEIVER)) {
        return(0);
    }

    switch (message)
    {
    case WM_INITDIALOG:
        if (st) {
            SetDlgItemText(hDlg, IDC_HOST, st->host);
            SetDlgItemInt(hDlg, IDC_PORT, st->port, FALSE);
            SetDlgItemInt(hDlg, IDC_NUM_CHANNELS, st->num_channels, FALSE);
            SetDlgItemInt(hDlg, IDC_NUM_SAMPLES, st->num_samples, FALSE);

            // reflect opened state in checkbox (IDC_OPENED must exist in resource)
            SendDlgItemMessage(hDlg, IDC_OPENED, BM_SETCHECK, st->opened ? BST_CHECKED : BST_UNCHECKED, 0);
            if (GLOBAL.udpReceiver_available) {
                add_to_listbox(hDlg, IDC_LIST, "UDP device connected.");
            }
            else {
                add_to_listbox(hDlg, IDC_LIST, "UDP device disconnected");
            }
        }
        return TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_CONNECT:
            if (st) {
                //GetDlgItemText(hDlg, IDC_HOST, st->host, sizeof(st->host));
                st->port = GetDlgItemInt(hDlg, IDC_PORT, NULL, FALSE);
                st->num_channels = GetDlgItemInt(hDlg, IDC_NUM_CHANNELS, NULL, FALSE);
                st->num_samples = GetDlgItemInt(hDlg, IDC_NUM_SAMPLES, NULL, FALSE);
                if (!st->open_socket()) {
                    MessageBox(hDlg, "Could not open UDP socket", "UDP Receiver", MB_OK | MB_ICONERROR);
                }
                else {
                    SendDlgItemMessage(hDlg, IDC_OPENED, BM_SETCHECK, BST_CHECKED, 0);
                    st->opened = 1;
                    add_to_listbox(hDlg, IDC_LIST, "UDP socket created.");
                }
            }
            return TRUE;
        case IDC_CLOSE:
            if (st) { st->close_socket(); SendDlgItemMessage(hDlg, IDC_OPENED, BM_SETCHECK, BST_UNCHECKED, 0); st->opened = 0; }
            add_to_listbox(hDlg, IDC_LIST, "UDP device disconnected");
            return TRUE;
        case IDC_OPENED:
            if (st) {
                LRESULT chk = SendDlgItemMessage(hDlg, IDC_OPENED, BM_GETCHECK, 0, 0);
                if (chk == BST_CHECKED) {
                    if (!st->open_socket()) {
                        MessageBox(hDlg, "Could not open UDP socket", "UDP Receiver", MB_OK | MB_ICONERROR);
                        SendDlgItemMessage(hDlg, IDC_OPENED, BM_SETCHECK, BST_UNCHECKED, 0);
                    }
                    else st->opened = 1;
                    add_to_listbox(hDlg, IDC_LIST, "UDP socket created.");
                }
                else {
                    st->close_socket();
                    st->opened = 0;
                    add_to_listbox(hDlg, IDC_LIST, "UDP device disconnected");
                }
            }
            return TRUE;
        }
        return TRUE;
    case WM_CLOSE:
        EndDialog(hDlg, LOWORD(wParam));
        return TRUE;
    }
    return FALSE;
}
