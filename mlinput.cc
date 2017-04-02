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
	: display(display), timer(ms), clicked(false), pressed(false) {}

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
				prev_pos = std::make_tuple(xevent.xmotion.x_root, xevent.xmotion.y_root);
				std::cout << "Mouse clicked " << mouse_button[mouse_click] << ": <" << std::get<Coord::X>(prev_pos)
				   << ", "
				   << std::get<Coord::Y>(prev_pos)
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
				prev_pos = std::make_tuple(xevent.xmotion.x_root, xevent.xmotion.y_root);
				std::cout << "Mouse clicked " << mouse_button[mouse_click] << ": <" << xevent.xmotion.x_root
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
	ungrab_pointer();
	if (!clicked) {
		prev_pos = std::make_tuple(-1, -1);
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

void MlInputListener::listen_for(const std::chrono::milliseconds& ms, MouseClick click, const char key)
{
	pressed = clicked = false;
	if (ms < std::chrono::milliseconds(0))
		return;
	MlTimer<std::chrono::milliseconds> watch(ms);
	auto character = std::string(1, static_cast<char>(key));
	XEvent event, fake_event;
	unsigned kc;
	const char* s;

	watch.start();
	grab_keyboard();
	grab_pointer();
	while (watch.wait()) {
		if ((!pressed || !clicked)) {
			if (XPending(display.display_ptr.get())) {
				XNextEvent(display.display_ptr.get(), &event);
				switch (event.type) {
				case KeyPress:
					kc = ((XKeyPressedEvent*)&event)->keycode;
					s = XKeysymToString(XkbKeycodeToKeysym(display.display_ptr.get(), kc, 0, 0));
					if(strlen(s) && character == s) {
						pressed = true;
						std::cout << "Keyboard pressed : key \'" << character << "\'" << std::endl;
					}
					break;
				case ButtonPress:
					if (event.xbutton.button == click) {
						clicked = true;
						prev_pos = std::make_tuple(event.xmotion.x_root, event.xmotion.y_root);
						std::cout << "Mouse clicked " << mouse_button[click] << ": <" << event.xmotion.x_root
					  	  	  << ", "
					  	  	  << event.xmotion.y_root
					  	  	  << ">"
					  	  	  << std::endl;
						ungrab_pointer();
						memset(&fake_event, 0x00, sizeof(event));

						fake_event.type = ButtonPress;
						fake_event.xbutton.button = click;
						fake_event.xbutton.same_screen = True;

						XQueryPointer(display.display_ptr.get(), RootWindow(display.display_ptr.get(), DefaultScreen(display.display_ptr.get())), &fake_event.xbutton.root, &fake_event.xbutton.window, &fake_event.xbutton.x_root, &fake_event.xbutton.y_root, &fake_event.xbutton.x, &fake_event.xbutton.y, &fake_event.xbutton.state);

						if(XSendEvent(display.display_ptr.get(), PointerWindow, True, 0xff, &fake_event) == 0) 
							std::cout << "Failed sending signal" << std::endl;
						XFlush(display.display_ptr.get());
						grab_pointer();
						break;
					}
				}
			}
		}
	}
	ungrab_keyboard();
	ungrab_pointer();
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


