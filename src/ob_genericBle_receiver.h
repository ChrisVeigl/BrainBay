/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software
    MODULE: OB_GENERICBLE_RECEIVER.H:  generic BLE receiver

-----------------------------------------------------------------------------*/

#include "brainBay.h"
#include <iostream>

using namespace std;

class GENERIC_BLE_RECEIVEROBJ : public BASE_CL
{
public:
     GENERIC_BLE_RECEIVEROBJ(int num);
    ~GENERIC_BLE_RECEIVEROBJ();

    void make_dialog(void);
    void load(HANDLE hFile);
    void save(HANDLE hFile);
    void work(void);
    void session_start(void);
    void session_stop(void);
    int connect(void);
    int close(void);
};
