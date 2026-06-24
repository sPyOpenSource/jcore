
package com.rpi;

public class Hardware {
    public static native void rpiPrint(String msg);
    public static native long rpiCurrentTimeMillis();
    public static native void rpiSetPin(int pin, int value);
    public static native int rpiRandom();
}
