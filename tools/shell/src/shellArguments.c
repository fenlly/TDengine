/*
 * Copyright (c) 2019 TAOS Data, Inc. <jhtao@taosdata.com>
 *
 * This program is free software: you can use, redistribute, and/or modify
 * it under the terms of the GNU Affero General Public License, version 3
 * or later ("AGPL"), as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "shellInt.h"
#include "../../inc/pub.h"
char   configDirShell[PATH_MAX] = {0};

#define TAOS_CONSOLE_PROMPT_CONTINUE "   -> "

#define SHELL_HOST     "The server FQDN to connect. The default host is localhost."
#define SHELL_PORT     "The TCP/IP port number to use for the connection."
#define SHELL_USER     "The user name to use when connecting to the server."
#define SHELL_PASSWORD "The password to use when connecting to the server."
#define SHELL_AUTH     "The auth string to use when connecting to the server."
#define SHELL_GEN_AUTH "Generate auth string from password."
#define SHELL_CFG_DIR  "Configuration directory."
#define SHELL_DMP_CFG  "Dump configuration."
#define SHELL_CMD      "Commands to run without enter the shell."
#define SHELL_RAW_TIME "Output time as uint64_t."
#define SHELL_FILE     "Script to run without enter the shell."
#define SHELL_DB       "Database to use when connecting to the server."
#define SHELL_CHECK    "Check the service status."
#define SHELL_STARTUP  "Check the details of the service status."
#define SHELL_WIDTH    "Set the default binary display width, default is 30."
#define SHELL_NET_ROLE "Net role when network connectivity test, options: client|server."
#define SHELL_PKT_LEN  "Packet length used for net test, default is 1024 bytes."
#define SHELL_PKT_NUM  "Packet numbers used for net test, default is 100."
#define SHELL_BI_MODE  "Set BI mode"
#define SHELL_VERSION  "Print program version."
#define SHELL_DSN      "Use dsn to connect to the cloud server or to a remote server which provides WebSocket connection."
#define SHELL_TIMEOUT  "Set the timeout for WebSocket query in seconds, default is 30."
#define SHELL_LOG_OUTPUT                                                                                              \
  "Specify log output. Options:\n\r\t\t\t     stdout, stderr, /dev/null, <directory>, <directory>/<filename>, "       \
  "<filename>\n\r\t\t\t     * If OUTPUT contains an absolute directory, logs will be stored in that directory "       \
  "instead of logDir.\n\r\t\t\t     * If OUTPUT contains a relative directory, logs will be stored in the directory " \
  "combined with logDir and the relative directory."

#ifdef WEBSOCKET
#define SHELL_DRIVER_DEFAULT "0." // todo simon -> 1
#else
#define SHELL_DRIVER_DEFAULT "0."
#endif

static int32_t shellParseSingleOpt(int32_t key, char *arg);

void shellPrintHelp() {
  char indent[] = "  ";
  printf("Usage: %s [OPTION...] \r\n\r\n", CUS_PROMPT);
  printf("%s%s%s%s\r\n", indent, "-a,", indent, SHELL_AUTH);
  printf("%s%s%s%s\r\n", indent, "-A,", indent, SHELL_GEN_AUTH);
  printf("%s%s%s%s\r\n", indent, "-B,", indent, SHELL_BI_MODE);
  printf("%s%s%s%s\r\n", indent, "-c,", indent, SHELL_CFG_DIR);
  printf("%s%s%s%s\r\n", indent, "-C,", indent, SHELL_DMP_CFG);
  printf("%s%s%s%s\r\n", indent, "-d,", indent, SHELL_DB);
  printf("%s%s%s%s\r\n", indent, "-f,", indent, SHELL_FILE);
  printf("%s%s%s%s\r\n", indent, "-h,", indent, SHELL_HOST);
  printf("%s%s%s%s\r\n", indent, "-k,", indent, SHELL_CHECK);
  printf("%s%s%s%s\r\n", indent, "-l,", indent, SHELL_PKT_LEN);
  printf("%s%s%s%s\r\n", indent, "-n,", indent, SHELL_NET_ROLE);
  printf("%s%s%s%s\r\n", indent, "-N,", indent, SHELL_PKT_NUM);
#if defined(LINUX)
  printf("%s%s%s%s\r\n", indent, "-o,", indent, SHELL_LOG_OUTPUT);
#endif
  printf("%s%s%s%s\r\n", indent, "-p,", indent, SHELL_PASSWORD);
  printf("%s%s%s%s\r\n", indent, "-P,", indent, SHELL_PORT);
  printf("%s%s%s%s\r\n", indent, "-r,", indent, SHELL_RAW_TIME);
  printf("%s%s%s%s\r\n", indent, "-s,", indent, SHELL_CMD);
  printf("%s%s%s%s\r\n", indent, "-t,", indent, SHELL_STARTUP);
  printf("%s%s%s%s\r\n", indent, "-u,", indent, SHELL_USER);
  printf("%s%s%s%s\r\n", indent, "-E,", indent, OLD_DSN_DESC);
  printf("%s%s%s%s\r\n", indent, "-T,", indent, SHELL_TIMEOUT);
  printf("%s%s%s%s\r\n", indent, "-w,", indent, SHELL_WIDTH);
  printf("%s%s%s%s\r\n", indent, "-V,", indent, SHELL_VERSION);
  printf("%s%s%s%s\r\n", indent, "-X,", indent, DSN_DESC);
  printf("%s%s%s%s\r\n", indent, "-Z,", indent, DRIVER_DESC);

#ifdef CUS_EMAIL
  printf("\r\n\r\nReport bugs to %s.\r\n", CUS_EMAIL);
#else
  printf("\r\n\r\nReport bugs to %s.\r\n", "support@taosdata.com");
#endif
}

#if defined(LINUX) && !defined(TD_ASTRA)
#include <argp.h>
#ifdef _ALPINE
#include <termios.h>
#else
#include <termio.h>
#endif

const char *argp_program_version = td_version;
#ifdef CUS_EMAIL
const char *argp_program_bug_address = CUS_EMAIL;
#else
const char *argp_program_bug_address = "support@taosdata.com";
#endif

static struct argp_option shellOptions[] = {
    {"host", 'h', "HOST", 0, SHELL_HOST},
    {"port", 'P', "PORT", 0, SHELL_PORT},
    {"user", 'u', "USER", 0, SHELL_USER},
    {0, 'p', 0, 0, SHELL_PASSWORD},
    {"auth", 'a', "AUTH", 0, SHELL_AUTH},
    {"generate-auth", 'A', 0, 0, SHELL_GEN_AUTH},
    {"config-dir", 'c', "DIR", 0, SHELL_CFG_DIR},
    {"dump-config", 'C', 0, 0, SHELL_DMP_CFG},
    {"commands", 's', "COMMANDS", 0, SHELL_CMD},
    {"raw-time", 'r', 0, 0, SHELL_RAW_TIME},
    {"file", 'f', "FILE", 0, SHELL_FILE},
    {"database", 'd', "DATABASE", 0, SHELL_DB},
    {"check", 'k', 0, 0, SHELL_CHECK},
    {"startup", 't', 0, 0, SHELL_STARTUP},
    {"display-width", 'w', "WIDTH", 0, SHELL_WIDTH},
    {"netrole", 'n', "NETROLE", 0, SHELL_NET_ROLE},
    {"pktlen", 'l', "PKTLEN", 0, SHELL_PKT_LEN},
    {"cloud-dsn", 'E', "DSN", 0, OLD_DSN_DESC},
    {"timeout", 'T', "SECONDS", 0, SHELL_TIMEOUT},
    {"pktnum", 'N', "PKTNUM", 0, SHELL_PKT_NUM},
    {"bimode", 'B', 0, 0, SHELL_BI_MODE},
    {"log-output", 'o', "OUTPUT", 0, SHELL_LOG_OUTPUT},
    {"dsn", 'X', "DSN", 0, DSN_DESC},
    {DRIVER_OPT, 'Z', "DRIVER", 0, DRIVER_DESC},
    {0},
};

static error_t shellParseOpt(int32_t key, char *arg, struct argp_state *state) { return shellParseSingleOpt(key, arg); }

static struct argp shellArgp = {shellOptions, shellParseOpt, "", ""};

static int32_t shellParseArgsUseArgp(int argc, char *argv[]) {
  argp_program_version = shell.info.programVersion;
  error_t err = argp_parse(&shellArgp, argc, argv, 0, 0, &shell.args);
  return (err != 0);
}

#endif

#ifndef ARGP_ERR_UNKNOWN
#define ARGP_ERR_UNKNOWN E2BIG
#endif

static int32_t shellParseSingleOpt(int32_t key, char *arg) {
  SShellArgs *pArgs = &shell.args;

  switch (key) {
    case 'h':
      pArgs->host = arg;
      break;
    case 'P':
      pArgs->port = atoi(arg);
      if (pArgs->port == 0) {
        pArgs->port = -1;
      } else {
        pArgs->port_inputted = true;
      }
      break;
    case 'u':
      pArgs->user = arg;
      break;
    case 'p':
      break;
    case 'a':
      pArgs->auth = arg;
      break;
    case 'A':
      pArgs->is_gen_auth = true;
      break;
    case 'B':
      pArgs->is_bi_mode = true;
      break;
    case 'c':
      pArgs->cfgdir = arg;
      break;
    case 'C':
      pArgs->is_dump_config = true;
      break;
    case 's':
      pArgs->commands = arg;
      break;
    case 'r':
      pArgs->is_raw_time = true;
      break;
    case 'f':
      tstrncpy(pArgs->file, arg, sizeof(pArgs->file));
      break;
    case 'd':
      pArgs->database = arg;
      break;
    case 'k':
      pArgs->is_check = true;
      break;
    case 't':
      pArgs->is_startup = true;
      break;
    case 'w':
      pArgs->displayWidth = atoi(arg);
      break;
    case 'n':
      pArgs->netrole = arg;
      break;
    case 'l':
      pArgs->pktLen = atoi(arg);
      break;
    case 'N':
      pArgs->pktNum = atoi(arg);
      break;
#if defined(LINUX)
    case 'o':
      if (strlen(arg) >= PATH_MAX) {
        printf("failed to set log output since length overflow, max length is %d\r\n", PATH_MAX);
        return TSDB_CODE_INVALID_CFG;
      }
      tsLogOutput = taosMemoryMalloc(PATH_MAX);
      if (!tsLogOutput) {
        printf("failed to set log output: '%s' since %s\r\n", arg, tstrerror(terrno));
        return terrno;
      }
      if (taosExpandDir(arg, tsLogOutput, PATH_MAX) != 0) {
        printf("failed to expand log output: '%s' since %s\r\n", arg, tstrerror(terrno));
        return terrno;
      }
      break;
#endif
    case 'E':
    case 'X':
      pArgs->dsn = arg;
      break;
    case 'T':
      pArgs->timeout = atoi(arg);
      break;
    case 'Z':
      pArgs->connMode = getConnMode(arg);
      break;
    case 'V':
      pArgs->is_version = true;
      break;
    case '?':
      pArgs->is_help = true;
      break;
    case 1:
      pArgs->abort = 1;
      break;
    default:
      return ARGP_ERR_UNKNOWN;
  }
  return 0;
}
#if defined(_TD_WINDOWS_64) || defined(_TD_WINDOWS_32) || defined(_TD_DARWIN_64) || defined(TD_ASTRA)

int32_t shellParseArgsWithoutArgp(int argc, char *argv[]) {
  SShellArgs *pArgs = &shell.args;
  int32_t     ret = 0;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "--usage") == 0
            || strcmp(argv[i], "-?") == 0 || strcmp(argv[i], "/?") == 0) {
      return shellParseSingleOpt('?', NULL);
    }

    char   *key = argv[i];
    int32_t keyLen = strlen(key);
    if (keyLen != 2) {
      fprintf(stderr, "invalid option %s\r\n", key);
      return -1;
    }
    if (key[0] != '-') {
      fprintf(stderr, "invalid option %s\r\n", key);
      return -1;
    }

    if (key[1] == 'h' || key[1] == 'P' || key[1] == 'u' || key[1] == 'a' || key[1] == 'c' || key[1] == 's' ||
        key[1] == 'f' || key[1] == 'd' || key[1] == 'w' || key[1] == 'n' || key[1] == 'l' || key[1] == 'N' ||
        key[1] == 'E' || key[1] == 'T' || key[1] == 'X' || key[1] == 'Z') {
      if (i + 1 >= argc) {
        fprintf(stderr, "option %s requires an argument\r\n", key);
        return -1;
      }
      char *val = argv[i + 1];
      if (val[0] == '-') {
        fprintf(stderr, "option %s requires an argument\r\n", key);
        return -1;
      }
      ret = shellParseSingleOpt(key[1], val);
      i++;
    } else if (key[1] == 'p' || key[1] == 'A' || key[1] == 'C' || key[1] == 'r' || key[1] == 'k' || key[1] == 't' ||
               key[1] == 'V' || key[1] == '?' || key[1] == 1 || key[1] == 'R'|| key[1] == 'B') {
      ret = shellParseSingleOpt(key[1], NULL);
    } else {
      fprintf(stderr, "invalid option %s\r\n", key);
      return -1;
    }

    if (ret != 0) {
      return ret;
    }
  }

  return 0;
}
#endif

static void shellInitArgs(int argc, char *argv[]) {
  for (int i = 1; i < argc; i++) {
    if (strncmp(argv[i], "-p", 2) == 0) {
      // printf(shell.info.clientVersion, taos_get_client_info());
      if (strlen(argv[i]) == 2) {
        printf("Enter password: ");
        taosSetConsoleEcho(false);
        if (scanf("%255s", shell.args.password) > 1) {
          fprintf(stderr, "password reading error\n");
        }
        taosSetConsoleEcho(true);
        if (EOF == getchar()) {
          fprintf(stderr, "getchar() return EOF\r\n");
        }
      } else {
        tstrncpy(shell.args.password, (char *)(argv[i] + 2), sizeof(shell.args.password));
        strcpy(argv[i], "-p");
      }
      printf("\r\n");
    }
  }
  if (strlen(shell.args.password) == 0) {
    tstrncpy(shell.args.password, TSDB_DEFAULT_PASS, sizeof(shell.args.password));
  }

  SShellArgs *pArgs = &shell.args;
  pArgs->user = TSDB_DEFAULT_USER;
  pArgs->pktLen = SHELL_DEF_PKG_LEN;
  pArgs->pktNum = SHELL_DEF_PKG_NUM;
  pArgs->displayWidth = SHELL_DEFAULT_MAX_BINARY_DISPLAY_WIDTH;
  pArgs->timeout = SHELL_WS_TIMEOUT;

  shell.exit = false;
}

static int32_t shellCheckArgs() {
  SShellArgs *pArgs = &shell.args;
  if (pArgs->host != NULL && (strlen(pArgs->host) <= 0 || strlen(pArgs->host) > TSDB_FQDN_LEN)) {
    printf("Invalid host:%s\r\n", pArgs->host);
    return -1;
  }

  if (pArgs->user != NULL && (strlen(pArgs->user) <= 0 || strlen(pArgs->user) > TSDB_USER_LEN)) {
    printf("Invalid user:%s\r\n", pArgs->user);
    return -1;
  }

  if (pArgs->auth != NULL && (strlen(pArgs->auth) <= 0 || strlen(pArgs->auth) > TSDB_PASSWORD_LEN)) {
    printf("Invalid auth:%s\r\n", pArgs->auth);
    return -1;
  }

  if (pArgs->database != NULL && (strlen(pArgs->database) <= 0 || strlen(pArgs->database) > TSDB_DB_NAME_LEN)) {
    printf("Invalid database:%s\r\n", pArgs->database);
    return -1;
  }

  if (pArgs->file[0] != 0) {
    char fullname[PATH_MAX] = {0};
    if (taosExpandDir(pArgs->file, fullname, PATH_MAX) == 0) {
      tstrncpy(pArgs->file, fullname, PATH_MAX);
    }
  }

  if (pArgs->cfgdir != NULL) {
    if (strlen(pArgs->cfgdir) <= 0 || strlen(pArgs->cfgdir) >= PATH_MAX) {
      printf("Invalid cfgdir:%s\r\n", pArgs->cfgdir);
      return -1;
    } else {
      if (taosExpandDir(pArgs->cfgdir, configDirShell, PATH_MAX) != 0) {
        tstrncpy(configDirShell, pArgs->cfgdir, PATH_MAX);
      }
      // check cfg dir exist
      /*
      if(!taosIsDir(configDirShell)) {
        printf("folder not exist. cfgdir:%s  expand:%s\r\n", pArgs->cfgdir, configDirShell);
        configDirShell[0] = 0;
        return -1;          
      }*/  
    }
  }

  if (pArgs->commands != NULL && (strlen(pArgs->commands) <= 0)) {
    printf("Invalid commands:%s\r\n", pArgs->commands);
    return -1;
  }

  if (pArgs->netrole != NULL && !(strcmp(pArgs->netrole, "client") == 0 || strcmp(pArgs->netrole, "server") == 0)) {
    printf("Invalid netrole:%s\r\n", pArgs->netrole);
    return -1;
  }

  if (/*pArgs->password != NULL && */ (strlen(pArgs->password) <= 0)) {
    printf("Invalid password\r\n");
    return -1;
  }

  if (pArgs->port < 0 || pArgs->port > 65535) {
    printf("Invalid port\r\n");
    return -1;
  }

  if (pArgs->pktLen < SHELL_MIN_PKG_LEN || pArgs->pktLen > SHELL_MAX_PKG_LEN) {
    printf("Invalid pktLen:%d, range:[%d, %d]\r\n", pArgs->pktLen, SHELL_MIN_PKG_LEN, SHELL_MAX_PKG_LEN);
    return -1;
  }

  if (pArgs->pktNum < SHELL_MIN_PKG_NUM || pArgs->pktNum > SHELL_MAX_PKG_NUM) {
    printf("Invalid pktNum:%d, range:[%d, %d]\r\n", pArgs->pktNum, SHELL_MIN_PKG_NUM, SHELL_MAX_PKG_NUM);
    return -1;
  }

  if (pArgs->displayWidth <= 0 || pArgs->displayWidth > 10 * 1024) {
    printf("Invalid displayWidth:%d, range:[1, 10 * 1024]\r\n", pArgs->displayWidth);
    return -1;
  }

  return 0;
}

