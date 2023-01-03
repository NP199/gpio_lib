#include <string>
namespace GPIO {

    void list(std::string const& gpio_chip);

    class Pin {
        public:
        Pin(std::string const& gpio_chip_, int offset_);
        ~Pin();
        void write(bool newValue);
        int read();
        void poll_falling();
        void poll_rising();

        private:
        int const offset;
        std::string const gpio_chip;
    };
}
