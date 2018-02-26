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

template <typename Data_Con, typename Label>
void write_data(ofstream&, 
                ofstream&, 
                Data_Con&&,
                Label&&);

void signal_handle(int);

volatile sig_atomic_t quit = 0;

int main()
{
    ofstream data_fs("data.csv", /*ios::app |*/ ios::out);
    ofstream data_label_fs("data_label.csv", /*ios::app |*/ ios::out);
    data_label_fs << fixed;
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

	input.get_press('b');


	while (!quit) { 
		timer.start();
		auto pic = screen.screenshot();
        auto result_future = img_proc.extract_feature_async(pic);
       	try {
            bool click = input.global_wait_click(input.LEFT_CLICK, timer.remaining());
            auto r = result_future.get();
            write_data(data_fs, data_label_fs, r, click);

	    } catch (std::runtime_error& ex) {
		    std::cerr << ex.what() << std::endl;
	    } 
	}

	return 0;
}

void signal_handle(int sig)
{
    if (sig == SIGINT)
        quit = 1;
}

template<typename Data_Con, typename Label>
void write_data(ofstream& data_writer, 
                ofstream& label_writer, 
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
        data_line << region; // display all floating point
    }
    data_writer << data_line.str() << '\n';
    label_writer << label << '\n';
}
