#include <iostream>
#include <fstream>
#include <iterator>
#include <chrono>
#include <vector>
#include <thread>
#include <csignal>
#include <unistd.h>
#include "mlscrcap.h"
#include "mltimer.h"
#include "mlinput.h"
#include "mlimage.h"
using namespace std;
//using namespace std::literals::chrono_literals;

template <typename Data_F_Itr,
          typename Data_Label_F_Itr,
          typename Data_Con,
          typename Label>
void write_data(Data_F_Itr&, 
                Data_Label_F_Itr&, 
                Data_Con&&,
                Label&&);

void signal_handle(int);

volatile sig_atomic_t quit = 0;

int main()
{
    ofstream data_file("data.csv", ios::app | ios::out);
    ofstream data_label_file("data_label.csv", ios::app | ios::out);
    data_label_file << fixed;
    ostream_iterator<string> d_writer(data_file, "\n");
    ostream_iterator<bool> l_writer(data_label_file, "\n");
    struct sigaction sa;
    sa.sa_handler = signal_handle;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction installation failed.");
        return EXIT_FAILURE;
    }

    cout << "pass" << endl;

	XInitThreads();
	MlDisplay display;
	MlTimer<std::chrono::milliseconds> timer(30ms);

	MlInputListener input(display);
    cout << "pass" << endl;
	MlScreenCapturer screen(display);
    cout << "pass" << endl;
    MlImageProcessor img_proc("settings.json");
    cout << "pass" << endl;
    //screen.size_captured = true;
	

	auto get_click = [&]
	{
		return input.get_click(input.LEFT_CLICK);
	};

	auto get_pos = [&]
	{
		return input.get_prev_position();
	};

	auto t = [] { return std::chrono::steady_clock::now(); };
	screen.capture_screen_size(get_click, get_pos);	
    img_proc.set_roi(screen.screenshot());

	cout << 'x' << endl;
	input.get_press('b');


	cout << "fine" << endl;
	while (!quit) { 
		timer.start();
		auto pic = screen.screenshot();
        auto result_future = img_proc.extract_feature_async(pic);
       	try {
            bool click = input.global_wait_click(input.LEFT_CLICK, timer.remaining());
            write_data(d_writer, l_writer, result_future.get(), click);

	    } catch (std::runtime_error& ex) {
		    std::cerr << ex.what() << std::endl;
	    } 
        cerr << "noted" << endl;
	}

	return 0;
}

void signal_handle(int sig)
{
    if (sig == SIGINT)
        quit = 1;
}

template <typename Data_F_Itr,
          typename Data_Label_F_Itr,
          typename Data_Con,
          typename Label>
void write_data(Data_F_Itr& data_writer, 
                Data_Label_F_Itr& label_writer, 
                Data_Con&& data,
                Label&& label)
{
    ostringstream data_line;
    bool not_first_write = false;
    for (const auto& region : data) {
        if (not_first_write) {
            data_line << ',';
        } else {
            not_first_write = true;
        }
        data_line << fixed << region; // display all floating point
    }
    *data_writer = data_line.str();
    *label_writer = label;
}
