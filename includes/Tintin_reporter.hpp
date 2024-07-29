#include "Matt_daemon.hpp"
#include <fstream>
#include <iomanip>

class Tintin_reporter
{

public:
    Tintin_reporter();
    Tintin_reporter &operator=(Tintin_reporter const &src);
    Tintin_reporter(Tintin_reporter const &src);
    ~Tintin_reporter();

    void log(std::string const &message);

private:
    std::string const LOG_FILE = "/var/log/matt_daemon.log";
    std::ofstream file;
};