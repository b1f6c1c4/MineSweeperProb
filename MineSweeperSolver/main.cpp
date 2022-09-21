#include <csignal>
#include <iostream>
#include <vector>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/wait.h>

#include "facade.hpp"

enum {
    SIG_NONE = 0,
    SIG_EXIT = 1,
    SIG_CHLD = 2,
};
static std::sig_atomic_t flag = SIG_NONE;

void sig_handler(int signal) {
    switch (signal) {
        case SIGINT:
        case SIGTERM:
            flag = SIG_EXIT;
            break;
        case SIGCHLD:
            flag = SIG_CHLD;
            break;
    }
}

[[noreturn]] void worker(int fd[2], const Configuration &cfg) {
    close(fd[0]);
    alarm(30);
    while (true) {
        auto res = run(cfg);
        alarm(30);
        char c = res ? 'S' : 'F';
        if (write(fd[1], &c, 1) != 1) {
            if (errno == EPIPE)
                exit(0);
            perror("write(2)");
            exit(3);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0]
                  << R"( [PSDF]L(@\[<I>,<J>\])?-(NH|Pure|[PZSEQFU]+)(-D<D>)?-<W>-<H>-T<M>-(SFAR|SNR) <number>)"
                  << std::endl;
        return 2;
    }

    auto total_num = std::atol(argv[2]);
    auto received = 0l;
    auto succeeded = 0l;
    auto report = [&]{
        std::cerr << received << "/" << total_num
                  << " (=" << 100.0 * static_cast<double>(received) / static_cast<double>(total_num)
                  << ") received, "
                  << succeeded << "/" << received
                  << " (=" << 100.0 * static_cast<double>(succeeded) / static_cast<double>(received)
                  << ") succeeded\r";
    };

    auto cfg = parse(argv[1]);
    cache(cfg);

    // parent reads from fd[0]
    // children write to fd[1]
    int fd[2];
    if (pipe(fd)) {
        perror("pipe(2)");
        return 1;
    }

    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);
    signal(SIGCHLD, sig_handler);

    auto n = get_nprocs();
    for (auto i = 0; i < n; i++)
        if (!fork())
            worker(fd, cfg);

    char buf[4096];
    while (received < total_num) {
        switch (flag) {
            case SIG_NONE:
                if (auto len = read(fd[0], buf, sizeof(buf)); len > 0) {
                    for (auto i = 0z; i < len; i++)
                        if (buf[i] == 'S')
                            succeeded++;
                    received += len;
                    report();
                } else if (len == 0) {
                    std::cerr << "Warning: read(2) returned 0\n";
                } else if (errno != EINTR) {
                    perror("read(2)");
                    exit(3);
                }
                break;
            case SIG_EXIT:
                goto finish;
            case SIG_CHLD:
                while (true) {
                    int ws;
                    auto pid = waitpid(-1, &ws, WNOHANG);
                    if (pid < 0) {
                        perror("waitpid(2)");
                        exit(3);
                    }
                    if (!pid)
                        break;
                    std::cerr << "\nWarning: Child " << pid << " exited abnormally " << WEXITSTATUS(ws) << "\n";
                    if (!fork())
                        worker(fd, cfg);
                }
                flag = SIG_NONE;
                break;
        }
    }

finish:
    signal(SIGCHLD, SIG_IGN);
    close(fd[0]);
    report();
    std::cout << argv[1] << std::endl;
    std::cout << received << std::endl;
    std::cout << succeeded << std::endl;
}
