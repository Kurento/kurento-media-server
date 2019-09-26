/*

  Copyright (c) 2012, Samsung R&D Institute Russia
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice, this
     list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 */

/*! @file death_handler.cc
 *  @brief Implementation of the SIGSEGV/SIGABRT handler which prints the debug
 *  stack trace.
 *  @author Markovtsev Vadim <v.markovtsev@samsung.com>
 *  @version 1.0
 *  @license Simplified BSD License
 *  @copyright 2012 Samsung R&D Institute Russia
 */

#include "death_handler.hpp"
#include <assert.h>
#include <cxxabi.h>
#include <execinfo.h>
#include <malloc.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <dlfcn.h>

#include <string>

#pragma GCC poison malloc realloc free backtrace_symbols \
  printf fprintf sprintf snprintf scanf sscanf  // NOLINT

#define checked(x) do { if ((x) <= 0) _Exit(EXIT_FAILURE); } while (false)

#define INLINE __attribute__((always_inline)) inline

namespace Debug
{

/// @brief This namespace contains some basic supplements
/// of the needed libc functions which potentially use heap.
namespace Safe
{
/// @brief Converts an integer to a preallocated string.
/// @pre base must be less than or equal to 16.
INLINE char *itoa (int val, char *memory, int base = 10)
{
  char *res = memory;

  if (val == 0) {
    res[0] = '0';
    res[1] = '\0';
    return res;
  }

  const int res_max_length = 32;
  int i;
  bool negative = val < 0;
  res[res_max_length - 1] = 0;

  for (i = res_max_length - 2; val != 0 && i != 0; i--, val /= base) {
    res[i] = "0123456789ABCDEF"[val % base];
  }

  if (negative) {
    res[i--] = '-';
  }

  return &res[i + 1];
}

/// @brief Converts an unsigned integer to a preallocated string.
/// @pre base must be less than or equal to 16.
INLINE char *utoa (uint64_t val, char *memory, int base = 10)
{
  char *res = memory;

  if (val == 0) {
    res[0] = '0';
    res[1] = '\0';
    return res;
  }

  const int res_max_length = 32;
  int i;
  res[res_max_length - 1] = 0;

  for (i = res_max_length - 2; val != 0 && i != 0; i--, val /= base) {
    res[i] = "0123456789ABCDEF"[val % base];
  }

  return &res[i + 1];
}

/// @brief Converts a pointer to a preallocated string.
INLINE char *ptoa (const void *val, char *memory)
{
  char *buf = utoa (reinterpret_cast<uint64_t> (val), memory + 32, 16);
  char *result = memory;  // 32
  strcpy (result + 2, buf); // NOLINT
  result[0] = '0';
  result[1] = 'x';
  return result;
}

/// @brief Reentrant printing to stderr.
INLINE void print2stderr (const char *msg, size_t len = 0)
{
  if (len > 0) {
    checked (write (STDERR_FILENO, msg, len) );
  } else {
    checked (write (STDERR_FILENO, msg, strlen (msg) ) );
  }
}
}  // namespace Safe

const size_t DeathHandler::kNeededMemory = 12288;
bool DeathHandler::generate_core_dump_ = true;
bool DeathHandler::cleanup_ = true;
#ifdef QUICK_EXIT
bool DeathHandler::quick_exit_ = false;
#endif
int DeathHandler::frames_count_ = 16;
bool DeathHandler::cut_common_path_root_ = true;
bool DeathHandler::cut_relative_paths_ = true;
bool DeathHandler::append_pid_ = false;
bool DeathHandler::color_output_ = true;
bool DeathHandler::thread_safe_ = true;
char *DeathHandler::memory_ = NULL;

typedef void (*sa_sigaction_handler) (int, siginfo_t *, void *);

DeathHandler::DeathHandler()
{
  if (memory_ == NULL) {
    memory_ = new char[kNeededMemory];
  }

  struct sigaction sa;

  sa.sa_sigaction = (sa_sigaction_handler) SignalHandler;

  sigemptyset (&sa.sa_mask);

  sa.sa_flags = SA_RESTART | SA_SIGINFO | SA_ONSTACK;

  sigaction (SIGSEGV, &sa, NULL);

  sigaction (SIGABRT, &sa, NULL);

  sigaction (SIGFPE, &sa, NULL);
}

DeathHandler::~DeathHandler()
{
  struct sigaction sa;

  sigaction (SIGSEGV, NULL, &sa);
  sa.sa_handler = SIG_DFL;
  sigaction (SIGSEGV, &sa, NULL);

  sigaction (SIGABRT, NULL, &sa);
  sa.sa_handler = SIG_DFL;
  sigaction (SIGABRT, &sa, NULL);

  sigaction (SIGFPE, NULL, &sa);
  sa.sa_handler = SIG_DFL;
  sigaction (SIGFPE, &sa, NULL);
  delete[] memory_;
}

bool DeathHandler::generate_core_dump()
{
  return generate_core_dump_;
}

void DeathHandler::set_generate_core_dump (bool value)
{
  generate_core_dump_ = value;
}

bool DeathHandler::cleanup()
{
  return cleanup_;
}

void DeathHandler::set_cleanup (bool value)
{
  cleanup_ = value;
}

#ifdef QUICK_EXIT
bool DeathHandler::quick_exit()
{
  return quick_exit_;
}

void DeathHandler::set_quick_exit (bool value)
{
  quick_exit_ = value;
}
#endif

int DeathHandler::frames_count()
{
  return frames_count_;
}

void DeathHandler::set_frames_count (int value)
{
  assert (value > 0 && value <= 100);
  frames_count_ = value;
}

bool DeathHandler::cut_common_path_root()
{
  return cut_common_path_root_;
}

void DeathHandler::set_cut_common_path_root (bool value)
{
  cut_common_path_root_ = value;
}

bool DeathHandler::cut_relative_paths()
{
  return cut_relative_paths_;
}

void DeathHandler::set_cut_relative_paths (bool value)
{
  cut_relative_paths_ = value;
}

bool DeathHandler::append_pid()
{
  return append_pid_;
}

void DeathHandler::set_append_pid (bool value)
{
  append_pid_ = value;
}

bool DeathHandler::color_output()
{
  return color_output_;
}

void DeathHandler::set_color_output (bool value)
{
  color_output_ = value;
}

bool DeathHandler::thread_safe()
{
  return thread_safe_;
}

void DeathHandler::set_thread_safe (bool value)
{
  thread_safe_ = value;
}

INLINE static void safe_abort()
{
  struct sigaction sa;
  sigaction (SIGABRT, NULL, &sa);
  sa.sa_handler = SIG_DFL;
  kill (getppid(), SIGCONT);
  sigaction (SIGABRT, &sa, NULL);
  abort();
}

/// @brief Invokes addr2line utility to determine the function name
/// and the line information from an address in the code segment.
static char *addr2line (const char *image, void *addr, bool color_output,
                        char **memory)
{
  int pipefd[2];

  if (pipe (pipefd) != 0) {
    safe_abort();
  }

  pid_t pid = fork();

  if (pid == 0) {
    close (pipefd[0]);
    dup2 (pipefd[1], STDOUT_FILENO);
    dup2 (pipefd[1], STDERR_FILENO);

    if (execlp ("addr2line", "addr2line",
                Safe::ptoa (addr, *memory), "-f", "-C", "-e", image,
                reinterpret_cast<void *> (NULL) ) == -1) {
      safe_abort();
    }
  }

  close (pipefd[1]);
  const int line_max_length = 4096;
  char *line = *memory;
  *memory += line_max_length;
  ssize_t len = read (pipefd[0], line, line_max_length);
  close (pipefd[0]);

  if (len == 0) {
    safe_abort();
  }

  line[len] = 0;

  if (waitpid (pid, NULL, 0) != pid) {
    safe_abort();
  }

  if (line[0] == '?') {
    char *straddr = Safe::ptoa (addr, *memory);

    if (color_output) {
      strcpy (line, "\033[32;1m"); // NOLINT
    }

    strcat (line, straddr); // NOLINT

    if (color_output) {
      strcat (line, "\033[0m"); // NOLINT
    }

    strcat (line, " at "); // NOLINT
    strcat (line, image); // NOLINT
    strcat (line, " "); // NOLINT
  } else {
    if (* (strstr (line, "\n") + 1) == '?') {
      char *straddr = Safe::ptoa (addr, *memory);
      strcpy (strstr (line, "\n") + 1, image); // NOLINT
      strcat (line, ":"); // NOLINT
      strcat (line, straddr); // NOLINT
      strcat (line, "\n"); // NOLINT
    }
  }

  return line;
}

/// @brief Returns non stripped version of the library.
static std::string nonStripped (const char *image)
{
  int pipefd[2];

  if (pipe (pipefd) != 0) {
    safe_abort();
  }

  pid_t pid = fork();

  if (pid == 0) {
    close (pipefd[0]);
    dup2 (pipefd[1], STDOUT_FILENO);
    dup2 (pipefd[1], STDERR_FILENO);

    if (execlp ("file", "file", "-L", image,
                reinterpret_cast<void *> (NULL) ) == -1) {
      safe_abort();
    }
  }

  close (pipefd[1]);
  const int line_max_length = 4096;
  char l[4096];
  ssize_t len = read (pipefd[0], l, line_max_length);
  close (pipefd[0]);

  if (len == 0) {
    safe_abort();
  }

  l[len] = 0;

  if (waitpid (pid, NULL, 0) != pid) {
    safe_abort();
  }

  std::string line (l);

  if (line.find ("not stripped") != std::string::npos) {
    /* Library is not stripped */
    return std::string (image);
  }

  const std::string BUILD_ID = "BuildID[sha1]=";
  std::size_t position = line.find (BUILD_ID);

  if (position == std::string::npos) {
    return std::string (image);
  }

  line = line.substr (position + BUILD_ID.length() );

  position = line.find (",");

  if (position == std::string::npos) {
    return std::string (image);
  }

  line = line.substr (0, position);

  std::string dir = line.substr (0, 2);
  std::string file = line.substr (2);

  dir = "/usr/lib/debug/.build-id/" + dir;

  file = dir + "/" + file + ".debug";

  // If debug file exists use it
  if (access ( file.c_str(), F_OK ) != -1 ) {
    return file;
  }

  return image;
}

/// @brief Used to workaround backtrace() usage of malloc().
void *DeathHandler::MallocHook (size_t size,
                                const void * /* caller */)
{
  char *malloc_buffer = memory_ + kNeededMemory - 512;

  if (size > 512U) {
    const char *msg = "malloc() replacement function should not return "
                      "a memory block larger than 512 bytes\n";
    Safe::print2stderr (msg, strlen (msg) + 1);
    _Exit (EXIT_FAILURE);
  }

  return malloc_buffer;
}

#if (__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

void DeathHandler::SignalHandler (int sig, void * /* info */, void *secret)
{
  // Stop all other running threads by forking
  pid_t forkedPid = fork();

  if (forkedPid != 0) {
    int status;

    if (thread_safe_) {
      // Freeze the original process, until it's child prints the stack trace
      kill (getpid(), SIGSTOP);
      // Wait for the child without blocking and exit as soon as possible,
      // so that no zombies are left.
      waitpid (forkedPid, &status, WNOHANG);
    } else {
      // Wait for the child, blocking only the current thread.
      // All other threads will continue to run, potentially crashing the parent.
      waitpid (forkedPid, &status, 0);
    }

#ifdef QUICK_EXIT

    if (quick_exit_) {
      ::quick_exit (EXIT_FAILURE);
    }

#endif

    if (generate_core_dump_) {
      struct sigaction sa;
      sigaction (SIGABRT, NULL, &sa);
      sa.sa_handler = SIG_DFL;
      sigaction (SIGABRT, &sa, NULL);
      abort();
    } else {
      if (cleanup_) {
        exit (EXIT_FAILURE);
      } else {
        _Exit (EXIT_FAILURE);
      }
    }
  }

  ucontext_t *uc = reinterpret_cast<ucontext_t *> (secret);

  if (dup2 (STDERR_FILENO, STDOUT_FILENO) == -1) { // redirect stdout to stderr
    Safe::print2stderr ("Failed to redirect stdout to stderr\n");
  }

  char *memory = memory_;
  {
    char *msg = memory;
    const int msg_max_length = 128;

    if (color_output_) {
      // \033[31;1mSegmentation fault\033[0m \033[33;1m(%i)\033[0m\n
      strcpy (msg, "\033[31;1m"); // NOLINT
    } else {
      msg[0] = '\0';
    }

    switch (sig) {
    case SIGSEGV:
      strcat (msg, "Segmentation fault"); // NOLINT
      break;

    case SIGABRT:
      strcat (msg, "Aborted"); // NOLINT
      break;

    case SIGFPE:
      strcat (msg, "Floating point exception"); // NOLINT
      break;

    default:
      strcat (msg, "Caught signal "); // NOLINT
      strcat (msg, Safe::itoa (sig, msg + msg_max_length) ); // NOLINT
      break;
    }

    if (color_output_) {
      strcat (msg, "\033[0m"); // NOLINT
    }

    strcat (msg, " (thread "); // NOLINT

    if (color_output_) {
      strcat (msg, "\033[33;1m"); // NOLINT
    }

    strcat (msg, Safe::utoa (pthread_self(), msg + msg_max_length) ); // NOLINT

    if (color_output_) {
      strcat (msg, "\033[0m"); // NOLINT
    }

    strcat (msg, ", pid "); // NOLINT

    if (color_output_) {
      strcat (msg, "\033[33;1m"); // NOLINT
    }

    strcat (msg, Safe::itoa (getppid(), msg + msg_max_length) ); // NOLINT

    if (color_output_) {
      strcat (msg, "\033[0m"); // NOLINT
    }

    strcat (msg, ")"); // NOLINT
    Safe::print2stderr (msg);
  }

  Safe::print2stderr ("\nStack trace:\n");
  void **trace = reinterpret_cast<void **> (memory);
  memory += (frames_count_ + 2) * sizeof (void *);
  // Workaround malloc() inside backtrace()
  void * (*oldMallocHook) (size_t, const void *) = __malloc_hook;
  void (*oldFreeHook) (void *, const void *) = __free_hook;
  __malloc_hook = MallocHook;
  __free_hook = NULL;
  int trace_size = backtrace (trace, frames_count_ + 2);
  __malloc_hook = oldMallocHook;
  __free_hook = oldFreeHook;

  if (trace_size <= 2) {
    safe_abort();
  }

  // Overwrite sigaction with caller's address
#if defined(__arm__)
  trace[1] = reinterpret_cast<void *> (uc->uc_mcontext.arm_pc);
#elif defined(__aarch64__)     
  trace[1] = reinterpret_cast<void *> (uc->uc_mcontext.pc);
#else
#if !defined(__i386__) && !defined(__x86_64__)
#error Only ARM, x86 and x86-64 are supported
#endif
#if defined(__x86_64__)
  trace[1] = reinterpret_cast<void *> (uc->uc_mcontext.gregs[REG_RIP]);
#else
  trace[1] = reinterpret_cast<void *> (uc->uc_mcontext.gregs[REG_EIP]);
#endif
#endif

  const int path_max_length = 2048;
  char *name_buf = memory;
  ssize_t name_buf_length = readlink ("/proc/self/exe", name_buf,
                                      path_max_length - 1);

  if (name_buf_length < 1) {
    safe_abort();
  }

  name_buf[name_buf_length] = 0;
  memory += name_buf_length + 1;
  char *cwd = memory;

  if (getcwd (cwd, path_max_length) == NULL) {
    safe_abort();
  }

  strcat (cwd, "/"); // NOLINT
  memory += strlen (cwd) + 1;
  char *prev_memory = memory;

  int stackOffset = trace[2] == trace[1] ? 2 : 1;

  for (int i = stackOffset; i < trace_size; i++) {
    memory = prev_memory;
    char *line;
    Dl_info dlinf;

    if (dladdr (trace[i], &dlinf) == 0 || dlinf.dli_fname[0] != '/' ||
        !strcmp (name_buf, dlinf.dli_fname) ) {
      line = addr2line (name_buf, trace[i], color_output_, &memory);
    } else {
      std::string name = nonStripped (dlinf.dli_fname);
      line = addr2line (name.c_str(), reinterpret_cast<void *> (
                          reinterpret_cast<char *> (trace[i]) -
                          reinterpret_cast<char *> (dlinf.dli_fbase) ),
                        color_output_, &memory);
    }

    char *function_name_end = strstr (line, "\n");

    if (function_name_end != NULL) {
      *function_name_end = 0;
      {
        // "\033[34;1m[%s]\033[0m \033[33;1m(%i)\033[0m\n
        char *msg = memory;
        const int msg_max_length = 512;

        if (color_output_) {
          strcpy (msg, "\033[34;1m"); // NOLINT
        } else {
          msg[0] = 0;
        }

        strcat (msg, "["); // NOLINT
        strcat (msg, line); // NOLINT
        strcat (msg, "]"); // NOLINT

        if (append_pid_) {
          if (color_output_) {
            strcat (msg, "\033[0m\033[33;1m"); // NOLINT
          }

          strcat (msg, " ("); // NOLINT
          strcat (msg, Safe::itoa (getppid(), msg + msg_max_length) ); // NOLINT
          strcat (msg, ")"); // NOLINT

          if (color_output_) {
            strcat (msg, "\033[0m"); // NOLINT
          }

          strcat (msg, "\n"); // NOLINT
        } else {
          if (color_output_) {
            strcat (msg, "\033[0m"); // NOLINT
          }

          strcat (msg, "\n"); // NOLINT
        }

        Safe::print2stderr (msg);
      }
      line = function_name_end + 1;

      // Remove the common path root
      if (cut_common_path_root_) {
        int cpi;

        for (cpi = 0; cwd[cpi] == line[cpi]; cpi++) {};

        if (line[cpi - 1] != '/') {
          for (; line[cpi - 1] != '/'; cpi--) {};
        }

        if (cpi > 1) {
          line = line + cpi;
        }
      }

      // Remove relative path root
      if (cut_relative_paths_) {
        char *path_cut_pos = strstr (line, "../");

        if (path_cut_pos != NULL) {
          path_cut_pos += 3;

          while (!strncmp (path_cut_pos, "../", 3) ) {
            path_cut_pos += 3;
          }

          line = path_cut_pos;
        }
      }

      // Mark line number
      if (color_output_) {
        char *number_pos = strstr (line, ":");

        if (number_pos != NULL) {
          char *line_number = memory;  // 128
          strcpy (line_number, number_pos); // NOLINT
          // Overwrite the new line char
          line_number[strlen (line_number) - 1] = 0;
          // \033[32;1m%s\033[0m\n
          strcpy (number_pos, "\033[32;1m"); // NOLINT
          strcat (line, line_number); // NOLINT
          strcat (line, "\033[0m\n"); // NOLINT
        }
      }
    }

    // Overwrite the new line char
    line[strlen (line) - 1] = 0;

    // Append pid
    if (append_pid_) {
      // %s\033[33;1m(%i)\033[0m\n
      strcat (line, " "); // NOLINT

      if (color_output_) {
        strcat (line, "\033[33;1m"); // NOLINT
      }

      strcat (line, "("); // NOLINT
      strcat (line, Safe::itoa (getppid(), memory) ); // NOLINT
      strcat (line, ")"); // NOLINT

      if (color_output_) {
        strcat (line, "\033[0m"); // NOLINT
      }
    }

    strcat (line, "\n"); // NOLINT
    Safe::print2stderr (line);
  }

  // Write '\0' to indicate the end of the output
  char end = '\0';
  checked (write (STDERR_FILENO, &end, 1) );

  if (thread_safe_) {
    // Resume the parent process
    kill (getppid(), SIGCONT);
  }

  // This is called in the child process
  _Exit (EXIT_SUCCESS);
}

#if (__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)
#pragma GCC diagnostic pop
#endif
#ifdef __clang__
#pragma clang diagnostic pop
#endif

}  // namespace Debug
