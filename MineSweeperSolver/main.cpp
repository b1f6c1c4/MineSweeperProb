#include <csignal>
#include <iostream>
#include <set>
#include <sys/sysinfo.h>
#include <sys/wait.h>
#include <unistd.h>
#include <nlohmann/json.hpp>

#include "random.h"
#include "facade.hpp"

NLOHMANN_JSON_SERIALIZE_ENUM(LogicMethod, {
    {LogicMethod::Passive, "PL"},
    {LogicMethod::Single, "SL"},
    {LogicMethod::SingleExtended, "SLE"},
    {LogicMethod::Double, "DL"},
    {LogicMethod::DoubleExtended, "DLE"},
    {LogicMethod::Full, "FL"},
})

std::string to_string(const std::vector<HeuristicMethod> &dt) {
    if (dt.empty())
        return "NH";
    std::string str;
    for (auto m: dt)
        switch (m) {
            case HeuristicMethod::None:
                str.push_back(' ');
                break;
            case HeuristicMethod::MinMineProb:
                str.push_back('P');
                break;
            case HeuristicMethod::MaxZeroProb:
                str.push_back('Z');
                break;
            case HeuristicMethod::MaxZerosProb:
                str.push_back('S');
                break;
            case HeuristicMethod::MaxZerosExp:
                str.push_back('E');
                break;
            case HeuristicMethod::MaxQuantityExp:
                str.push_back('Q');
                break;
            case HeuristicMethod::MinFrontierDist:
                str.push_back('F');
                break;
            case HeuristicMethod::MaxUpperBound:
                str.push_back('U');
                break;
        }
    return str;
}

static pid_t g_monitor;
static std::sig_atomic_t g_exiting = 0;

void sig_handler(int signal) {
    (void)signal;
    switch (g_exiting++) {
        case 0:
            break;
        case 1:
            if (g_monitor)
                kill(g_monitor, SIGKILL);
            break;
        default:
            std::raise(SIGKILL);
            break;
    }
}

void sig_empty(int signal) {
    (void)signal;
}

[[noreturn]] void worker_entry(int fd[2], const Configuration &cfg) {
    std::signal(SIGTERM, SIG_DFL);

    SeedEngine();

    alarm(600);
    while (true) {
        char c;
        try {
            auto res = run(cfg);
            c = res ? 'S' : 'F';
        } catch (std::exception &e) {
            c = 'E';
        }
        alarm(600);
        if (write(fd[1], &c, 1) != 1) {
            if (errno == EPIPE)
                exit(0);
            perror("worker/write(3)");
            exit(3);
        }
    }
}

[[noreturn]] void monitor_entry(int fd[2], const Configuration &cfg) {
    close(fd[0]);

    struct sigaction sa = {};
    sa.sa_handler = &sig_empty;
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGQUIT, &sa, nullptr);
    sigaction(SIGPIPE, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);

    // spawn workers
    std::set<pid_t> workers;
    auto n = get_nprocs();
    for (auto i = 0; i < n; i++)
        if (auto p = fork(); !p)
            worker_entry(fd, cfg);
        else
            workers.insert(p);

    // monitor worker exits
    while (true) {
        int ws;
        auto pid = waitpid(-1, &ws, 0);
        if (pid < 0 && errno != EINTR) {
            perror("monitor/waitpid(3)");
            exit(3);
        }

        // Verify if main process is still there
        char ch;
        if (pid <= 0) {
            ch = 't';
        } else {
            if (WIFSIGNALED(ws)) {
                switch (WTERMSIG(ws)) {
                    case SIGALRM: // timeout
                        std::cerr << "\nWarning: Worker " << pid << " timeout\n";
                        ch = 'T';
                        break;
                    case SIGSEGV: // error
                        std::cerr << "\nWarning: Worker " << pid << " killed by SIGSEGV\n";
                        ch = 'E';
                        break;
                    case SIGABRT: // error
                        std::cerr << "\nWarning: Worker " << pid << " killed by SIGABRT\n";
                        ch = 'E';
                        break;
                    default:
                        std::cerr << "\nWarning: Worker " << pid << " killed by " << WTERMSIG(ws) << "\n";
                        ch = 'U';
                        break;
                }
            } else if (WIFEXITED(ws)) {
                if (WEXITSTATUS(ws) != 0) {
                    std::cerr << "\nWarning: Worker " << pid << " exited with " << WEXITSTATUS(ws) << "\n";
                    ch = 'U';
                } else {
                    ch = 't';
                }
            } else {
                std::cerr << "\nWarning: Worker " << pid << " wtf-ed\n";
                ch = 'U';
            }
            workers.erase(pid);
        }
        if (write(fd[1], &ch, 1) != 1) {
            if (errno == EPIPE)
                break;
            perror("monitor/write(3)");
            exit(3);
        }

        // replenish workers
        if (pid > 0) {
            if (auto p = fork(); !p)
                worker_entry(fd, cfg);
            else
                workers.insert(p);
        }
    }

    // Exiting: send SIGTERM and then SIGKILL
    for (auto p : workers)
        if (kill(p, SIGKILL))
            if (errno != ESRCH)
                perror("monitor/kill(3)");
    while (true) {
        int ws;
        auto pid = waitpid(-1, &ws, 0);
        if (pid > 0) {
            workers.erase(pid);
            continue;
        }
        switch (errno) {
            case ECHILD:
                goto finish;
            case EINTR:
                continue;
            default:
                perror("monitor/waitpid(3)");
                exit(3);
        }
    }
