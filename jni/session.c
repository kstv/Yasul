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
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "def.h"
#include "log.h"
#include "session.h"


// ysl_session_create():
//
ysl_session_t *ysl_session_create(const char *logdir, int pid, int flags) {
    ysl_session_t *s = malloc(sizeof(ysl_session_t));
    if (! s)
        return NULL;

    s->einval = -1; // will not be valid till validated by stdout handler
    s->pid = pid;
    s->flags = flags;
    s->ipcin = s->ipcout = s->ipcerr = -1;
    s->ltty = NULL;

    s->outepath = malloc(strlen(YSL_EOUT_FMT) + (strlen(logdir)-2) + 1 +1);
    s->errepath = malloc(strlen(YSL_EERR_FMT) + (strlen(logdir)-2) + 1 +1);
    if ( (! s->outepath) || (! s->errepath) ) {
        ysl_session_delete(s);
        return NULL;
    }
    sprintf(s->outepath, YSL_EOUT_FMT, logdir, pid);
    s->outefile = NULL;
    sprintf(s->errepath, YSL_EERR_FMT, logdir, pid);
    s->errefile = NULL;

    s->ism = malloc(sizeof(pthread_mutex_t));
    s->lecc = malloc(sizeof(pthread_cond_t));
    if ( (! s->ism) || (! s->lecc) ) {
        ysl_session_delete(s);
        return NULL;
    }
    pthread_mutex_init(s->ism, NULL); // default attrs
    pthread_cond_init(s->lecc, NULL); // default attrs

    s->pthout = malloc(sizeof(pthread_t));
    s->ptherr = malloc(sizeof(pthread_t));
    if ( (! s->pthout) || (! s->ptherr) ) {
        ysl_session_delete(s);
        return NULL;
    }

    ysl_log_debugf("created new session [%d] at address: 0x%x\n",
            s->pid, s);
    return s;
}

// ysl_session_delete():
//
void ysl_session_delete(ysl_session_t *s) {
    if (s) {
        if (s->lecc) {
            pthread_cond_destroy(s->lecc);
            free(s->lecc);
        }
        if (s->ism) {
            pthread_mutex_destroy(s->ism);
            free(s->ism);
        }
        free(s->outepath);
        free(s->errepath);
        free(s->outefile);
        free(s->errefile);
        free(s->ltty);
        free(s->pthout);
        free(s->ptherr);
        free(s);
    }
}

// ysl_session_cfset():
//
int ysl_session_cfset(ysl_session_t *s, int flag, unsigned char isfset) {
    if (s->flags & YSL_SF_VERB)
        ysl_log_printf2(s->pid, "ysl_session_cfset(Ox%x , %d)\n", flag,
                isfset);

    // clear flag bit
    s->flags = s->flags & ~flag;
    // set flag bit
    if (isfset)
        s->flags |= flag;

    if (s->flags & YSL_SF_VERB)
        ysl_log_printf2(s->pid, "new flags: Ox%x\n", s->flags);

    return s->flags;
}

// ysl_session_cfget():
//
unsigned char ysl_session_cfget(ysl_session_t *s, int flag) {
    return s->flags & flag;
}

// ysl_session_stat():
//
int ysl_session_stat(ysl_session_t *session) {
    // session should not have been invalidated
    if (session->einval)
        return EPIPE;
    // we also should be able to send() 0 byte to shell process stdin
    if (send(session->ipcin, NULL, 0, MSG_NOSIGNAL) != 0)
        return EPIPE;
        
    return 0;
}

// ysl_session_exec():
//
int ysl_session_exec(ysl_session_t *s, const char *cmdstr, int *ecode,
        char** ltty) {
    if (s->einval)
        return EPIPE;
    if (s->flags & YSL_SF_VERB)
        ysl_log_printf2(s->pid, "ysl_session_exec(\"%s\"):\n", cmdstr);
    
    int err = 0;

    // acquires lock
    pthread_mutex_lock(s->ism);
   
    // send() cmd string
    int cmdlen = strlen(cmdstr);
    int eoclen = strlen(YSL_LEC_CMD);
    if ( (send(s->ipcin, cmdstr, cmdlen, MSG_NOSIGNAL) == cmdlen)
            && (send(s->ipcin, "\n", 1, MSG_NOSIGNAL) == 1)
            && (send(s->ipcin,
                    YSL_LEC_CMD,
                    eoclen,
                    MSG_NOSIGNAL) == eoclen) ) {
        // waits for completion
        pthread_cond_wait(s->lecc, s->ism); // releases lock, then relock
                                            // when signaled
        // sets exit code
        (*ecode) = s->lec;
        // sets last TTY line
        (*ltty) = malloc(strlen(s->ltty)+1);
        if (*ltty)
            strcpy(*ltty, s->ltty);
        else
            err = ENOMEM;
    }
    else 
        err = EPIPE;

    // if verbose
    if (s->flags & YSL_SF_VERB) {
        if (err) {
            ysl_log_printf2(s->pid, "Failed to process command !\n");
            ysl_log_errno(errno);
        }
        else
            ysl_log_printf2(s->pid, "exit code: %d , LTTY: \"%s\"\n",
                    (*ecode), (*ltty));
    }
    // unlocks before exit
    pthread_mutex_unlock(s->ism);
    return err;
}

// ysl_session_exit():
//
void ysl_session_exit(ysl_session_t *s) {
    if (s->flags & YSL_SF_VERB)
        ysl_log_printf2(s->pid, "ysl_session_close():\n");

    if (! s->einval) {
        // terminates shell process
        pthread_mutex_lock(s->ism);    
        send(s->ipcin, YSL_CLOSE_CMD, strlen(YSL_CLOSE_CMD), MSG_NOSIGNAL);
        // lock released by pthout thread
    
        // wait handler threads termination
        void *retval;
        pthread_join(*s->pthout, &retval);
        pthread_join(*s->ptherr, &retval);
    }
    // session invalidation
    ysl_session_delete(s);
}

