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

    MemoryRegion pool_prot_region;

    inline void call_prev_sighandler(struct sigaction *prev_sighandler, int signal, siginfo_t *info, void *ucontext) {
        if ((prev_sighandler->sa_flags & SA_SIGINFO) != 0) {
            prev_sighandler->sa_sigaction != nullptr
                ? prev_sighandler->sa_sigaction(signal, info, ucontext)
                : (void) 0;
        } else if (prev_sighandler->sa_handler == SIG_DFL) {
            sigaction(signal, prev_sighandler, nullptr);
            raise(signal);
        } else if (prev_sighandler->sa_handler != SIG_IGN) {
            prev_sighandler->sa_handler != nullptr
                ? prev_sighandler->sa_handler(signal)
                : (void) 0;
        }
    }

    void overflow_signal_handler(int signal, siginfo_t *info, void *ucontext) {
        const char *overflow_msg = "Program termination due to memory overflow\n";

        if (pool_prot_region.contains(static_cast<std::byte *>(info->si_addr))) {
            write(STDERR_FILENO, overflow_msg, strlen(overflow_msg));
            std::exit(EXIT_FAILURE);
        }

        call_prev_sighandler(
            &(signal == SIGSEGV ? prev_sa_sigsegv : prev_sa_sigbus),
            signal,
            info,
            ucontext
        );
    }
}

void add_overflow_handler() {
    struct sigaction sig_action = {0};
    sig_action.sa_flags = SA_SIGINFO;
    sig_action.sa_sigaction = &overflow_signal_handler;
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

void add_memory_protected_region(MemoryRegion region) {
    pool_prot_region = region;
}

void remove_memory_protected_region() {
    pool_prot_region = MemoryRegion{nullptr, 0};
}
