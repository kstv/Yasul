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
#include <unistd.h>

#include "def.h"
#include "log.h"
#include "buf.h"
#include "session.h"

static int ysl_wait_shell_ack(ysl_session_t *s);

void *ysl_pthout_fn(void *arg) {
    ysl_session_t *s = (ysl_session_t *) arg;
    if (s->flags & YSL_SF_VERB)
        ysl_log_printf2(s->pid, "started stdout handler thread.\n");

    s->outefile = fopen(s->outepath, "w");
    if (! s->outefile) {
        pthread_exit(NULL);
    }

    if (s->flags & YSL_SF_VERB)
        ysl_log_printf2(s->pid, "stdout echo file: %s\n", s->outepath);

    int fdout = fileno(s->outefile);
    
    // waiting for shell process ACK
    if (ysl_wait_shell_ack(s)) {
        ysl_log_printf2(s->pid,
                "Shell process failed to confirm session, aborting !\n");
        return NULL;
    }
    else {
        s->einval = 0;
        ysl_log_debugf2(s->pid, "Shell process's confirms session\n");
    }

    // starts to parse shell process STDOUT as commands output
    int hsz = 0;
    char *hltty = NULL; // human-readable last tty
    ysl_buf_t *b = ysl_buf_create();
    char z;
    while (read(s->ipcout, &z, 1) == 1) {
        // echo byte if requested
        if (s->flags & YSL_SF_EOUT)
            write(fdout, &z, 1);

        if (z != '\n')
            ysl_buf_add(b, &z, 1);        
        else {
            // EOL
            if (ysl_buf_strstr(b, YSL_LEC_TAG)) {
                // set session's ltty
                if (! (s->flags & YSL_SF_TAIL)) {
                    free(s->ltty);
                    s->ltty = malloc(hsz);
                    if (s->ltty) 
                        memcpy(s->ltty, hltty, hsz);
                } 
                // command completed, set LEC
                sscanf(b->data, YSL_LEC_SSCAN, &s->lec);
                // wakeup main thread waiting for exit code
                pthread_cond_signal(s->lecc);
            }
            else {
                // terminates C string
                ysl_buf_addbyte(b, 0x00);
                // updates humain readable ltty
                free(hltty);                    
                hltty = b->data;
                hsz = b->len;
                b = ysl_buf_create();
                
                // when tail flag set, copy to session ltty
                // FIXME: should be protected be mutex
                if (s->flags & YSL_SF_TAIL) {
                    free(s->ltty);
                    s->ltty = malloc(b->len);
                    if (s->ltty) 
                        memcpy(s->ltty, b->data, b->len);
                }
            }
            
            ysl_buf_reset(b);
        } // end EOL

    } // end read()
    
    // invalidate session asap
    s->einval = 1;
    ysl_log_printf2(s->pid, "session invalidated\n"); 

    // then exits    
    close(fdout);
    s->outefile = NULL;
    ysl_buf_delete(b);

    if (! pthread_mutex_trylock(s->ism)) {
        if (s->flags & YSL_SF_VERB)
            ysl_log_printf2(s->pid, "shell process has died abnormaly !\n");
        // if we can acquire the lock, main thread may be waiting
        pthread_cond_signal(s->lecc);
    }
    pthread_mutex_unlock(s->ism);

    if (s->flags & YSL_SF_VERB)
        ysl_log_printf2(s->pid, "stopped stdout handler thread.\n");
    pthread_exit(NULL);
} 

int ysl_wait_shell_ack(ysl_session_t *s) {

    // sends the session confirmation message request
    int err = 0, ackw = 0;
    while ( (! err) 
            &&
            (ackw = send(s->ipcin, YSL_ACK_CMD, strlen(YSL_ACK_CMD), 
                         MSG_NOSIGNAL | MSG_DONTWAIT))
            != strlen(YSL_ACK_CMD) ) {

         if ( (errno == EAGAIN) || (errno == EWOULDBLOCK) ) {
             ysl_log_debugf(
                     "we assume process forked but no shell ready ...\n");
             sleep(1); // let's wait 1s
         }
         else {
             err = errno;
             ysl_log_debugf(
                     "we assume process forked and then exited.\n");
         }
    }
    if (err)
        return err;
    else
        ysl_log_debugf("waiting for shell process ACK ...\n");

    // consume ack to consume it ...
    char ack[strlen(YSL_ACK_TAG)];    
    if (read(s->ipcout, ack, strlen(YSL_ACK_TAG)) == strlen(YSL_ACK_TAG))
        return 0;
    else
        return -1;
}
