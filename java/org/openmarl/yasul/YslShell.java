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

public class YslShell {

    private final YslSession mYslSession;

    public YslShell(YslSession mYslSession) {
        this.mYslSession = mYslSession;
    }

    public String pwd() throws YslEpipeExcetion {
        YslParcel parcel = mYslSession.exec("pwd");
        return parcel.lastTty;
    }

    public boolean cd(String path) throws YslEpipeExcetion {
        YslParcel parcel = mYslSession.exec(String.format("cd %s", path));
        return (parcel.exitCode == 0);
    }

    public boolean mkdir(String path, int mode) throws YslEpipeExcetion {
        YslParcel parcel = mYslSession.exec(String.format( "mkdir -p %s", path));
        if (parcel.exitCode == 0)
            return chmod(mode, path);
        else
            return false;
    }

    public boolean cp(String srcPath, String destPath) throws YslEpipeExcetion {
        YslParcel parcel = mYslSession.exec(String.format( "cp %s %s", srcPath, destPath));
        return (parcel.exitCode == 0);
    }

    public boolean cpd(String srcPath, String destPath) throws YslEpipeExcetion {
        YslParcel parcel = mYslSession.exec(String.format( "cp -R %s %s", srcPath, destPath));
        return (parcel.exitCode == 0);
    }

    public boolean cpa(String asset, String srcPath, String destPath, int moide)
            throws YslEpipeExcetion {
        return false;
    }

    public boolean rm(String path, boolean force) throws YslEpipeExcetion {
        YslParcel parcel = mYslSession.exec(String.format( "rm %s %s", force ? "-f" : "", path));
        return (parcel.exitCode == 0);
    }

    public boolean rmd(String path, boolean force) throws YslEpipeExcetion {
        YslParcel parcel = mYslSession.exec(String.format( "rm -R %s %s", force ? "-f" : "", path));
        return (parcel.exitCode == 0);
    }

    public boolean mv(String srcPath, String destPath) throws YslEpipeExcetion {
        YslParcel parcel = mYslSession.exec(String.format( "mv %s %s", srcPath, destPath));
        return (parcel.exitCode == 0);
    }

    public String getenv(String variableName) throws YslEpipeExcetion {
        YslParcel parcel = mYslSession.exec(String.format( "echo \"$%s\"", variableName));
        return parcel.lastTty;
    }

    public void setenv(String variableName, String variableValue, boolean exportSubproc)
            throws YslEpipeExcetion {
        YslParcel parcel = mYslSession.exec(String.format( "%s %s=\"%s\"",
                exportSubproc ? "export" : "",
                variableName,
                variableValue));
    }

    public boolean chown(String user, String path) throws YslEpipeExcetion {
        YslParcel parcel = mYslSession.exec(String.format( "chown %s %s", user, path));
        return (parcel.exitCode == 0);
    }

    public boolean chgrp(int group, String path) throws YslEpipeExcetion {
        YslParcel parcel = mYslSession.exec(String.format( "chgrp %d %s", group, path));
        return (parcel.exitCode == 0);
    }

    public boolean chmod(int mode, String path) throws YslEpipeExcetion {
        YslParcel parcel = mYslSession.exec(String.format( "chmod %#04o %s", mode, path));
        return (parcel.exitCode == 0);
    }

    public boolean stat(String path) throws YslEpipeExcetion {
        YslParcel parcel = mYslSession.exec(String.format( "stat -c %%n %s", path));
        return (parcel.exitCode == 0);
    }

    public int stat_u(String path) throws YslEpipeExcetion {
        YslParcel parcel = mYslSession.exec(String.format( "stat -c %%u %s", path));
        int uid = -1;
        try {
            uid = Integer.valueOf(parcel.lastTty);
        }
        catch (NumberFormatException e) {
            Log.w(TAG, e.toString());
        }
        return uid;
    }

    public long stat_s(String path) throws YslEpipeExcetion {
        YslParcel parcel = mYslSession.exec(String.format( "stat -c %%s %s", path));
        int sz = -1;
        try {
            sz = Integer.valueOf(parcel.lastTty);
        }
        catch (NumberFormatException e) {
            Log.w(TAG, e.toString());
        }
        return sz;
    }

    public int stat_g(String path) throws YslEpipeExcetion {
        YslParcel parcel = mYslSession.exec(String.format( "stat -c %%g %s", path));
        int gid = -1;
        try {
            gid = Integer.valueOf(parcel.lastTty);
        }
        catch (NumberFormatException e) {
            Log.w(TAG, e.toString());
        }
        return gid;
    }

    public int stat_a(String path) throws YslEpipeExcetion {
        YslParcel parcel = mYslSession.exec(String.format( "stat -c %%a %s", path));
        int mode = -1;
        try {
            mode = Integer.parseInt(parcel.lastTty, 8);
        }
        catch (NumberFormatException e) {
            Log.w(TAG, e.toString());
        }
        return mode;
    }

    public boolean touch(String path, int mode) throws YslEpipeExcetion {
        YslParcel parcel = mYslSession.exec(String.format("touch %s", path));
        if (parcel.exitCode == 0)
            return chmod(mode, path);
        else
            return false;
    }

    public String id() throws YslEpipeExcetion {
        YslParcel parcel = mYslSession.exec("id");
        return parcel.lastTty;
    }

    public String uname() throws YslEpipeExcetion {
        YslParcel parcel = mYslSession.exec("uname -a");
        return parcel.lastTty;
    }

    public String uname_m() throws YslEpipeExcetion {
        YslParcel parcel = mYslSession.exec("uname -m");
        return parcel.lastTty;
    }

    public String uname_r() throws YslEpipeExcetion {
        YslParcel parcel = mYslSession.exec("uname -r");
        return parcel.lastTty;
    }

    public String hostname() throws YslEpipeExcetion {
        YslParcel parcel = mYslSession.exec("uname -n");
        return parcel.lastTty;
    }

    public String[] ps()
            throws YslEpipeExcetion {
        return null;
    }

    private static final String TAG = "YASUL";
}
