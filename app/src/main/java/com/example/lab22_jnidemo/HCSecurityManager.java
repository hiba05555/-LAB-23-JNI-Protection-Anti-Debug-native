package com.example.lab22_jnidemo;

public class HCSecurityManager {

    static {
        System.loadLibrary("native-lib");
    }

    public native int getSecurityStatus();

    public String getStatusMessage() {
        int code = getSecurityStatus();
        switch (code) {
            case 0: return "Securite : OK";
            case 1: return "ALERTE : ptrace detecte";
            case 2: return "ALERTE : bibliotheques suspectes";
            case 3: return "ALERTE : TracerPid detecte";
            case 4: return "ALERTE : signaux multiples detectes";
            default: return "Etat inconnu";
        }
    }

    public boolean isSuspicious() {
        return getSecurityStatus() != 0;
    }
}