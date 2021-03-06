/*
 * yasul: Yet another Android SU library. 
 *
 * t0kt0ckus
 * (C) 2014,2015
 * 
 * License LGPLv2, GPLv3
 * 
 */
package org.openmarl.yasul;

import android.content.Context;
import android.util.Log;

/** Bootstraps the Yasul shared library, and acts as a sesion factory.
 *
 */
public class YslContext {


    private static YslContext _singleton = null;

    public static YslContext getInstance(Context appCtx, boolean shouldDebugYsl) {
        if (_singleton == null) {
            try {
                _singleton = new YslContext(appCtx, shouldDebugYsl);
            }
            catch (BootstrapError e) {
                _singleton = null;
            }
        }
        return _singleton;
    }

    public static YslContext getInstance() {
        return _singleton;
    }

    private Context mAppCtx;

    private YslContext(Context appContext, boolean shouldDebugYsl) throws BootstrapError {
        mAppCtx = appContext;
        String baseDir = appContext.getApplicationContext().getFilesDir().getAbsolutePath();
        Log.i(TAG,
                String.format("Bootstraping Ysl context in %s mode with base directory %s",
                        shouldDebugYsl ? "debug" : "quiet",
                        baseDir));

        if (Libyasul.bootstrap(baseDir, shouldDebugYsl) != 0) {
            Log.e(TAG, "Failed to bootstrap, see Logcat and/or yasul-<pid>.log for possible cause !");
            throw new BootstrapError();
        }
    }

    public String getLogpath() {
        return Libyasul.getLogpath();
    }

    public int findPidByCmdline(String cmdline) {
        return Libyasul.findPidByCmdline(cmdline);
    }

    public void openSession(YslObserver client, int ctlFlags, long timeout) {
        new YslAsyncFactory(mAppCtx, this, client, ctlFlags, timeout).execute();
    }

    private class BootstrapError extends Exception {
    }

    private static final String TAG = "YASUL";
}
