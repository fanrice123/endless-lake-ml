#ifndef MLINPUT_H
#define MLINPUT_H
#include <chrono>
#include <tuple>
#include <atomic>
#include <X11/Xlib.h>
#include "mldisplay.h"
#include "mltimer.h"
#include "position.h"

/**
 * MlInputListener is an abstraction of input devices with button.
 * Its child class has to implement function :
 * 	bool get_press(const int);
 * 	bool wait_press(const int);
 * 
 */
class MlInputListener {
public:
	enum MouseClick {NONE, LEFT_CLICK, MIDDLE_CLICK, RIGHT_CLICK};
	
	/** 
	 * To initialise an inputListener, 
	 * pass in a display object and time limit to wait for a button press
	 * by calling function wait_press.
	 */
	/**
	 * constructor of inputListener
	 * required parameters:
	 * 	display object
	 * 	time limit for waiting a button press
	 * 
	 * Note that the time limit can be any duration as long as
	 * it is a templete type std::chrono::duration.
	 */
	MlInputListener(MlDisplay&, const std::chrono::milliseconds& = std::chrono::milliseconds(0));
	template <class Rep, class Period>
	MlInputListener(MlDisplay&, const std::chrono::duration<Rep, Period>&);

	// member functions
	
	/**
	 * this function is expected to wait until a mouse click event emitted.
	 * parameter :
	 * 	enum MouseClick
	 */
	bool get_click(const MouseClick);
	
	/**
	 * this function is expected to wait until a key is pressed.
	 * parameter :
	 * 	char 
	 */
	bool get_press(const char);

	/**
	 * this function is expected to wait for a mouse click event given the
	 * time limit provided when constructing the object of inputListener itself
	 * or using function set_wait_for to reset a new time limit.
	 */
	bool wait_click(const MouseClick);

	/**
	 * this function is expected to wait for a key being pressed given the
	 * time limit provided when constructing the object of inputListener itself
	 * or using function set_wait_for to reset a new time limit.
	 */
	bool wait_press(const char);

	/**
	 * this function is used for reseting time limit of waiting a 
	 * key/button being press by calling function wait_press.
	 * the type of time should be std::chrono::milliseconds.
	 */
	void set_wait_for(const std::chrono::milliseconds&);

	template <class Rep, class Period>
	void set_wait_for(const std::chrono::duration<Rep, Period>&);

	/**
	 * a getter to check if function get_click() has finished execution.
	 */
	bool has_got_click() const noexcept;
	
	/**
	 * a getter to check if function get_press() has finished execution.
	 */
	bool has_got_press() const noexcept;
	
	/**
	 * helper function of wait_press() and get_press to get last clicked position
	 * while executing wait_press() and get_press().
	 */
	Position get_prev_position();

	void listen_for(const std::chrono::milliseconds&, MouseClick, const char = 0);
private:
	// private member function
	
	/**
	 * grabbing pointer so user can listen mouse event.
	 */
	void grab_pointer() const;

	/**
	 * ungrab pointer so other task or application can listen
	 * mouse event.
	 */
	void ungrab_pointer() const;

	/**
	 * grabbing keyboard so user can listen keyboard event.
	 */
	void grab_keyboard() const;

	/**
	 * ungrab pointer so other task or application can listen
	 * mouse event.
	 */
	void ungrab_keyboard() const;

	MlDisplay display;
	MlTimer<std::chrono::milliseconds> timer;
	bool clicked, pressed;
	
	Position prev_pos;
	
	/**
	 * name of button on mouse :
	 * 	0 : undefined
	 * 	1 : left click
	 * 	2 : middle click
	 * 	3 : right click
	 */
	static const char *mouse_button[4];

};


/// template function implementation

// MlInputListener constructor
template <class Rep, class Period>
MlInputListener::MlInputListener(MlDisplay& display, const std::chrono::duration<Rep, Period>& duration)
	: display(display),
	  timer(std::chrono::duration_cast<std::chrono::milliseconds>(duration)), 
	  clicked(false),
	  pressed(false)
{}

// MlInputListener member functions

template <class Rep, class Period>
void MlInputListener::set_wait_for(const std::chrono::duration<Rep, Period>& time)
{
	set_wait_for(std::chrono::duration_cast<const std::chrono::milliseconds&>(time));
}

#endif // MLINPUT_H