int32_t shellParseArgs(int32_t argc, char *argv[]) {
  shellInitArgs(argc, argv);
  shell.info.clientVersion =
      "Welcome to the %s Command Line Interface, %s Client Version:%s \r\n"
      "Copyright (c) 2025 by %s, all rights reserved.\r\n\r\n";
#ifdef CUS_NAME
  strcpy(shell.info.cusName, CUS_NAME);
#else
  strcpy(shell.info.cusName, "TDengine");
#endif
  char promptContinueFormat[32] = {0};
#ifdef CUS_PROMPT
  sprintf(shell.info.promptHeader, "%s> ", CUS_PROMPT);
  sprintf(promptContinueFormat, "%%%zus> ", strlen(CUS_PROMPT));
#else
  sprintf(shell.info.promptHeader, "taos> ");
  sprintf(promptContinueFormat, "%%%zus> ", strlen("taos"));
#endif
  sprintf(shell.info.promptContinue, promptContinueFormat, " ");
  shell.info.promptSize = strlen(shell.info.promptHeader);
#ifdef TD_ENTERPRISE
  snprintf(shell.info.programVersion, sizeof(shell.info.programVersion),
           "%s\n%s version: %s compatible_version: %s\ngit: %s\ngitOfInternal: %s\nbuild: %s", TD_PRODUCT_NAME,
           CUS_PROMPT, td_version, td_compatible_version, td_gitinfo, td_gitinfoOfInternal, td_buildinfo);
#else
  snprintf(shell.info.programVersion, sizeof(shell.info.programVersion),
           "%s\n%s version: %s compatible_version: %s\ngit: %s\nbuild: %s", TD_PRODUCT_NAME, CUS_PROMPT, td_version,
           td_compatible_version, td_gitinfo, td_buildinfo);
#endif

#if defined(_TD_WINDOWS_64) || defined(_TD_WINDOWS_32)
  shell.info.osname = "Windows";
  snprintf(shell.history.file, TSDB_FILENAME_LEN, "C:/TDengine/%s", SHELL_HISTORY_FILE);
  if (shellParseArgsWithoutArgp(argc, argv) != 0) return -1;
#elif defined(_TD_DARWIN_64)
  shell.info.osname = "Darwin";
  snprintf(shell.history.file, TSDB_FILENAME_LEN, "%s/%s", getpwuid(getuid())->pw_dir, SHELL_HISTORY_FILE);
  if (shellParseArgsWithoutArgp(argc, argv) != 0) return -1;
#elif defined(TD_ASTRA)
  shell.info.osname = "Astra";
  snprintf(shell.history.file, TSDB_FILENAME_LEN, "C:%sTDengine%s%s", TD_DIRSEP, TD_DIRSEP,
           SHELL_HISTORY_FILE);  // TD_ASTRA_TODO getenv("HOME")
  if (shellParseArgsWithoutArgp(argc, argv) != 0) return -1;
#else
  shell.info.osname = "Linux";
  snprintf(shell.history.file, TSDB_FILENAME_LEN, "%s/%s", getenv("HOME"), SHELL_HISTORY_FILE);
  if (shellParseArgsUseArgp(argc, argv) != 0) return -1;
  if (shell.args.abort) {
    return -1;
  }
#endif

  return shellCheckArgs();
}

int32_t getDsnEnv() {
  if (shell.args.connMode == CONN_MODE_NATIVE) {
    if (shell.args.dsn != NULL) {
      fprintf(stderr, DSN_NATIVE_CONFLICT);
      return -1;
    }
  } else {
    if (shell.args.dsn != NULL) {
      return 0;
    } else {
      // read cloud
      shell.args.dsn = getenv("TDENGINE_CLOUD_DSN");
      if (shell.args.dsn && strlen(shell.args.dsn) > 4) {
        fprintf(stderr, "Use the environment variable TDENGINE_CLOUD_DSN:%s as the input for the DSN option.\r\n",
                shell.args.dsn);
        return 0;
      }

      // read local
      shell.args.dsn = getenv("TDENGINE_DSN");
      if (shell.args.dsn && strlen(shell.args.dsn) > 4) {
        fprintf(stderr, "Use the environment variable TDENGINE_DSN:%s as the input for the DSN option.\r\n",
                shell.args.dsn);
        return 0;
      }
      shell.args.dsn = NULL;
    }
  }

  return 0;
}
