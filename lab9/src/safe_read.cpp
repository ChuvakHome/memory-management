#include "safe_read.hpp"

#include <csetjmp>
#include <csignal>

#include <cstdio>
#include <cstdlib>
#include <exception>
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

    if (sigaction(SIGSEGV, &sig_action, nullptr)) {
        perror("Unable to set signal handler for SIGSEV");
        std::terminate();
    }

    if (sigaction(SIGBUS, &sig_action, nullptr)) {
        perror("Unable to set signal handler for SIGBUS");
        std::terminate();
    }

    if (sigsetjmp(safe_read_error_handler, SIG_JMP_MASK)) {
        return std::nullopt;
    }

    return {*p};
}
