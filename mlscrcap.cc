#include "mlscrcap.h"
#include <chrono>
#include <stdexcept>
#include <iostream>
#include <cstdint>
#include <tuple>
#include <opencv2/opencv.hpp> // must include opencv before X11 lib
#include <X11/Xlib.h>
#include <sys/stat.h>
#include "mldisplay.h"
#include "mltimer.h"

	
MlScreenCapturer::MlScreenCapturer(MlDisplay& display)
	: display(display), positions{std::make_tuple(0, 0), std::make_tuple(0, 0)}
{
	// checking if directory exists
	// if not, create one.
	struct stat stat_buff;
	stat(save_dir.c_str(), &stat_buff);
	if (!S_ISDIR(stat_buff.st_mode)) {
		mkdir(save_dir.c_str(), 0777);
	}
	auto screen = ScreenOfDisplay(display.display_ptr.get(), 0);
	positions[1] = std::make_tuple(screen->width, screen->height);
}



ScreenshotArea MlScreenCapturer::get_screen_coordinate() const
{
	return {positions[0], positions[1]};
}

void MlScreenCapturer::screenshot()
{
	if (!size_captured) {
		throw std::logic_error("never call MlScreenCapturer::capture_screen_size() before calling MlScreenCapturer::screenshot()!");
	}	
	int scr_width = std::get<Coord::X>(positions[1]) - std::get<Coord::X>(positions[0]);
	int scr_height = std::get<Coord::Y>(positions[1]) - std::get<Coord::Y>(positions[0]);

	// Get screenshot
	auto img = XGetImage(display.display_ptr.get(),
				display.window,
				std::get<Coord::X>(positions[0]),
				std::get<Coord::Y>(positions[0]),
				scr_width,
				scr_height,
				AllPlanes,
				ZPixmap);

	auto pixels = cv::Mat(scr_height,
			      scr_width,
			      CV_8UC4,
			      img->data);
	
	try {
		cv::imwrite(save_dir + '/' + std::to_string(MlTimer<std::chrono::milliseconds>::get_timestamp()) + ".jpg", pixels);
	} catch (std::runtime_error& ex) {
		std::cerr << ex.what() << std::endl;
	}
}

const std::string MlScreenCapturer::save_dir = "data";
