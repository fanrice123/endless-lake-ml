#ifndef MLSCRCAP_H
#define MLSCRCAP_H
#include <opencv2/opencv.hpp>
#include <stdexcept>
#include "position.h"
#include "mldisplay.h"

class MlScreenCapturer;

/**
 * This is a Struct to coordinates of 2 positions that User has clicked,
 * returned by MlScreen::capture_screen_size();
 */
struct ScreenArea
{
	Position positions[2];
};


/**
 * MlScreenCapturer handles all of the Screenshot request
 * including selecting the size of screen to capture.
 */
class MlScreenCapturer {
public:
	/**
	 * Constructor this object requires the display object.
	 * When constructing object, a directory will be created if
	 * the directory doesn't exist.
	 * The directory name is stored in this::save_dir;
	 */
	MlScreenCapturer(MlDisplay&);
	
	/**
	 * Copy Constructor is disabled.
	 */
	MlScreenCapturer(const MlScreenCapturer&) = delete;

	// public member functions
	
	/**
	 * This function is used to select the area of screenshot going to be capture.
	 * parameters:
	 * 	Callable1&& : a callable object to select area on screen.
	 * 		      Note that this callable can be a blank implementation
	 * 		      as long as calling Callable2 can receive 2 position on
	 * 		      screen.
	 * 	Callable2&& : a callable object to get 2 positions on screen.
	 */
	template <class Callable1, class Callable2>
	void capture_screen_size(Callable1&&, Callable2&&);

	/**
	 * This function can return the area of screenshot going to be captured.
	 */
	ScreenArea get_screen_coordinate() const;

	/**
	 * This function is called to operate a screenshot.
	 * Before calling this function, function capture_screen_size(C1&&, C2&&)
	 * has to be called otherwise it will capture a fullscreen screenshot.
	 */
    cv::Mat screenshot();

	// public member variable
	
	/**
	 * this static class member stores the directory of storing screenshot.
	 */
	static const std::string save_dir;
private:
	// private data variables
	MlDisplay display;
	Position positions[2];
	bool size_captured;
};

/// template function implementation

// MlScreenCapturer member function
template <class Callable1, class Callable2>
void MlScreenCapturer::capture_screen_size(Callable1&& func1, Callable2&& func2)
{

	for (int point = 0; point !=2; ++point) {
		while(!func1())
			;
		positions[point] = func2();
	}
	size_captured = true;

}

#endif // MLSCRCAP_H
