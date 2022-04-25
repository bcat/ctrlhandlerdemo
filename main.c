// Copyright 2022 Jonathan Rascher
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
// REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
// LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
// OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include "stddef.h"
#include "stdio.h"
#include "string.h"
#include "windows.h"

#define BUF_SIZE 256
#define STOP_DELAY_MS 10000

HANDLE log_file;

void LogInternal(const char *str) {
  fputs(str, stdout);

  DWORD num_bytes = strlen(str);
  DWORD num_bytes_written;
  if (!WriteFile(log_file, str, num_bytes, &num_bytes_written, NULL)) {
    fprintf(stderr, "WriteFile failed: %lu", GetLastError());
  } else if (num_bytes_written != num_bytes) {
    fprintf(stderr, "WriteFile only wrote %lu bytes (wanted %lu)",
            num_bytes_written, num_bytes);
  }
}

void Log(const char *msg) {
  SYSTEMTIME now;
  GetLocalTime(&now);

  char buf[BUF_SIZE];
  if (GetDateFormat(LOCALE_INVARIANT, 0, &now, NULL, buf, sizeof(buf)) != 0) {
    LogInternal(buf);
    LogInternal(" ");
  } else {
    fprintf(stderr, "GetDateFormat failed: %lu", GetLastError());
  }
  if (GetTimeFormat(LOCALE_INVARIANT, 0, &now, NULL, buf, sizeof(buf)) != 0) {
    LogInternal(buf);
    LogInternal(" ");
  } else {
    fprintf(stderr, "GetTimeFormat failed: %lu", GetLastError());
  }

  LogInternal(msg);
  LogInternal("\n");
}

const char *GetCtrlType(DWORD ctrl_type) {
  switch (ctrl_type) {
  case CTRL_C_EVENT:
    return "CTRL_C_EVENT";
  case CTRL_BREAK_EVENT:
    return "CTRL_BREAK_EVENT";
  case CTRL_CLOSE_EVENT:
    return "CTRL_CLOSE_EVENT";
  case CTRL_LOGOFF_EVENT:
    return "CTRL_LOGOFF_EVENT";
  case CTRL_SHUTDOWN_EVENT:
    return "CTRL_SHUTDOWN_EVENT";
  }
  return "[unknown]";
}

BOOL WINAPI CtrlHandler(DWORD dwCtrlType) {
  char buf[BUF_SIZE];
  snprintf(buf, sizeof(buf), "Received %s", GetCtrlType(dwCtrlType));
  Log(buf);

  // Return false to delegate to the default control handler. This normally
  // calls ExitProcess, but when running as a service the default handler
  // doesn't exit in response to CLOSE, LOGOFF, or SHUTDOWN:
  // https://docs.microsoft.com/en-us/windows/console/handlerroutine#remarks.
  return FALSE;
}

int main(int argc, char **argv) {
  setbuf(stdout, NULL);

  log_file = CreateFile("ctrlhandlerdemo.log", GENERIC_WRITE, FILE_SHARE_READ,
                        NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if (log_file == INVALID_HANDLE_VALUE) {
    fprintf(stderr, "CreateFile failed: %lu", GetLastError());
    return 1;
  }

  if (!SetConsoleCtrlHandler(CtrlHandler, TRUE)) {
    fprintf(stderr, "SetConsoleCtrlHandler failed: %lu", GetLastError());
    return 1;
  }

  Log("Waiting for \"stop\" command...");
  for (;;) {
    char buf[BUF_SIZE];
    if (fgets(buf, sizeof(buf), stdin) == NULL) {
      Log("Received EOF on stdin");

      // We don't want to exit on EOF, so just sleep forever to avoid burning
      // CPU on more fgets calls that will immediately return.
      Sleep(INFINITE);
    }

    if (strcmp(buf, "stop\n") == 0) {
      Log("Exiting due to \"stop\" command");
      break;
    }
  }

  Log("Simulating cleanup on exit...");
  Sleep(STOP_DELAY_MS);
  Log("Exiting after simulated cleanup");
  return 0;
}
