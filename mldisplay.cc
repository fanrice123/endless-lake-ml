#include "mldisplay.h"
#include <X11/Xlib.h>
#include <sys/stat.h>

// bad_display_connection implementation
bad_display_connection::bad_display_connection(const std::string& s)
	: std::runtime_error(s) { }

// MlDisplay implementation
MlDisplay::MlDisplay()
	: display_ptr(XOpenDisplay(NULL), XCloseDisplay)
{
	if (!display_ptr) {
		throw bad_display_connection("Failed to connect an XDisplay.");
	}

	window = DefaultRootWindow(display_ptr.get());

	XMapRaised(display_ptr.get(), window);

}

