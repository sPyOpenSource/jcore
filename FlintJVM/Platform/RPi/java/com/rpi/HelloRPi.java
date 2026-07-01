
package com.rpi;

public class HelloRPi {
    public static void main(String[] args) {
        Hardware.rpiPrint("Hello from FlintJVM on RPi!\n");
        
        long startTime = Hardware.rpiCurrentTimeMillis();
        Hardware.rpiPrint("Time: " + startTime + "ms\n");
        
        Hardware.rpiSetPin(21, 1);
        Hardware.rpiPrint("GPIO 21 set HIGH\n");
        
        int random = Hardware.rpiRandom();
        Hardware.rpiPrint("Random: " + random + "\n");
        
        Hardware.rpiPrint("FlintJVM-RPi test complete!\n");
    }
}
