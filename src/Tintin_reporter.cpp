#include "Tintin_reporter.hpp"

Tintin_reporter::Tintin_reporter()
{

    if (!getuid()) // Secu car global
    {
        mkdir(LOG_FOLDER.c_str(), 0755);
        file.open(LOG_FILE, std::ios::app);
        if (!file.is_open())
        {
            std::cerr << "Failed to open log file\n";
            exit(1);
        }
    }
}

Tintin_reporter::Tintin_reporter(Tintin_reporter const &src)
{
    *this = src;
}

Tintin_reporter &Tintin_reporter::operator=(__attribute__((unused)) Tintin_reporter const &src)
{
    return *this;
}

Tintin_reporter &Tintin_reporter::operator<<(std::string const &message)
{
    log(message);
    return *this;
}

Tintin_reporter::~Tintin_reporter()
{
    file.close();
}

void Tintin_reporter::log(std::string const &message)
{
    std::time_t t = std::time(nullptr);
    std::tm tm = *std::localtime(&t);
    file << std::put_time(&tm, "[%d/%m/%Y-%H:%M:%S]") << "\t" << message;
    file.flush();
}