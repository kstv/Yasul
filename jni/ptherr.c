/*
 * yasul: Yet another Android SU library. 
 *
 * t0kt0ckus
 * (C) 2014,2015
 * 
 * License LGPLv2, GPLv3
 * 
 */
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "def.h"
#include "log.h"
#include "buf.h"
#include "session.h"

void *ysl_ptherr_fn(void *arg) {
    ysl_session_t *s = (ysl_session_t *) arg;
    if (s->flags & YSL_SF_VERB)
        ysl_log_printf2(s->pid, "started stderr handler thread.\n");

    s->errefile = fopen(s->errepath, "w");
    if (! s->errefile) {
        pthread_exit(NULL);
    }

    if (s->flags & YSL_SF_VERB)
        ysl_log_printf2(s->pid, "stderr echo file: %s\n", s->errepath);

    int fderr = fileno(s->errefile);

    char z;
    while (read(s->ipcerr, &z, 1) == 1) {
        // echo byte if requested
        if (s->flags & YSL_SF_EERR)
            write(fderr, &z, 1);
    } // end read()
    close(fderr);
    s->errefile = NULL;

    if (s->flags & YSL_SF_VERB)
        ysl_log_printf2(s->pid, "stopped stderr handler thread.\n");
    pthread_exit(NULL);
} 

