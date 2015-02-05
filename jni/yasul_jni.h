/*
 * yasul: Yet another Android SU library. 
 *
 * t0kt0ckus
 * (C) 2014,2015
 * 
 * License LGPLv2, GPLv3
 * 
 */
#ifndef _YASUL_JNI_H
#define _YASUL_JNI_H

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jint JNICALL
    Java_org_openmarl_yasul_Libyasul_bootstrap(JNIEnv *env, 
            jobject jInstance,
            jstring jLogdir,
            jboolean jDebug);

JNIEXPORT jobject JNICALL
    Java_org_openmarl_yasul_Libyasul_open(JNIEnv *env, 
            jobject jInstance,
            jint jFlags);

JNIEXPORT jint JNICALL
    Java_org_openmarl_yasul_Libyasul_stat(JNIEnv *jEnv, 
            jobject jInstance,
            jlong jSessionId);
            
JNIEXPORT jint JNICALL
    Java_org_openmarl_yasul_Libyasul_cfset(JNIEnv *env, 
            jobject jInstance,
            jlong jSessionId,
            jint jFlag,
            jboolean jIsSet);

JNIEXPORT jboolean JNICALL
    Java_org_openmarl_yasul_Libyasul_cfget(JNIEnv *env, 
            jobject jInstance,
            jlong jSessionId,
            jint jFlag);

JNIEXPORT jobject JNICALL
    Java_org_openmarl_yasul_Libyasul_exec(JNIEnv *jEnv, 
            jobject jInstance,
            jlong jSessionId,
            jstring cmdstr);
   
JNIEXPORT void JNICALL
    Java_org_openmarl_yasul_Libyasul_close(JNIEnv *jEnv, 
            jobject jInstance,
            jlong jSessionId);

JNIEXPORT jint JNICALL
    Java_org_openmarl_yasul_Libyasul_findPidByCmdline(JNIEnv *env, 
            jobject jInstance,
            jstring jCmdline);

            
// JNI_OnLoad
//
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved);

// JNI_OnUnload
//
JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *vm, void *reserved);

#ifdef __cplusplus
}
#endif

#endif
