#include "brainBay.h"

class SAMPLE_HOLDOBJ : public BASE_CL
{

	public:

	float hold,act, trigger, old_value;
	int mode;

	SAMPLE_HOLDOBJ(int num);

	void make_dialog(void);

	void load(HANDLE hFile);

	void incoming_data(int port, float value);

	void save(HANDLE hFile);
	
	void work(void);

	~SAMPLE_HOLDOBJ();

	friend LRESULT CALLBACK Sample_HoldDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

};
