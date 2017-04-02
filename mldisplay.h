#ifndef MLDISPLAY_H
#define MLDISPLAY_H
#include <stdexcept>
#include <memory>
#include <X11/Xlib.h>

class bad_display_connection;
class bad_window;
class MlDisplay;

/**
 * bad_display_connection will be thrown whenever failing to request a Display connection
 */
class bad_display_connection : public std::runtime_error {
public:
	explicit bad_display_connection(const std::string&);
};


/**
 * MlDisplay abstracts X11 interface to hide up all X11 interfaces
 */
class MlDisplay {
friend class MlScreenCapturer;
friend class MlInputListener;

public:
	// constructor
	
	/**
	 * Constructor will open up a connection to local xorg server
	 * if connection establishment fails, then a bad_display_connection
	 * will be thrown
	 */
	MlDisplay();

private:
	// private member variables
	std::shared_ptr<Display> display_ptr;
	Window window;
};


#endif // MLDISPLAY_H
