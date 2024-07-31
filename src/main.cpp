#include "Matt_daemon.hpp"
#include "Tintin_reporter.hpp"

int lock_file;
Tintin_reporter reporter;

void signal_handler(int signal)
{
    if (DEBUG)
        std::cout << "Signal " << signal << " received, exiting\n";
    reporter << "Signal " + std::to_string(signal) + " received, exiting\n";
    if (flock(lock_file, LOCK_UN) == -1)
    {
        std::cerr << "Failed to lock file\n";
        close(lock_file);
    }
    close(lock_file);
    std::remove(LOCK_FILE);
    exit(0);
}

int main(void)
{
    if (getuid())
    {
        std::cerr << "This program must be run as root\nQuitting !\n";
        return 1;
    }

    // Set up the lock file and start the fork
    if (!DEBUG)
    {
        pid_t pid = fork();
        if (pid == -1)
        {
            std::cerr << "Failed to fork\n";
            close(lock_file);
            return 1;
        }

        if (pid > 0)
        {
            // Parent process
            close(lock_file);
            return 0;
        }
        // Write PID to lock file
        // char pid_str[16];
        // std::snprintf(pid_str, sizeof(pid_str), "%d", getpid());
        // write(lock_file, pid_str, std::strlen(pid_str));

        // Child process
        setsid();
        chdir("/");
        umask(0);
    }

    int signals[] = {SIGINT, SIGTERM, SIGQUIT, SIGHUP};
    int num_signals = sizeof(signals) / sizeof(signals[0]);

    // Set up signal handlers
    for (int i = 0; i < num_signals; i++)
    {
        if (signal(signals[i], signal_handler) == SIG_ERR)
        {
            std::cerr << "Can't catch signal " << signals[i] << "\n";
        }
    }

    lock_file = open(LOCK_FILE, O_CREAT | O_WRONLY, 0644);
    if (lock_file == -1)
    {
        std::cerr << "Failed to open lock file\n";
        return 1;
    }

    if (flock(lock_file, LOCK_EX) == -1)
    {
        std::cerr << "Failed to lock file\n";
        close(lock_file);
        return 1;
    }

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        std::cerr << "Failed to create socket\n";
        return 1;
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
        std::cerr << "Failed to set socket options\n";
        close(server_fd);
        return 1;
    }

    struct sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        std::cerr << "Failed to bind socket\n";
        std::cout << strerror(errno);
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, MAX_CLIENTS + 1) == -1)
    {
        std::cerr << "Failed to listen on socket\n";
        close(server_fd);
        return 1;
    }
    if (DEBUG)
        std::cout << "Listening on port " << PORT << "\n";
    reporter << "Listening on port " + std::to_string(PORT) + "\n";

    fd_set readfds;
    int max_sd = server_fd;
    int client_sockets[MAX_CLIENTS] = {0};
    int active_connections = 0;

    while (true)
    {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);

        for (int i = 0; i < MAX_CLIENTS; ++i)
        {
            if (client_sockets[i] > 0)
            {
                FD_SET(client_sockets[i], &readfds);
            }
            if (client_sockets[i] > max_sd)
            {
                max_sd = client_sockets[i];
            }
        }

        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0 && errno != EINTR)
        {
            std::cerr << "select() error: " << strerror(errno) << "\n";
            break;
        }

        if (FD_ISSET(server_fd, &readfds))
        {
            if (active_connections < MAX_CLIENTS)
            {
                int new_socket = accept(server_fd, NULL, NULL);
                if (new_socket < 0)
                {
                    std::cerr << "accept() error\n";
                    continue;
                }
                if (DEBUG)
                    std::cout << "New connection, socket fd is " << new_socket << "\n";
                reporter << "New connection, socket fd is " + std::to_string(new_socket) + "\n";

                // Add new socket to client sockets array
                for (int i = 0; i < MAX_CLIENTS; ++i)
                {
                    if (client_sockets[i] == 0)
                    {
                        client_sockets[i] = new_socket;
                        break;
                    }
                }
                active_connections++;
            }
            else
            {
                int reject_socket = accept(server_fd, NULL, NULL);
                if (reject_socket >= 0)
                {
                    if (DEBUG)
                        std::cout << "Rejected connection, socket fd is " << reject_socket << " (Too many active connections)\n";
                    reporter << "Rejected connection, socket fd is " + std::to_string(reject_socket) + " (Too many active connections)\n";
                    close(reject_socket);
                }
            }
        }

        for (int i = 0; i < MAX_CLIENTS; ++i)
        {
            int fd = client_sockets[i];
            if (FD_ISSET(fd, &readfds))
            {
                char buffer[1024];
                ssize_t bytes_received = recv(fd, buffer, sizeof(buffer) - 1, 0);
                if (bytes_received == 0)
                {
                    if (DEBUG)
                        std::cout << "Client disconnected, socket fd is " << fd << "\n";
                    reporter << "Client disconnected, socket fd is " + std::to_string(fd) + "\n";
                    close(fd);
                    client_sockets[i] = 0;
                    active_connections--;
                }
                else if (bytes_received > 0)
                {
                    buffer[bytes_received] = '\0';
                    if (DEBUG)
                        std::cout << "Received from fd " << fd << ": " << buffer;
                    reporter << "Received from fd " + std::to_string(fd) + ": " + buffer;
                    if (std::string(buffer) == "quit\n")
                    {
                        close(fd);
                        if (DEBUG)
                            std::cout << "Quitting\n";
                        reporter << "Quitting\n";
                        exit(0);
                    }
                }
                else
                {
                    std::cerr << "recv() error: " << strerror(errno) << "\n";
                    close(fd);
                    client_sockets[i] = 0;
                    active_connections--;
                }
            }
        }
    }

    close(server_fd);
    return 0;
}
