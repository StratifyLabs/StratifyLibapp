#ifndef CHRONO_MICRO_TIMER_HPP
#define CHRONO_MICRO_TIMER_HPP

#include "MicroTime.hpp"

namespace chrono {

/*! \brief Timer Class
 * \details This class implements a logical timer based on the Stratify OS
 * system timer.
 *
 *
 *
 * Physical timers are controlled using the hal::Tmr class.
 *
 * Here is an example of using the timer to time events.
 * \code
 * #include <sapi/sys.hpp>
 *
 * int main(int argc, char * argv[]){
 * 	Timer t;
 * 	t.start(); //start
 * 	MicroTimer::wait(500);
 * 	t.stop();
 * 	printf("Timer value after 500usec is %d\n", t.usec());
 *
 * }
 * \endcode
 *
 * The output of the above code varies depending on the MCU clock cycles and the
 * scheduler.  The following is a sample output.
 *
 *     Timer value after 500usec is 502
 *
 *
 */
class MicroTimer : public api::ChronoWorkObject {
public:
#ifdef __MCU_ONLY__
    static int init(Tmr::port_t port, u32 heartbeat = 10000);
#endif

    /*! \details This method will delay the specified number of seconds.
     *
     * @param timeout Time to wait in seconds
     */
    static void wait_seconds(u32 timeout);
    static void wait_sec(u32 timeout){ wait_seconds(timeout); }

    /*! \details This method will delay the specified number of milliseconds.
     *
     * @param timeout Time to wait in milliseconds
     */
    static void wait_milliseconds(u32 timeout){ wait(MicroTime::from_milliseconds(timeout)); }
    static void wait_msec(u32 timeout){ wait(MicroTime::from_msec(timeout)); }

    /*! \details This method will delay the specified number of microseconds.
     *
     * @param timeout Time to wait in microseconds
     */
    static void wait_microseconds(u32 timeout){ wait(timeout); }
    static void wait_usec(u32 timeout){ wait(timeout); }

    /*! \details This method will delay based on the specified MicroTime object.
     *
     * @param micro_time A reference to the MicroTime object
     *
     * Here are some examples:
     *
     * \code
     * MicroTimer::wait(MicroTime::from_msec(100)); //delay 100 milliseconds
     * MicroTimer::wait(1000); //delay 1000 microseconds -- MicroTime() is auto constructed using microseconds
     * MicroTimer::wait(MicroTime::from_sec(5)); //delay for 5 seconds
     * \endcode
     *
     *
     */
    static void wait(const MicroTime & micro_time);


#if !defined __link

    /*! \details Constructs an empty MicroTimer. */
    MicroTimer();


    /*! \details Returns the microseconds on the real-time clock (starts counting on device reset) */
    static int get_clock_microseconds();
    static int get_clock_usec() { return get_clock_microseconds(); }

    /*! \details Returns the milliseconds on the real-time clock (starts counting on device reset) */
    static int get_clock_milliseconds();
    static int get_clock_msec(){ return get_clock_milliseconds(); }

    /*! \details Returns the seconds on the real-time clock (starts counting on device reset) */
    static int get_clock_seconds();
    static int get_clock_sec(){ return get_clock_seconds(); }

    /*! \details Starts the timer.
     *
     * If the timer is currently running, this method has no effect.  If the timer has been
     * stopped, it will restart.  Use resume() to resume running a stopped timer.
     *
     */
    void start(); //start counting

    /*! \details Restarts the timer.
     *
     * If the timer is currently running,
     * it starts over.  If it is not running, it is started.
     *
     */
    void restart();


    /*! \details Resumes counting after a stop().
     *
     * If the timer is currently running
     * this method has no effect.  If the timer has not been started, this method will
     * start the timer.
     */
    void resume();

    /*! \details Returns true if the timer is currently counting meaning
     * it has been started but has not been stopped.
     *
     * If the timer has been stopped and resumed, this method will return true;
     *
     */
    bool is_running() const { return !is_stopped(); }

    /*! \details Returns true if the timer has been started.
     *
     * It the timer has been started and stopped, this method will return true.
     * If the timer has been reset() or never started, this method will return false.
     *
     */
    bool is_started() const { return (m_start.tv_nsec + m_start.tv_sec) != 0; }

    /*! \details Returns true if the timer is stopped.
     *
     * If the timer has not yet been started or has been reset(), this method will return true.
     * If the timer is currently running, this method will return false.
     *
     *
     */
    bool is_stopped() const { return (m_stop.tv_sec != (time_t)-1) || (m_stop.tv_sec == 0); }


    /*! \details Returns true if the timer is in a reset state.
     *
     */
    bool is_reset() const { return m_stop.tv_sec == 0; }

    /*! \details Returns the value of the Timer as
     * a MicroTime object.
     */
    MicroTime calc_value() const;

    //deprecated
    MicroTime value() const { return calc_value(); }



    /*! \details Resets the value of the timer.
     *
     *  After calling this method, is_running(), and is_started() will both
     *  all return false;
     *
     */
    void reset();

    /*! \details Calculates the timer value in milliseconds.
     *
     * @return The number of milliseconds that have elapsed since start.  This value
     * can be read when the timer is running to get a live value or after it has
     * been stopped to get the time elapsed between start() and stop()
     */
    u32 milliseconds() const { return calc_value().milliseconds(); }
    //deprecated
    u32 msec() const { return calc_msec(); }
    u32 calc_msec() const { return calc_value().msec(); }

    /*! \details Calculates the timer value in microseconds.
     *
     * This is similar to calc_msec() but returns
     * the value in microseconds rather than milliseconds.
     *
     * @return The number of microseconds that have elapsed since start.
     */
    u32 microseconds() const { return calc_value(); }

    //deprecated
    u32 usec() const { return calc_value(); }
    u32 calc_usec() const { return calc_value(); }


    /*! \details Calculates the timer value in seconds.
     *
     * This is similar to calc_msec() but returns
     * the value in seconds rather than milliseconds.
     *
     * @return The number of seconds that have elapsed since start.
     */
    u32 seconds() const { return calc_value().seconds(); }

    //deprecated
    u32 calc_sec() const;
    u32 sec() const { return calc_sec(); }

    /*! \details Stops the timer from counting.
     *
     * Subsequent calls to value() will return the same number.  Once the timer has been stopped(),
     * a call to resume() will resume counting and a call to start() or restart() will restart
     * counting from zero.
     */
    void stop();

#endif

private:
#if !defined __link
    struct timespec m_start;
    struct timespec m_stop;
#endif
};

}

#endif // CHRONO_MICRO_TIMER_HPP