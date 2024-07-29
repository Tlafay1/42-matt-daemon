#include "Matt_daemon.hpp"
#include "Tintin_reporter.hpp"

int main(__attribute__((unused)) int argc, __attribute__((unused)) char *argv[])
{
    if (getuid())
        std::cout << "This program must be run as root\nQuitting !\n";

    if (access("/var/lock/matt_daemon.lock", F_OK) != -1)
    {
        std::cout << "Daemon already running\nQuitting !\n";
        return 1;
    }
    return 0;
}