finish:
    close(fd[1]);
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc != 2 && argc != 3) {
        std::cout << "Usage: " << argv[0]
                  << R"( [PSDF]L(@\[<I>,<J>\])?-(NH|Pure|[PZSEQFU]+)(-D<D>)?-<W>-<H>-T<M>-(SFAR|SNR) [<number>])"
                  << std::endl;
        return 2;
    }

    auto cfg = parse(argv[1]);
    cache(cfg);

    if (argc == 2) {
        SeedEngine();

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
        while (true) {
            auto res = run(cfg);
            std::cout << (res ? 'S' : 'F');
            std::flush(std::cout);
        }
#pragma clang diagnostic pop
    }

    auto total_num = std::atol(argv[2]);
    auto received = 0l;
    auto succeeded = 0l;
    auto errored = 0l;
    auto timeout = 0l;
    auto strange = 0l;
    auto report = [&]{
        std::cerr << received << "/" << total_num
                  << " (" << 100.0 * static_cast<double>(received) / static_cast<double>(total_num)
                  << "%): "
                  << succeeded << "/" << received
                  << " (" << 100.0 * static_cast<double>(succeeded) / static_cast<double>(received)
                  << "%) P, "
                  << errored << " E, "
                  << timeout << " T, "
                  << strange << " U\r";
    };

    // Process Hierarchy:
    //
    // main: read from fd[0]
    //   monitor: ensure enough workers are running
    //      worker: compute and write to fd[1]
    //      worker: compute and write to fd[1]
    //      ...

    int fd[2];
    if (pipe(fd)) {
        perror("main/pipe(3)");
        return 1;
    }

    if (!(g_monitor = fork()))
        monitor_entry(fd, cfg);
    close(fd[1]);
    std::cerr << "Main PID: " << getpid() << "  Monitor PID: " << g_monitor << "\n";

    struct sigaction sa = {};
    sa.sa_handler = &sig_handler;
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGQUIT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);

    char buf[16384];
    while (received < total_num) {
        if (g_exiting)
            break;
        if (auto len = read(fd[0], buf, sizeof(buf)); len > 0) {
            for (auto i = 0z; i < len; i++)
                switch (buf[i]) {
                    case 'S':
                        succeeded++, received++;
                        break;
                    case 'F':
                        received++;
                        break;
                    case 'T':
                        timeout++;
                        break;
                    case 'E':
                        errored++;
                        break;
                    case 'U':
                        strange++;
                        break;
                }
            report();
        } else if (len == 0) {
            std::cerr << "Warning: Pipe closed too soon\n";
            break;
        } else if (errno != EINTR) {
            perror("main/read(3)");
            exit(3);
        }
    }

    close(fd[0]);
    kill(g_monitor, SIGTERM);
    int ws;
    if (waitpid(g_monitor, &ws, 0) < 0) {
        perror("main/waitpid(3)");
        exit(3);
    }

    nlohmann::json j;
    j["string"] = argv[1];
    j["game"]["width"] = cfg.Width;
    j["game"]["height"] = cfg.Height;
    j["game"]["mines"] = cfg.TotalMines;
    j["game"]["snr"] = cfg.IsSNR;
    j["strategy"]["logic"] = cfg.Logic;
    if (!cfg.InitialPositionSpecified)
        j["strategy"]["initial"] = nullptr;
    else
        j["strategy"]["initial"] = { { "x", cfg.Index % cfg.Width + 1 }, { "y", cfg.Index / cfg.Width + 1 } };
    if (!cfg.HeuristicEnabled)
        j["strategy"]["heuristic"] = "Pure";
    else {
        j["strategy"]["heuristic"] = to_string(cfg.DecisionTree);
    }
    if (!cfg.ExhaustEnabled)
        j["strategy"]["exhaust"] = 0;
    else
        j["strategy"]["exhaust"] = cfg.ExhaustCriterion;
    if (!cfg.PruningEnabled)
        j["strategy"]["pruning"] = 0;
    else
        j["strategy"]["pruning"] = cfg.PruningCriterion;
    j["result"]["pass"] = succeeded;
    j["result"]["fail"] = received - succeeded;
    j["result"]["error"] = errored;
    j["result"]["timeout"] = timeout;
    j["result"]["strange"] = strange;
    std::cout << j << std::endl;
}
