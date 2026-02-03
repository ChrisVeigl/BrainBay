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

LRESULT CALLBACK GenericUdpDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

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
    return 1;
}

void GENERIC_UDP_RECEIVEROBJ::close_socket()
{
    if (packet) { SDLNet_FreePacket(packet); packet = NULL; }
    if (sock) { SDLNet_UDP_Close(sock); sock = NULL; }
    opened = 0;
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
}

void GENERIC_UDP_RECEIVEROBJ::work(void)
{
    if (!sock) return; // not opened
    if (!packet) return;

    if (SDLNet_UDP_Recv(sock, packet))
    {
        int len = packet->len;
        int channels = len / 4;

        // printf("udp-packed len %d received\n", len);

        if (channels <= 0) return;
        if (channels > outports) return; // discard if more than available

        // interpret as float32 values
        for (int i = 0; i < channels; i++) {
            float v;
            memcpy(&v, packet->data + i * 4, sizeof(float));
            pass_values(i, v);
        }
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
            // reflect opened state in checkbox (IDC_OPENED must exist in resource)
            SendDlgItemMessage(hDlg, IDC_OPENED, BM_SETCHECK, st->opened ? BST_CHECKED : BST_UNCHECKED, 0);
        }
        return TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_CONNECT:
            if (st) {
                //GetDlgItemText(hDlg, IDC_HOST, st->host, sizeof(st->host));
                st->port = GetDlgItemInt(hDlg, IDC_PORT, NULL, FALSE);
                if (!st->open_socket()) {
                    MessageBox(hDlg, "Could not open UDP socket", "UDP Receiver", MB_OK | MB_ICONERROR);
                }
                else {
                    SendDlgItemMessage(hDlg, IDC_OPENED, BM_SETCHECK, BST_CHECKED, 0);
                    st->opened = 1;
                }
            }
            return TRUE;
        case IDC_CLOSE:
            if (st) { st->close_socket(); SendDlgItemMessage(hDlg, IDC_OPENED, BM_SETCHECK, BST_UNCHECKED, 0); st->opened = 0; }
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
                }
                else {
                    st->close_socket();
                    st->opened = 0;
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
