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

import android.os.AsyncTask;
import android.util.Log;


public class YslAsyncFactory extends AsyncTask<Void,Void,YslSession> {

    private final YslContext mYslContext;
    private final YslObserver mClient;
    private final int mCtlFlags;
    private final long mTimeoutMillis;

    private static final long WAIT_MILLIS = 1000;

    public YslAsyncFactory(YslContext yslContext, YslObserver client, int flags, long timeout) {
        mYslContext =yslContext;
        mClient = client;
        mCtlFlags = flags;
        mTimeoutMillis = timeout;
    }

    @Override
    protected YslSession doInBackground(Void... params) {
        YslPort port = Libyasul.open(mCtlFlags);
        if (port != null) {
            Log.d(TAG, String.format(
                    "opening session, waiting for shell process confirmation (PID: %d) ...",
                    port.pid));

            long t0 = System.currentTimeMillis();
            long t1 = System.currentTimeMillis();
            while ( ((t1 - t0) < mTimeoutMillis) && (Libyasul.stat(port.ID) != 0) ) {
                Log.d(TAG,
                        String.format("shell process IPC not ready, waiting %d ms...",
                                WAIT_MILLIS));
                try {
                    Thread.sleep(WAIT_MILLIS, 0);
                    t1 = System.currentTimeMillis();
                } catch (InterruptedException e) {
                    Log.d(TAG, e.toString());
                }
            }
            if (Libyasul.stat(port.ID) == 0) {
                YslSession session = new YslSession(port.pid, port.ID, port.stdout, port.stderr);
                Log.i(TAG,
                        String.format("Shell session: %s", session.toString()));
                return session;
            }
            else
                Log.w(TAG,"session was not reachable before timeout !");
        }
        Log.e(TAG, "Failed to create session, see Logcat and/or yasul-<pid>.log for possible cause !");
        return null;
    }

    @Override
    protected void onPostExecute(YslSession yslSession) {
        super.onPostExecute(yslSession);
        mClient.onSessionFactoryEvent(yslSession);
    }

    private static final String TAG = "YASUL";
}
