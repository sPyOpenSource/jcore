public class JThrowable {
    private final Object[] fields = new Object[2];

    public String getDetailMessage() {
        return (String) fields[0];
    }

    public void setDetailMessage(String strObj) {
        fields[0] = strObj;
    }

    public JThrowable getCause() {
        return (JThrowable) fields[1];
    }

    public void setCause(JThrowable cause) {
        fields[1] = cause;
    }
}
