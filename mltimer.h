#ifndef MLTIMER_H
#define MLTIMER_H
#include <chrono>
#include <cstdint>

/**
 * MlTimer is a utility class providing
 * basic time counting functions.
 * Stating the template parameter with std::chrono::duration type
 * when declaring this object is required.
 * In default, the templete parameter is std::chrono::milliseconds
 */
template <typename Duration = std::chrono::milliseconds>
class MlTimer {
public:
	// Constructors
	
	/**
	 * Construct a Timer object require templete type 
	 * std::chrono::duration argument.
	 */
	template <class Rep, class Period>
	MlTimer(const std::chrono::duration<Rep, Period>&);

	/**
	 * Calling this constructor to create an object when
	 * Duration type is the same as template parameter typle
	 * of the object.
	 */
	MlTimer(const Duration&);

	// public member functions
	
	/**
	 * This function is to start the time counting.
	 */
	void start() noexcept;

	/**
	 * This function returns a bool to state whether the
	 * time interval of this watch is ended.
	 * If it is not, then it will return a false.
	 */
	bool wait();

	/**
	 * Return current timestamp since epoch time.
	 */
	static int64_t get_timestamp() noexcept;
	
	/**
	 * a timer function to wait for given duration.
	 * parameter :
	 * 	std::chrono::duration
	 */
	template <class Rep, class Period>
	static void wait_for(const std::chrono::duration<Rep, Period>&);
private:
	Duration threshold;
	std::chrono::steady_clock::time_point begin;
};

template <typename Duration>
template <class Rep, class Period>
MlTimer<Duration>::MlTimer(const std::chrono::duration<Rep, Period>& duration)
	: threshold(std::chrono::duration_cast<Duration>(duration))  { }

template <typename Duration>
MlTimer<Duration>::MlTimer(const Duration& duration)
	: threshold(duration) {}

template <typename Duration>
void MlTimer<Duration>::start() noexcept
{
	begin = std::chrono::steady_clock::now();
}

template <typename Duration>
bool MlTimer<Duration>::wait()
{
	using namespace std::chrono;
	return duration_cast<Duration>(steady_clock::now() - begin).count() < threshold.count();
}

template <typename Duration>
int64_t MlTimer<Duration>::get_timestamp() noexcept
{
	using namespace std::chrono;
	return time_point_cast<Duration>(steady_clock::now()).time_since_epoch().count();
}

template <typename Duration>
template <class Rep, class Period>
void MlTimer<Duration>::wait_for(const std::chrono::duration<Rep, Period>& duration)
{
	using namespace std::chrono;
	auto b = steady_clock::now();

	while ((steady_clock::now() - b).count() < duration.count())
		continue;
}

#endif // MLTIMER_H
