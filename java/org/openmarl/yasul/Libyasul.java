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

/**
 * JNI bridge to <code>libyasul.so</code>.
 *
 */
public class Libyasul {

    static {
        System.loadLibrary(Ysl.LIBRARY);
    }

    /**
     *
     * @param logDir
     * @param debug
     */
    public static native int bootstrap(String logDir, boolean debug);

    /**
     *
     * @param flags
     * @return
     */
    public static native YslPort open(int flags);

    /**
     *
     * @param sessionId
     * @param flag
     * @param isSet
     * @return
     */
    public static native int cfset(long sessionId, int flag, boolean isSet);

    /**
     *
     * @param sessionId
     * @param flag
     * @return
     */
    public static native boolean cfget(long sessionId, int flag);

    /**
     *
     * @param sessionId
     * @return
     */
    public static native int stat(long sessionId);

    /**
     *
     * @param sessionId
     * @param cmdStr
     * @return
     */
    public static native YslParcel exec(long sessionId, String cmdStr);

    /**
     *
     * @param sessionId
     * @return
     */
    public static native String lasttty(long sessionId);

    /**
     *
     * @param sessionId
     */
    public static native void close(long sessionId);

    /**
     *
     * @param cmdline
     * @return
     */
    public static native int findPidByCmdline(String cmdline);

}
