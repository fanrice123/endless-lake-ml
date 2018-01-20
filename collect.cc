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
#include "thread_pool.h"
using namespace std;
//using namespace std::literals::chrono_literals;

void write_data(ostream_iterator<unsigned>&, 
                ostream_iterator<bool>&, 
                const vector<region_type>&,
                bool);

void signal_handle(int);

volatile sig_atomic_t quit = 0;

int main()
{
    ofstream data_file("data.csv");
    ostream_iterator<unsigned> d_writer(data_file, ", ");
    ostream_iterator<bool> l_writer(data_file, "\n");
    thread_pool<3> pool;
    struct sigaction sa;
    sa.sa_handler = signal_handle;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction installation failed.");
        return EXIT_FAILURE;
    }


	XInitThreads();
	MlDisplay display;
	MlTimer<std::chrono::milliseconds> timer(30ms);

	MlInputListener input(display);
	MlScreenCapturer screen(display);
    MlImageProcessor img_proc("settings.json");
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

void write_data(ostream_iterator<unsigned>& data_writer, 
                ostream_iterator<bool>& label_writer, 
                const vector<region_type>& data,
                bool label)
{
    for (const auto& region : data) {
        *data_writer = static_cast<unsigned>(region);
    }
    *label_writer = label;
}
