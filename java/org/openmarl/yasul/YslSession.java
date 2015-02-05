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

import android.util.Log;

public class YslSession {

    private boolean mInvalidated; // by shell process broken pipe or normal close()
    private final long mID;
    private final int mPid;
    private String mStdout;
    private String mStderr;

    private YslShell mShell;

    public YslSession(int pid, long id, String stdout, String stderr) {
        mPid = pid;
        mID = id;
        mStdout = stdout;
        mStderr = stderr;
        mInvalidated = false;
    }

    public long ID() {
        return mID;
    }

    public int getPid() {
        return mPid;
    }

    public String getStdout() {
        return mStdout;
    }

    public String getStderr() {
        return mStderr;
    }

    public boolean isAvailable() {
        return ((! mInvalidated) && (Libyasul.stat(mID) == 0));
    }

    public int setCtlFlag(int flag, boolean isSet) throws YslEpipeExcetion {
        if (mInvalidated)
            throw new YslEpipeExcetion();
        return 0;
    }

    public int getCtlFlag(int flag) throws YslEpipeExcetion {
        if (mInvalidated)
            throw new YslEpipeExcetion();
        return 0;
    }

    public String getLastTty() throws YslEpipeExcetion {
        if (mInvalidated)
            throw new YslEpipeExcetion();
        return null;
    }

    public YslParcel exec(String cmdStr)
            throws YslEpipeExcetion {
        YslParcel retval = Libyasul.exec(mID, cmdStr);
        if (retval == null)
            invalidateOnEpipe();
        return retval;
    }

    public void close() {
        mInvalidated = true;
        Libyasul.close(mID);
    }

    public YslShell getShell() {
        if (mShell == null) {
            mShell = new YslShell(this);
        }
        return mShell;
    }

    @Override
    public String toString() {
        return String.format("[PID: %d , address: 0x%x, stdout: <%s>, stderr: <%s>]: %s",
                mPid, mID, mStdout, mStderr, mInvalidated ? "INVALIDATED" : "valid");
    }

    private void invalidateOnEpipe() throws YslEpipeExcetion {
        Log.w(TAG, String.format("EPIPE: %s", this.toString()));
        mInvalidated = true;
        throw new YslEpipeExcetion();
    }

    private static final String TAG = "YASUL";
}
