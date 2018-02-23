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
	: display(display), size_captured(false)
{
	auto screen = ScreenOfDisplay(display.display_ptr.get(), 0);
	positions[1] = { screen->width, screen->height };
}



ScreenArea MlScreenCapturer::get_screen_coordinate() const
{
	return {positions[0], positions[1]};
}

cv::Mat MlScreenCapturer::screenshot()
{
    /*
	if (!size_captured) {
		throw std::logic_error("never call MlScreenCapturer::capture_screen_size() before calling MlScreenCapturer::screenshot()!");
	}	
    */
	int scr_width = positions[1].x - positions[0].x;
	int scr_height = positions[1].y - positions[0].y;

	// Get screenshot
	auto img = XGetImage(display.display_ptr.get(),
				display.window,
				positions[0].x,
				positions[0].y,
				scr_width,
				scr_height,
				AllPlanes,
				ZPixmap);

	return cv::Mat(scr_height,
			       scr_width,
			       CV_8UC4,
			       img->data);
    /*
	auto pixels = cv::Mat(scr_height,
			      scr_width,
			      CV_8UC4,
			      img->data);
	
	try {
		cv::imwrite(save_dir + '/' + std::to_string(MlTimer<std::chrono::milliseconds>::get_timestamp()) + ".jpg", pixels);
	} catch (std::runtime_error& ex) {
		std::cerr << ex.what() << std::endl;
	}
    */
}

