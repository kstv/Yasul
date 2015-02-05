/*
 * yasul: Yet another Android SU library. 
 *
 * t0kt0ckus
 * (C) 2014,2015
 * 
 * License LGPLv2, GPLv3
 * 
 */
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "def.h"
#include "log.h" 
#include "yasul.h"

#ifdef YSL_BUILD_DEBUG
static char *YSL_SU_CANDIDATES[] = {"/bin/bash", NULL};
static char * const YSL_SU_ENV[] = { NULL };
#else
static const char *YSL_SU_CANDIDATES[] = {"/sbin/su",
    "/system/sbin/su",
    "/system/bin/su",
    "/system/xbin/su",
    NULL
};
static char *const YSL_SU_ENV[] = {
            "PATH=/sbin:/vendor/bin:/system/sbin:/system/bin:/system/xbin",
            NULL
};
#endif

static char *ysl_find_suexec();


// yasul_open_session():
//
ysl_session_t *yasul_open_session(const char *logdir, int flags) {
    ysl_log_printf("+ yasul_open_session (0x%x):\n");

    // lookup su
    char *suexec = ysl_find_suexec();
    if (! suexec) {
        ysl_log_printf("Failed to locate suitable su binary !\n");
        return NULL;
    }
    else
        ysl_log_debugf("Resolved SU binary: %s\n", suexec);

    // prepare IPC sockets
    int svin[2], svout[2], sverr[2];
    if ( (socketpair(PF_LOCAL, SOCK_STREAM, 0, svin))
            || (socketpair(PF_LOCAL, SOCK_STREAM, 0, svout))
            ||  (socketpair(PF_LOCAL, SOCK_STREAM, 0, sverr)) ) {
        ysl_log_printf("Failed to setup IPC endpoints !\n");
        ysl_log_errno(errno);
        return NULL;
    }

    // creates shell process
    pid_t pid = fork();
    if (pid == -1) {
        ysl_log_printf("Failed to create shell process !\n");
        ysl_log_errno(errno);
        return NULL;
    }
    else if (pid == 0) {
        // shell process closes unused descriptors,
        close(svin[0]);
        close(svout[0]);
        close(sverr[0]);
        // duplicates the local socket descriptors to its owns,
        dup2(svin[1], 0);
        dup2(svout[1], 1);
        dup2(sverr[1], 2);
        // and attempts to start requested shell
        char *const params[2] = {suexec, NULL};
        execve(suexec, params, YSL_SU_ENV);
        exit(1);
    }

    // parent process, closes unused descriptors
    close(svin[1]);
    close(sverr[1]);
    close(svout[1]);
    // creates session
    ysl_session_t *s = ysl_session_create(logdir, pid, flags);
    if (! s) {
        ysl_log_printf("Failed to create session !\n");
        return NULL;
    }
    // setut session IPC
    s->ipcin = svin[0];
    s->ipcout = svout[0];
    s->ipcerr = sverr[0];
    // should be ready to start handlers
    if (pthread_create(s->pthout, 0, ysl_pthout_fn, s)) {        
        ysl_log_printf("Failed to create stdout handler thread !\n");
        return NULL;
    }
    if (pthread_create(s->ptherr, 0, ysl_ptherr_fn, s)) {
        ysl_log_printf("Failed to create stderr handler thread !\n");
        return NULL;
    }

    return s;
}

char *ysl_find_suexec() {
    struct stat sustat;
    char* suexec = NULL;
    int i = 0;
    while (YSL_SU_CANDIDATES[i]) {
        if (stat(YSL_SU_CANDIDATES[i], &sustat))
            i++;
        else {
            suexec = (char *) YSL_SU_CANDIDATES[i];
            break;
        }
    }
    return suexec;
}

