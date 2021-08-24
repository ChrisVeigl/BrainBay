 
#include <iostream>
#include <stdlib.h>
#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "board_shim.h"

using namespace std;

// bool parse_args(int argc, char* argv[], struct BrainFlowInputParams* params, int* board_id);


int test_brainflow()
{
    BoardShim::enable_dev_board_logger();

    struct BrainFlowInputParams params;

    int board_id = -1;  // synthetic board
    // params.serial_port = std::string("COM8");
    // params.ip_address = std::string(argv[i]);
    // params.ip_port = std::stoi(std::string(argv[i]));
    // params.ip_protocol = std::stoi(std::string(argv[i]));
    // params.timeout = std::stoi(std::string(argv[i]));
    // params.other_info = std::string(argv[i]);
    // params.mac_address = std::string(argv[i]);
    // params.serial_number = std::string(argv[i]);
    // params.file = std::string(argv[i]);

    // if (!parse_args(argc, argv, &params, &board_id))        return -1;
    int res = 0;

    BoardShim* board = new BoardShim(board_id, params);

    cout << "Brainflow: Board created." << std::endl;

    try
    {
        board->prepare_session();
        cout << "Brainflow: Session prepared." << std::endl;

        board->start_stream();
        cout << "Brainflow: Stream started." << std::endl;

#ifdef _WIN32
        Sleep(1000);
#else
        sleep(5);
#endif

        board->stop_stream();
        cout << "Brainflow: Stream stopped." << std::endl;

        // BrainFlowArray<double, 2> data = board->get_current_board_data(10);
        // cout << "Brainflow: 10 samples of data fetched." << std::endl;

        cout << "Brainflow: Data is as follows:" << std::endl;

        // cout << data << std::endl;

//        BrainFlowArray<double, 2> unprocessed_data = board->get_current_board_data(20);
        BrainFlowArray<double, 2> unprocessed_data = board->get_board_data();
        vector <int> eeg_channels = BoardShim::get_eeg_channels(board_id);
        
        vector<int>::iterator ptr;

        // Displaying vector elements using begin() and end()
        cout << "The vector elements are : " << std::endl;
        for (int i = 0; i < unprocessed_data.get_size(1); i++) {
            for (ptr = eeg_channels.begin(); ptr < eeg_channels.end(); ptr++)
                cout << "\tchn " << *ptr << ":" << unprocessed_data.at(*ptr,i);
            cout << std::endl;

        }

        board->release_session();
        cout << "Brainflow: Session released, done!" << std::endl;

    }
    catch (const BrainFlowException& err)
    {
        cout << "Brainflow: Exception handler triggered." << std::endl;

        BoardShim::log_message((int)LogLevels::LEVEL_ERROR, err.what());
        res = err.exit_code;
        if (board->is_prepared())
        {
            board->release_session();
        }
    }

    cout << "Brainflow: now delete board and return." << std::endl;

    delete board;

    return res;
}

/*
bool parse_args(int argc, char* argv[], struct BrainFlowInputParams* params, int* board_id)
{
    bool board_id_found = false;
    for (int i = 1; i < argc; i++)
    {
        if (std::string(argv[i]) == std::string("--board-id"))
        {
            if (i + 1 < argc)
            {
                i++;
                board_id_found = true;
                *board_id = std::stoi(std::string(argv[i]));
            }
            else
            {
                std::cerr << "missed argument" << std::endl;
                return false;
            }
        }
        if (std::string(argv[i]) == std::string("--ip-address"))
        {
            if (i + 1 < argc)
            {
                i++;
                params->ip_address = std::string(argv[i]);
            }
            else
            {
                std::cerr << "missed argument" << std::endl;
                return false;
            }
        }
        if (std::string(argv[i]) == std::string("--ip-port"))
        {
            if (i + 1 < argc)
            {
                i++;
                params->ip_port = std::stoi(std::string(argv[i]));
            }
            else
            {
                std::cerr << "missed argument" << std::endl;
                return false;
            }
        }
        if (std::string(argv[i]) == std::string("--serial-port"))
        {
            if (i + 1 < argc)
            {
                i++;
                params->serial_port = std::string(argv[i]);
            }
            else
            {
                std::cerr << "missed argument" << std::endl;
                return false;
            }
        }
        if (std::string(argv[i]) == std::string("--ip-protocol"))
        {
            if (i + 1 < argc)
            {
                i++;
                params->ip_protocol = std::stoi(std::string(argv[i]));
            }
            else
            {
                std::cerr << "missed argument" << std::endl;
                return false;
            }
        }
        if (std::string(argv[i]) == std::string("--timeout"))
        {
            if (i + 1 < argc)
            {
                i++;
                params->timeout = std::stoi(std::string(argv[i]));
            }
            else
            {
                std::cerr << "missed argument" << std::endl;
                return false;
            }
        }
        if (std::string(argv[i]) == std::string("--other-info"))
        {
            if (i + 1 < argc)
            {
                i++;
                params->other_info = std::string(argv[i]);
            }
            else
            {
                std::cerr << "missed argument" << std::endl;
                return false;
            }
        }
        if (std::string(argv[i]) == std::string("--mac-address"))
        {
            if (i + 1 < argc)
            {
                i++;
                params->mac_address = std::string(argv[i]);
            }
            else
            {
                std::cerr << "missed argument" << std::endl;
                return false;
            }
        }
        if (std::string(argv[i]) == std::string("--serial-number"))
        {
            if (i + 1 < argc)
            {
                i++;
                params->serial_number = std::string(argv[i]);
            }
            else
            {
                std::cerr << "missed argument" << std::endl;
                return false;
            }
        }
        if (std::string(argv[i]) == std::string("--file"))
        {
            if (i + 1 < argc)
            {
                i++;
                params->file = std::string(argv[i]);
            }
            else
            {
                std::cerr << "missed argument" << std::endl;
                return false;
            }
        }
    }
    if (!board_id_found)
    {
        std::cerr << "board id is not provided" << std::endl;
        return false;
    }
    return true;
}

*/