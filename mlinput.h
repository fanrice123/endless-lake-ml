#ifndef MLINPUT_H
#define MLINPUT_H
#include <chrono>
#include <tuple>
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
 * implemented virtual functions:
 * 	void set_wait_for(const std::chrono::milliseconds&);
 */
class MlInputListener {
public:
	/** 
	 * To initialise an inputListener, 
	 * pass in a display object and time limit to wait for a button press
	 * by calling function wait_press.
	 */
	MlInputListener(MlDisplay&, const std::chrono::milliseconds&);
	virtual ~MlInputListener();

	// member functions
	
	/**
	 * this function is expected to wait until a key/button is pressed
	 */
	virtual bool get_press(const int) = 0;

	/**
	 * this function is expected to wait for a key/button being pressed
	 * given the time limit provided when constructing the object of inputListener itself
	 * or using function set_wait_for to reset a new time limit.
	 */
	virtual bool wait_press(const int) = 0;

	/**
	 * this function is used for reseting time limit of waiting a 
	 * key/button being press by calling function wait_press.
	 * the type of time should be std::chrono::milliseconds.
	 */
	virtual void set_wait_for(const std::chrono::milliseconds&);
protected:
	MlDisplay display;
	MlTimer<std::chrono::milliseconds> timer;
};

/**
 * Implementation of mouse event listener
 * Implemented a helper function get_wait() to assist the job of wait_press.
 */
class MlMouseListener : public MlInputListener {
public:
	enum MouseClick {LEFT_CLICK = 1, MIDDLE_CLICK, RIGHT_CLICK};

	// constructor
	
	/**
	 * constructor of mouseListener
	 * required parameters:
	 * 	display object
	 * 	time limit for waiting a button press
	 * 
	 * Note that the time limit can be any duration as long as
	 * it is a templete type std::chrono::duration.
	 */
	template <class Rep, class Period>
	MlMouseListener(MlDisplay&, const std::chrono::duration<Rep, Period>&);
	
	/**
	 * constructor of mouseListener
	 * parameter list is similar to template constructor above
	 * but only eccept time limit of type std::chrono::milliseconds.
	 */
	MlMouseListener(MlDisplay&, const std::chrono::milliseconds& = std::chrono::milliseconds(0));

	// member functions
	
	/**
	 * This function is used for waiting a user click
	 * parameter:
	 * 	enum MouseClick
	 */
	bool get_press(const int) override;

	/**
	 * this function is used for waiting a user click within a time limit
	 * given by calling function set_time_for() or constructing object
	 * parameter:
	 * 	enum MouseClick
	 */
	bool wait_press(const int) override;

	/**
	 * helper function of wait_press() to get last clicked position
	 * while executing wait_press().
	 */
	Position get_wait();
	
private:
	// private member functions
	
	/**
	 * grabbing pointer so u can listen mouse event.
	 */
	void grab_pointer() const;
	/**
	 * ungrab pointer so other task or application can listen
	 * mouse event.
	 */
	void ungrab_pointer() const;

	Position prev_pos;

	/**
	 * name of button on mouse :
	 * 	0 : undefined
	 * 	1 : left click
	 * 	2 : middle click
	 * 	3 : right click
	 */
	static const char *button[4];
};


/**
 * keyboard event listener
 */
class MlKeyboardListener : public MlInputListener {
public:
	// constructor
	
	/**
	 * Similar to MlMouseListener except this is for keyboard event.
	 */
	template <class Rep, class Period>
	MlKeyboardListener(MlDisplay&, const std::chrono::duration<Rep, Period>&);

	/**
	 * Similar to MlMouseListener except this is for keyboard event.
	 */
	MlKeyboardListener(MlDisplay&, const std::chrono::milliseconds& = std::chrono::milliseconds(0));

	// member functions
	
	
	/**
	 * Similar to function MlMouseListener::get_press() except this is for keyboard event.
	 * parameter:
	 * 	char expected character
	 */
	bool get_press(const int) override;

	/**
	 * Similar to MlMouseListener except this is for keyboard event.
	 * parameter:
	 * 	char expected character
	 */
	bool wait_press(const int) override;

private:
	// private member functions
	
	/**
	 * grab the keyboard so this program can listen to keyboard event.
	 */
	void grab_keyboard() const;

	/**
	 * release the keyboard grab so other program can listen to keyboard
	 * event.
	 */
	void ungrab_keyboard() const;
};

/// template function implementation

// MlMouseListener constructor
template <class Rep, class Period>
MlMouseListener::MlMouseListener(MlDisplay& display, const std::chrono::duration<Rep, Period>& duration)
	: MlInputListener(display, std::chrono::duration_cast<std::chrono::milliseconds>(duration)) {}


// MlKeyboardListener constructor
template <class Rep, class Period>
MlKeyboardListener::MlKeyboardListener(MlDisplay& display, const std::chrono::duration<Rep, Period>& duration)
	: MlInputListener(display, std::chrono::duration_cast<std::chrono::milliseconds>(duration)) {}


#endif // MLINPUT_H
