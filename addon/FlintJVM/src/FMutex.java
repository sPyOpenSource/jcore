import java.util.concurrent.atomic.AtomicBoolean;

public class FMutex {
    private final AtomicBoolean locked = new AtomicBoolean(false);
    private int lockNest = 0;
    private Thread lockThread = null;

    public FMutex() {
        // Constructor initializes variables
    }

    public void lock() {
        Thread currentThread = Thread.currentThread();
        while (true) {
            while (locked.getAndSet(true)) {
                try {
                    Thread.sleep(1);
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                }
            }
            if (lockThread == null) {
                lockNest = 1;
                lockThread = currentThread;
                locked.set(false);
                return;
            } else if (lockThread == currentThread) {
                lockNest++;
                locked.set(false);
                return;
            }
            locked.set(false);
            try {
                Thread.sleep(1);
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        }
    }

    public void unlock() {
        while (locked.getAndSet(true)) {
            try {
                Thread.sleep(1);
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        }
        if (lockNest > 0) {
            if (--lockNest == 0) {
                lockThread = null;
            }
        }
        locked.set(false);
    }
}
