#include "stddef.h"
#include "stdio.h"
#include "string.h"
#include "windows.h"

#define BUF_SIZE 256

HANDLE log_file;

void LogInternal(const char *str) {
  fputs(str, stdout);

  DWORD num_bytes = strlen(str);
  DWORD num_bytes_written;
  if (WriteFile(log_file, str, num_bytes, &num_bytes_written, NULL) == 0) {
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
  snprintf(buf, sizeof(buf), "Exiting due to %s", GetCtrlType(dwCtrlType));
  Log(buf);
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
      Sleep(INFINITE);
    }
    if (strcmp(buf, "stop\n") == 0) {
      Log("Exiting due to \"stop\" command");
      break;
    }
  }

  Log("Simulating cleanup on exit...");
  Sleep(5000);
  Log("Exiting after simulated cleanup");
  return 0;
}
