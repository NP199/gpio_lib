#include <string>
/*
enum gpio_v2_line_flag {
	GPIO_V2_LINE_FLAG_USED			        = _BITULL(0),
	GPIO_V2_LINE_FLAG_ACTIVE_LOW		    = _BITULL(1),
	GPIO_V2_LINE_FLAG_INPUT			        = _BITULL(2),
	GPIO_V2_LINE_FLAG_OUTPUT		        = _BITULL(3),
	GPIO_V2_LINE_FLAG_EDGE_RISING		    = _BITULL(4),
	GPIO_V2_LINE_FLAG_EDGE_FALLING		    = _BITULL(5),
	GPIO_V2_LINE_FLAG_OPEN_DRAIN		    = _BITULL(6),
	GPIO_V2_LINE_FLAG_OPEN_SOURCE		    = _BITULL(7),
	GPIO_V2_LINE_FLAG_BIAS_PULL_UP		    = _BITULL(8),
	GPIO_V2_LINE_FLAG_BIAS_PULL_DOWN	    = _BITULL(9),
	GPIO_V2_LINE_FLAG_BIAS_DISABLED		    = _BITULL(10),
	GPIO_V2_LINE_FLAG_EVENT_CLOCK_REALTIME  = _BITULL(11),
	GPIO_V2_LINE_FLAG_EVENT_CLOCK_HTE	    = _BITULL(12),
};
000000000000

*/


namespace GPIO {

    void list(std::string const& gpio_chip);

    class Pin {
        public:
        Pin(std::string const& gpio_chip_, int offset_);
        ~Pin();
        void write(bool newValue);
        int read();
        void poll_(std::uint16_t bitmask);
        void poll_falling();
        void poll_rising();

        private:
        int const offset;
        std::string const gpio_chip;
    };
}
