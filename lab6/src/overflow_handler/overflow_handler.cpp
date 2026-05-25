#include "overflow_handler.hpp"

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/signal.h>
#include <unistd.h>

namespace {
    struct sigaction prev_sa_sigsegv;
    struct sigaction prev_sa_sigbus;

    void overflow_sigsegv_handler(int signal, siginfo_t* info, void* ucontext) {
        const char *overflow_msg = "Program termination due to memory overflow\n";

        write(STDERR_FILENO, overflow_msg, strlen(overflow_msg));
        std::exit(EXIT_FAILURE);
    }
}

void add_overflow_handler() {
    struct sigaction sig_action = {0};
    sig_action.sa_flags = SA_SIGINFO;
    sig_action.sa_sigaction = &overflow_sigsegv_handler;
    sigemptyset(&sig_action.sa_mask);

    if (sigaction(SIGSEGV, &sig_action, &prev_sa_sigsegv)) {
        std::perror("Unable to set overflow_handler for SIGSEGV");
        std::exit(EXIT_FAILURE);
    }

    if (sigaction(SIGBUS, &sig_action, &prev_sa_sigbus)) {
        std::perror("Unable to set overflow_handler for SIGBUS");
        std::exit(EXIT_FAILURE);
    }
}
