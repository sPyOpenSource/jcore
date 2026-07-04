package x86os;
public class Hello {
    public static native void helloPrint(String msg);
    public static void main(String[] args) {
        helloPrint("Hello from x86OS JVM!\n\r");
    }
}
