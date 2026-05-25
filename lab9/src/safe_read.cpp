#include "safe_read.hpp"

#include <csetjmp>
#include <csignal>

#include <cstdio>
#include <cstdlib>
#include <optional>
#include <sys/signal.h>

#define SIG_JMP_MASK 1

static sigjmp_buf safe_read_error_handler;

static void sigaction_handler(int signal) {
    siglongjmp(safe_read_error_handler, true);
}

std::optional<std::uint8_t> safe_read_uint8(const std::uint8_t *p) {
    struct sigaction sig_action = {0};
    sigemptyset(&sig_action.sa_mask);
    sig_action.sa_handler = sigaction_handler;

    struct sigaction prev_sa_sigsegv, prev_sa_sigbus;

    if (sigaction(SIGSEGV, &sig_action, &prev_sa_sigsegv)) {
        perror("Unable to set signal handler for SIGSEV");
        std::exit(EXIT_FAILURE);
    }

    if (sigaction(SIGBUS, &sig_action, &prev_sa_sigbus)) {
        perror("Unable to set signal handler for SIGBUS");
        std::exit(EXIT_FAILURE);
    }

    std::optional<std::uint8_t> result_opt = std::nullopt;

    if (!sigsetjmp(safe_read_error_handler, SIG_JMP_MASK)) {
        result_opt = {*p};
    }

    if (sigaction(SIGSEGV, &prev_sa_sigsegv, nullptr)) {
        perror("Unable to set signal handler for SIGSEV");
        std::exit(EXIT_FAILURE);
    }

    if (sigaction(SIGBUS, &prev_sa_sigbus, nullptr)) {
        perror("Unable to set signal handler for SIGBUS");
        std::exit(EXIT_FAILURE);
    }

    return result_opt;
}
