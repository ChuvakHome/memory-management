#include "overflow_handler.hpp"

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/signal.h>
#include <unistd.h>
#include <utility>

#include "../allocator/pool_registry.hpp"

namespace {
    struct sigaction prev_sa_sigsegv;
    struct sigaction prev_sa_sigbus;

    std::size_t stringify_number(char buffer[], std::size_t n, std::size_t num) {
        std::size_t len;

        for (len = 0; len < n && num > 10; ++len) {
            buffer[len] = '0' + (num % 10);
            num /= 10;
        }

        buffer[len++] = '0' + num;
        buffer[len] = 0;

        for (std::size_t i = 0; i < len / 2; ++i) {
            std::swap(buffer[i], buffer[len - i - 1]);
        }

        return len;
    }

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
        char buf[128] = "Program termination due to memory overflow in pool \0";

        std::size_t i = pool_registry::find_pool(static_cast<std::byte *>(info->si_addr));

        if (i != pool_registry::npos) {
            std::size_t n = strlen(buf);

            std::size_t len = stringify_number(buf + n, sizeof(buf), i);
            n += len;

            buf[n++] = '\n';
            buf[n] = 0;

            write(STDERR_FILENO, buf, n);
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
