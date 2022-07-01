#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <linux/stat.h>
#include <fcntl.h>
#include <libgen.h>
#include <regex.h>
#include <time.h>
#include <json-c/json.h>
#include <nng/nng.h>
#include <nng/protocol/pubsub0/pub.h>
#include <nng/protocol/pubsub0/sub.h>
#include <nng/protocol/reqrep0/rep.h>
#include <nng/protocol/reqrep0/req.h>
#include "iniparser.h"
#include "dictionary.h"


#define SERVER "server"
#define CLIENT  "client"
#define WEB      "web"
