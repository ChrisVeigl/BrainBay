
#include "brainBay.h"
#include "emotiv\\my_edk.h"
#include "emotiv\\my_EmoStateDLL.h"
#include "emotiv\\my_edkErrorCode.h"

void process_emotiv(void);

class EMOTIVOBJ : public BASE_CL
{

public:
	char archivefile[256];
	HANDLE filehandle;
	int  filemode;
	int state;

	EMOTIVOBJ(int num);
	void make_dialog(void);
	void load(HANDLE hFile);
	void save(HANDLE hFile);
	void updateHeadsetStatus(void);
	void work(void);
	
	void session_reset(void);
	void session_start(void);
	void session_stop (void);
	~EMOTIVOBJ();
};
	