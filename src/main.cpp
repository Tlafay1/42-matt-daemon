#include "Matt_daemon.hpp"

int main(__attribute__((unused)) int argc, __attribute__((unused)) char *argv[])
{
    if (getuid())
        std::cout << "This program must be run as root\nQuitting !\n";
    return 0;
}