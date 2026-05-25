
import jx.classfile.FieldData;


public class JClass {
    private static class InternalData {
        FieldData fields;
        String typeName;
        ClassLoader classLoader;
    }

    private InternalData data;

    public JClass(String typeName, ClassLoader loader) {
        this.data = new InternalData();
        this.data.typeName = typeName;
        this.data.classLoader = loader;
    }

    public String getTypeName() {
        return data.typeName;
    }

    public ClassLoader getClassLoader() {
        return data.classLoader;
    }

    public static char isPrimitive(String typeName) {
        if (typeName == null) return 0;
        switch (typeName) {
            case "int": return 'I';
            case "void": return 'V';
            case "byte": return 'B';
            case "char": return 'C';
            case "long": return 'J';
            case "float": return 'F';
            case "short": return 'S';
            case "double": return 'D';
            case "boolean": return 'Z';
            default: return 0;
        }
    }

    public boolean isPrimitive() {
        return getClassLoader() == null;
    }

    public boolean isArray() {
        String typeName = getTypeName();
        return typeName != null && !typeName.isEmpty() && typeName.charAt(0) == '[';
    }

    public JClass getNestHost(FExec ctx) {
        if (isArray() || isPrimitive()) return this;
        return getClassLoader().getNestHost(ctx);
    }

    public int getNestMembersCount() {
        return getClassLoader().getNestMembersCount();
    }

    public JClass getNestMember(FExec ctx, int index) {
        return getClassLoader().getNestMember(ctx, index);
    }

    public int componentSize() {
        String typeName = getTypeName();
        if (typeName == null || typeName.length() < 2 || typeName.charAt(0) != '[')
            return 0;
        switch (typeName.charAt(1)) {
            case 'Z':
            case 'B':
                return 1;
            case 'C':
            case 'S':
                return 2;
            case 'J':
            case 'D':
                return 8;
            default:
                return 4;
        }
    }

    public int size() {
        // Java does not have sizeof operator like C++.
        // This method can return an estimated size or be omitted.
        // Here we return a fixed value or 0 as placeholder.
        return 0;
    }
}
