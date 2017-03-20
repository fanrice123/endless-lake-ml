#include "mlinput.h"
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <chrono>
#include <iostream>
#include <tuple>
#include <cstring>
#include "mldisplay.h"

// MlInputListener Implementation
MlInputListener::MlInputListener(MlDisplay& display, const std::chrono::milliseconds& ms)
	: display(display), timer(ms) {}

MlInputListener::~MlInputListener() { }

void MlInputListener::set_wait_for(const std::chrono::milliseconds& ms)
{
	timer = MlTimer<std::chrono::milliseconds>(ms);
}

// MlMouseListener Implementation
MlMouseListener::MlMouseListener(MlDisplay& display, const std::chrono::milliseconds& ms)
	: MlInputListener(display, ms) {}

bool MlMouseListener::get_press(const int mouse_click)
{
	bool ret_val = false;
	XEvent xevent;

	grab_pointer();
	while (true) {
		XNextEvent(display.display_ptr.get(), &xevent);
		if (xevent.type == ButtonPress) {
			if (xevent.xbutton.button == mouse_click) {
				prev_pos = std::make_tuple(xevent.xmotion.x_root, xevent.xmotion.y_root);
				std::cout << "Mouse clicked " << button[mouse_click] << ": <" << std::get<Coord::X>(prev_pos)
				   << ", "
				   << std::get<Coord::Y>(prev_pos)
		 	 	   << ">"
				   << std::endl;
				break;
			} else {
				std::cout << "Mouse Clicked. Waiting " << button[mouse_click] << " but received " << button[xevent.xbutton.button] << std::endl;
				ret_val = true;
				break;
			}
		}
	}
	ungrab_pointer();

	return ret_val;
}

bool MlMouseListener::wait_press(const int mouse_click) 
{
	bool ret_val = false;
	XEvent xevent;

	grab_pointer();
	timer.start();
	
	while (timer.wait()) {
		XNextEvent(display.display_ptr.get(), &xevent);
		if (xevent.type == ButtonPress) {
			if (xevent.xbutton.button == mouse_click) {
				ret_val = true;
				prev_pos = std::make_tuple(xevent.xmotion.x_root, xevent.xmotion.y_root);
				std::cout << "Mouse clicked " << button[mouse_click] << ": <" << xevent.xmotion.x_root
					  << ", "
					  << xevent.xmotion.y_root
					  << ">"
					  << std::endl;
				ungrab_pointer();
				XEvent event;
				memset(&event, 0x00, sizeof(event));

				event.type = ButtonPress;
				event.xbutton.button = mouse_click;
				event.xbutton.same_screen = True;

				XQueryPointer(display.display_ptr.get(), RootWindow(display.display_ptr.get(), DefaultScreen(display.display_ptr.get())), &event.xbutton.root, &event.xbutton.window, &event.xbutton.x_root, &event.xbutton.y_root, &event.xbutton.x, &event.xbutton.y, &event.xbutton.state);

				if(XSendEvent(display.display_ptr.get(), PointerWindow, True, 0xff, &event) == 0) 
					std::cout << "Failed sending signal" << std::endl;
				XFlush(display.display_ptr.get());
				grab_pointer();
			}
		}
	}

	return ret_val;
}

Position MlMouseListener::get_wait()
{
	return prev_pos;
}

void MlMouseListener::grab_pointer() const
{
	// Calling this function will block all of the other app to listen mouse event.
	// Hence, don't forget to 'ungrab' it once you are done.
	XGrabPointer(display.display_ptr.get(),
		     display.window,
		     true,
		     ButtonPressMask,
		     GrabModeAsync,
		     GrabModeAsync,
		     None,
		     None,
		     CurrentTime);
	std::cout << "Pointer grabbed.\n";
}

void MlMouseListener::ungrab_pointer() const
{
	// forgetting ungrab pointer will block all of the mouse event
	XUngrabPointer(display.display_ptr.get(), CurrentTime);
	std::cout << "Pointer ungrabbed.\n";
}

const char *MlMouseListener::button[4] = {"Undefined button",
			   "Button Left",
			   "Button Middle",
			   "Button Right"};

// MlKeyboardListener Implementation
MlKeyboardListener::MlKeyboardListener(MlDisplay& display, const std::chrono::milliseconds& ms)
	: MlInputListener(display, ms) {}

bool MlKeyboardListener::get_press(const int key)
{
	bool ret_val = false;
	auto character = std::string(static_cast<char>(key), 1);
	XEvent xevent;

	grab_keyboard();
	while (true) {
		XNextEvent(display.display_ptr.get(), &xevent);

      		if (xevent.type == KeyPress) {
        		unsigned kc = ((XKeyPressedEvent*)&xevent)->keycode;
			const char* s = XKeysymToString(XkbKeycodeToKeysym(display.display_ptr.get(), kc, 0, 0));
 
			if(strlen(s) && character == s) {

				std::cout << "Keyboard pressed : key \'" << character << "\'" << std::endl;
				break;
			} else {
				ret_val = true;
				std::cout << "Keyboard pressed. Waiting " << character << " but received " << s << std::endl;
			}
		}
	}
	ungrab_keyboard();

	return ret_val;
}

bool MlKeyboardListener::wait_press(const int key)
{
	bool ret_val = false;
	auto character = std::string(static_cast<char>(key), 1);
	bool got = false;
	XEvent xevent;

	grab_keyboard();
	timer.start();
	
	while (timer.wait()) {
		if (!got) {
			XNextEvent(display.display_ptr.get(), &xevent);
 
      			if (xevent.type == KeyPress) {
        			unsigned kc = ((XKeyPressedEvent*)&xevent)->keycode;
				const char* s = XKeysymToString(XkbKeycodeToKeysym(display.display_ptr.get(), kc, 0, 0));
 
				if(strlen(s) && character == s) {
					ret_val = got = true;
				}
			}
		}
	}

	return ret_val;
}

void MlKeyboardListener::grab_keyboard() const
{
	XGrabKeyboard(display.display_ptr.get(), 
			display.window,
                        false, 
                        GrabModeAsync, 
                        GrabModeAsync, 
                        CurrentTime);
	std::cout << "Keyboard grabbed.\n";
}

void MlKeyboardListener::ungrab_keyboard() const
{
	// forgetting ungrab pointer will block all of the mouse event
	XUngrabKeyboard(display.display_ptr.get(), CurrentTime);
	std::cout << "Keyboard ungrabbed.\n";
}

