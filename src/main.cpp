#include "Matt_daemon.hpp"
#include "Tintin_reporter.hpp"

int main(__attribute__((unused)) int argc, __attribute__((unused)) char *argv[])
{
    if (getuid())
    {
        std::cout << "This program must be run as root\nQuitting !\n";
        return 1;
    }

    if (access("/var/lock/matt_daemon.lock", F_OK) != -1)
    {
        std::cout << "Daemon already running\nQuitting !\n";
        return 1;
    }

    // Set up the lock file and start the fork
    if (!DEBUG) {
        int lock_file = open("/var/lock/matt_daemon.lock", O_CREAT | O_RDWR, 0666);
        if (lock_file == -1) {
            std::cerr << "Failed to open lock file\n";
            return 1;
        }

        if (lockf(lock_file, F_TLOCK, 0) == -1) {
            std::cerr << "Failed to lock file\n";
            close(lock_file);
            return 1;
        }

        pid_t pid = fork();
        if (pid == -1) {
            std::cerr << "Failed to fork\n";
            close(lock_file);
            return 1;
        }

        if (pid > 0) {
            // Parent process
            close(lock_file);
            return 0;
        }
        // Write PID to lock file
        char pid_str[16];
        std::snprintf(pid_str, sizeof(pid_str), "%d", getpid());
        write(lock_file, pid_str, std::strlen(pid_str));

        // Child process
        setsid();
        chdir("/");
        umask(0);
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
    }
    Tintin_reporter reporter;

    // Create a socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "Failed to create socket\n";
        return 1;
    }

    // Set socket options to allow reuse of address
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        std::cerr << "Failed to set socket options\n";
        close(server_fd);
        return 1;
    }

    // Define server address
    struct sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the socket to the address
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        std::cerr << "Failed to bind socket\n";
        close(server_fd);
        return 1;
    }

    // Listen for incoming connections
    if (listen(server_fd, MAX_CLIENTS + 1) == -1) {
        std::cerr << "Failed to listen on socket\n";
        close(server_fd);
        return 1;
    }

    if (DEBUG)
        std::cout << "Listening on port " << PORT << "\n";
    reporter << "Listening on port " + std::to_string(PORT) + "\n";

    // Set up the file descriptor set
    fd_set readfds;
    int max_sd = server_fd;
    int active_connections = 0;  // To track the number of active connections

    while (true) {
        // Clear the socket set and add server socket to the set
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);

        // Add all active client sockets to the set
        for (int fd = 0; fd <= max_sd; fd++) {
            if (FD_ISSET(fd, &readfds)) {
                FD_SET(fd, &readfds);
            }
        }

        // Wait for an activity on one of the sockets
        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0 && errno != EINTR) {
            std::cerr << "select() error: " << strerror(errno) << "\n";
            break; // Exit the loop on select error
        }

        // If something happened on the master socket, it's an incoming connection
        if (FD_ISSET(server_fd, &readfds)) {
            if (active_connections < 3) {
                int new_socket = accept(server_fd, NULL, NULL);
                if (new_socket < 0) {
                    std::cerr << "accept() error\n";
                    continue;
                }
                if (DEBUG)
                    std::cout << "New connection, socket fd is " << new_socket << "\n";
                reporter << "New connection, socket fd is " + std::to_string(new_socket) + "\n";
                if (new_socket > max_sd) {
                    max_sd = new_socket;
                }
                active_connections++;
            } else {
                // Reject new connections if there are already 3 active connections
                int reject_socket = accept(server_fd, NULL, NULL);
                if (reject_socket >= 0) {
                    if (DEBUG)
                        std::cout << "Rejected connection, socket fd is " << reject_socket << " (Too many active connections)\n";
                    reporter << "Rejected connection, socket fd is " + std::to_string(reject_socket) + " (Too many active connections)\n";
                    close(reject_socket);
                }
            }
        }

        // Handle IO on other sockets
        for (int fd = 0; fd <= max_sd; fd++) {
            if (FD_ISSET(fd, &readfds) && fd != server_fd) {
                char buffer[1024];
                ssize_t bytes_received = recv(fd, buffer, sizeof(buffer) - 1, 0);
                if (bytes_received == 0) {
                    // Connection closed
                    if (DEBUG)
                        std::cout << "Client disconnected, socket fd is " << fd << "\n";
                    reporter << "Client disconnected, socket fd is " + std::to_string(fd) + "\n";
                    close(fd);
                    FD_CLR(fd, &readfds);
                    active_connections--;

                    // Recalculate max_sd if needed
                    if (fd == max_sd) {
                        while (max_sd > server_fd && !FD_ISSET(max_sd, &readfds)) {
                            max_sd--;
                        }
                    }
                } else if (bytes_received > 0) {
                    buffer[bytes_received] = '\0';
                    if (DEBUG)
                        std::cout << "Received from fd " << fd << ": " << buffer;
                    reporter << "Received from fd " + std::to_string(fd) + ": " + buffer;
                } else {
                    std::cerr << "recv() error\n";
                    close(fd);
                    FD_CLR(fd, &readfds);
                    active_connections--;
                }
            }
        }
    }

    close(server_fd);
    return 0;
}