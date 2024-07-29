#include "Matt_daemon.hpp"
#include <fstream>
#include <iomanip>

class Tintin_reporter
{

public:
    Tintin_reporter();
    Tintin_reporter &operator=(Tintin_reporter const &src);
    Tintin_reporter &operator<<(std::string const &message);
    Tintin_reporter(Tintin_reporter const &src);
    ~Tintin_reporter();

    void log(std::string const &message);

private:
    std::string const LOG_FILE = "/var/log/matt_daemon/matt_daemon.log";
    std::string const LOG_FOLDER = "/var/log/matt_daemon";
    std::ofstream file;
};