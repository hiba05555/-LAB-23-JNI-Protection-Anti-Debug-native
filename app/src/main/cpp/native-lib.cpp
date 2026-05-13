#include <jni.h>
#include <string>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <android/log.h>
#include <sys/ptrace.h>
#include <unistd.h>

#define LOG_TAG "HC_ANTIDEBUG"
#define HC_LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define HC_LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define HC_LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Contrôle 1 : ptrace
static bool hc_isBeingTraced() {
    long res = ptrace(PTRACE_TRACEME, 0, 0, 0);
    if (res == -1) {
        HC_LOGE("[HC] ptrace : processus surveille");
        return true;
    }
    HC_LOGI("[HC] ptrace : OK");
    return false;
}

// Contrôle 2 : /proc/self/maps
static bool hc_hasSuspiciousLibraries() {
    FILE* maps = fopen("/proc/self/maps", "r");
    if (!maps) {
        HC_LOGW("[HC] maps : impossible d'ouvrir");
        return false;
    }
    char line[512];
    while (fgets(line, sizeof(line), maps)) {
        if (strstr(line, "frida") ||
            strstr(line, "xposed") ||
            strstr(line, "libfrida") ||
            strstr(line, "gdbserver") ||
            strstr(line, "magisk")) {
            HC_LOGE("[HC] maps : signature suspecte -> %s", line);
            fclose(maps);
            return true;
        }
    }
    fclose(maps);
    HC_LOGI("[HC] maps : OK");
    return false;
}

// Contrôle 3 : TracerPid dans /proc/self/status
static bool hc_hasTracerPid() {
    FILE* status = fopen("/proc/self/status", "r");
    if (!status) {
        HC_LOGW("[HC] status : impossible d'ouvrir");
        return false;
    }
    char line[256];
    while (fgets(line, sizeof(line), status)) {
        if (strncmp(line, "TracerPid:", 10) == 0) {
            int pid = atoi(line + 10);
            fclose(status);
            if (pid != 0) {
                HC_LOGE("[HC] TracerPid detecte : %d", pid);
                return true;
            }
            HC_LOGI("[HC] TracerPid : OK (0)");
            return false;
        }
    }
    fclose(status);
    return false;
}

// Contrôle global : retourne un code d'état
extern "C"
JNIEXPORT jint JNICALL
Java_com_example_lab22_1jnidemo_HCSecurityManager_getSecurityStatus(
        JNIEnv* env, jobject) {

    bool traced = hc_isBeingTraced();
    bool suspicious = hc_hasSuspiciousLibraries();
    bool tracerPid = hc_hasTracerPid();

    int signals = (traced ? 1 : 0) + (suspicious ? 1 : 0) + (tracerPid ? 1 : 0);

    if (signals >= 2) { HC_LOGE("[HC] ALERTE : signaux multiples"); return 4; }
    if (traced)       { HC_LOGE("[HC] ALERTE : ptrace");             return 1; }
    if (suspicious)   { HC_LOGE("[HC] ALERTE : libs suspectes");     return 2; }
    if (tracerPid)    { HC_LOGE("[HC] ALERTE : TracerPid");          return 3; }

    HC_LOGI("[HC] Securite : OK");
    return 0;
}

// Fonctions du Lab 22
extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_lab22_1jnidemo_MainActivity_helloFromJNI(
        JNIEnv* env, jobject) {
    HC_LOGI("[HC] helloFromJNI chargee");
    return env->NewStringUTF("Hello from C++ via JNI !");
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_example_lab22_1jnidemo_MainActivity_factorial(
        JNIEnv* env, jobject, jint n) {
    if (n < 0) return -1;
    long long result = 1;
    for (int i = 1; i <= n; i++) result *= i;
    HC_LOGI("[HC] Factoriel %d = %lld", n, result);
    return static_cast<jint>(result);
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_lab22_1jnidemo_MainActivity_reverseString(
        JNIEnv* env, jobject, jstring javaString) {
    if (javaString == nullptr) return env->NewStringUTF("Erreur");
    const char* raw = env->GetStringUTFChars(javaString, nullptr);
    std::string hcStr(raw);
    env->ReleaseStringUTFChars(javaString, raw);
    std::reverse(hcStr.begin(), hcStr.end());
    return env->NewStringUTF(hcStr.c_str());
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_example_lab22_1jnidemo_MainActivity_sumArray(
        JNIEnv* env, jobject, jintArray array) {
    if (array == nullptr) return -1;
    jsize len = env->GetArrayLength(array);
    jint* hcElements = env->GetIntArrayElements(array, nullptr);
    if (hcElements == nullptr) return -2;
    long long total = 0;
    for (jsize i = 0; i < len; i++) total += hcElements[i];
    env->ReleaseIntArrayElements(array, hcElements, 0);
    return static_cast<jint>(total);
}