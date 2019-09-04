/* -----------------------------------------------------------------------------

	BrainBay  -  OpenSource Biofeedback Software, contact: chris@shifz.org

  OB_BIOSEMI.H:  contains the interface to the BIOSEMI Active II devices (Mk1 and Mk2)

-----------------------------------------------------------------------------*/

#include "brainBay.h"
#include <vector>
#include <string>
#include <cstdint>
#define BIOSEMI_LINKAGE __cdecl


typedef void* (BIOSEMI_LINKAGE* OPEN_DRIVER_ASYNC_t)(void);
typedef int (BIOSEMI_LINKAGE* USB_WRITE_t)(void*, const unsigned char*);
typedef int (BIOSEMI_LINKAGE* READ_MULTIPLE_SWEEPS_t)(void*, char*, int);
typedef int (BIOSEMI_LINKAGE* READ_POINTER_t)(void*, unsigned*);
typedef int (BIOSEMI_LINKAGE* CLOSE_DRIVER_ASYNC_t)(void*);

// process for receiving chunks, called in timer.cpp
void process_biosemi(void);

class BIOSEMIOBJ : public BASE_CL
{
public:
	// public variables that can be accessed from Dialog Handler
	
	int  test;
	char  archivefile[256];
	char  device[100];
	HANDLE filehandle;
	int  filemode;
	long filelength;

	BIOSEMIOBJ(int num);
	void make_dialog(void);

	typedef std::vector<int32_t> sample_t;
	typedef std::vector<sample_t> chunk_t;

	void session_reset(void);
	void session_start(void);
	void session_stop(void);
	void session_pos(long pos);

	void work(void);
	void get_chunk(chunk_t& result);
	void biosemi_io(void);
	void get_channel_set_index(void);
	void save_ui(void);
	void clear_ui(void);
	void update_output_ui(void);
	void reset_output_ui(void);
	void update_outports(void);
	void release_biosemi(void);
	
	void load(HANDLE hFile);
	void save(HANDLE hFile);
	~BIOSEMIOBJ();
	friend LRESULT CALLBACK BioSemiDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

	int chansetn;
	int chansetn_tmp;
	int labeln;
	int labeln_tmp;
	int memory_montage;
	int memory_montage_tmp;

	std::vector<int> opn;
	std::vector<int> active_outports;

	// vector of channel labels (in BioSemi naming scheme)
	std::vector<std::string> channel_labels_;
	// vector of channel types (in LSL Semi naming scheme)
	std::vector<std::string> channel_types_;
	char chanset_string[50];
	//std::string capdesign_string;
	char refch[50];

	// get channel names
	const std::vector<std::string>& channel_labels() const { return channel_labels_; }
	// get channel types (Sync, Trigger, EEG, EXG,
	const std::vector<std::string>& channel_types() const { return channel_types_; }

private:


protected:
	float value;
};
