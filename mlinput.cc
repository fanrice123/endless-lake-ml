#include "mlinput.h"
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <chrono>
#include <iostream>
#include <tuple>
#include <cstring>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#include "mldisplay.h"
#define MS_LEFT(X) ((X) & 0x1)
#define MS_RIGHT(X) (((X) & 0x2) >> 1)
#define MS_MIDDLE(X) (((X) & 0x4) >> 2)

// MlInputListener Implementation
MlInputListener::MlInputListener(MlDisplay& display, const std::chrono::milliseconds& ms)
	: display(display), 
      timer(ms), 
      clicked(false), 
      global_clicked(false), 
      pressed(false),
      mice_fd(open("/dev/input/mice", O_RDONLY | O_NONBLOCK))
{
    if (mice_fd < 0)
        throw new std::runtime_error("Open \"/dev/input/mice\" failed.");

}

bool MlInputListener::has_got_click() const noexcept
{
	return clicked;
}

bool MlInputListener::has_got_press() const noexcept
{
	return pressed;
}

bool MlInputListener::get_click(const MouseClick mouse_click)
{
	clicked = false;
	XEvent xevent;

	grab_pointer();
	while (true) {
		XNextEvent(display.display_ptr.get(), &xevent);
		if (xevent.type == ButtonPress) {
			if (xevent.xbutton.button == mouse_click) {
				prev_pos = { xevent.xmotion.x_root, xevent.xmotion.y_root };
				std::cout << "Mouse clicked " << mouse_button[mouse_click] << ": <" << prev_pos.x
				   << ", "
				   << prev_pos.y
		 	 	   << ">"
				   << std::endl;
				clicked = true;
				break;
			} else {
				std::cout << "Mouse Clicked. Waiting " << mouse_button[mouse_click] << " but received " << mouse_button[xevent.xbutton.button] << std::endl;
			}
		}
	}
	ungrab_pointer();

	return clicked;
}

bool MlInputListener::get_press(const char key)
{
	pressed = false;
	auto character = std::string(1, static_cast<char>(key));
	XEvent xevent;

	grab_keyboard();
	while (true) {
		XNextEvent(display.display_ptr.get(), &xevent);

      		if (xevent.type == KeyPress) {
        		unsigned kc = ((XKeyPressedEvent*)&xevent)->keycode;
			const char* s = XKeysymToString(XkbKeycodeToKeysym(display.display_ptr.get(), kc, 0, 0));
 
			if(strlen(s) && character == s) {

				std::cout << "Keyboard pressed : key \'" << character << "\'" << std::endl;
				pressed = true;
				break;
			} else {
				std::cout << "Keyboard pressed. Waiting " << character << " but received " << s << std::endl;
			}
		}
	}
	ungrab_keyboard();
	return pressed;
}

bool MlInputListener::global_wait_click(const MouseClick mouse_click, const std::chrono::milliseconds& ms)
{

    MlTimer<std::chrono::milliseconds> watch(ms);

    uint8_t data[3];

    global_clicked = false;

    watch.start();
    while (watch.wait()) {
        int count;
        if (!global_clicked) {
            count = read(mice_fd, data, sizeof(data));
            if (count >= 0) {

                switch (mouse_click) {
                case LEFT_CLICK:
                    global_clicked = MS_LEFT(data[0]);
                    break;
                case MIDDLE_CLICK:
                    global_clicked = MS_MIDDLE(data[0]);
                    break;
                case RIGHT_CLICK:
                    global_clicked = MS_RIGHT(data[0]);
                    break;
                }
            //} else if (count < 0 && errno == EAGAIN) {

            }
        }
    }

    return global_clicked;
}
    

bool MlInputListener::wait_click(const MouseClick mouse_click) 
{
	clicked = false;
	XEvent xevent;

	grab_pointer();
	timer.start();
	
	while (timer.wait()) {
		while (XPending(display.display_ptr.get()))
			XNextEvent(display.display_ptr.get(), &xevent);
		if (xevent.type == ButtonPress) {
			if (xevent.xbutton.button == mouse_click) {
				clicked = true;
				prev_pos = { xevent.xmotion.x_root, xevent.xmotion.y_root };
				std::cout << "Mouse clicked " << mouse_button[mouse_click] << ": <" << xevent.xmotion.x_root
					  << ", "
					  << xevent.xmotion.y_root
					  << ">"
					  << std::endl;
			}
		}
	}
	ungrab_pointer();
	if (!clicked) {
		prev_pos = { -1, -1 };
	}

	return clicked;
}

bool MlInputListener::wait_press(const char key)
{
	pressed = false;
	auto character = std::string(static_cast<char>(key), 1);
	XEvent xevent;

	grab_keyboard();
	timer.start();
	
	while (timer.wait()) {
		if (!pressed) {
			while (XPending(display.display_ptr.get()))
				XNextEvent(display.display_ptr.get(), &xevent);
      			if (xevent.type == KeyPress) {
        			unsigned kc = ((XKeyPressedEvent*)&xevent)->keycode;
				const char* s = XKeysymToString(XkbKeycodeToKeysym(display.display_ptr.get(), kc, 0, 0));
 
				if(strlen(s) && character == s) {
					pressed = true;
					std::cout << "Keyboard pressed : key \'" << character << "\'" << std::endl;
				}
			}
		}
	}
	ungrab_keyboard();

	return pressed;
}

void MlInputListener::set_wait_for(const std::chrono::milliseconds& ms)
{
	timer = decltype(timer)(ms);
}

Position MlInputListener::get_prev_position()
{
	return prev_pos;
}

void MlInputListener::grab_pointer() const
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

void MlInputListener::ungrab_pointer() const
{
	// forgetting ungrab pointer will block all of the mouse event
	XUngrabPointer(display.display_ptr.get(), CurrentTime);
	std::cout << "Pointer ungrabbed.\n";
}

void MlInputListener::grab_keyboard() const
{
	XGrabKeyboard(display.display_ptr.get(), 
			display.window,
                        false, 
                        GrabModeAsync, 
                        GrabModeAsync, 
                        CurrentTime);
	std::cout << "Keyboard grabbed.\n";
}

void MlInputListener::ungrab_keyboard() const
{
	// forgetting ungrab pointer will block all of the mouse event
	XUngrabKeyboard(display.display_ptr.get(), CurrentTime);
	std::cout << "Keyboard ungrabbed.\n";
}

const char *MlInputListener::mouse_button[4] = {"Undefined button",
			   "Button Left",
			   "Button Middle",
			   "Button Right"};


