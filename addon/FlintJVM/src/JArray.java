import java.nio.ByteBuffer;
import java.nio.IntBuffer;

public class JArray {
    protected int size;
    protected JType type;
    protected ByteBuffer data;

    public int getLength() {
        return size / type.componentSize();
    }

    public int getSizeInByte() {
        return size;
    }

    public byte componentSize() {
        return type.componentSize();
    }

    public String getCompTypeName(int[] length) {
        String name = getTypeName();
        int len = 0;
        int index = 0;
        while (index < name.length() && name.charAt(index) == '[') index++;
        if (index < name.length() && name.charAt(index) == 'L') index++;
        int start = index;
        while (index < name.length() && name.charAt(index) != ';') index++;
        len = index - start;
        if (len == 1) {
            char c = name.charAt(start);
            switch (c) {
                case 'Z': length[0] = "boolean".length(); return "boolean";
                case 'C': length[0] = "char".length(); return "char";
                case 'F': length[0] = "float".length(); return "float";
                case 'D': length[0] = "double".length(); return "double";
                case 'B': length[0] = "byte".length(); return "byte";
                case 'S': length[0] = "short".length(); return "short";
                case 'I': length[0] = "int".length(); return "int";
                case 'J': length[0] = "long".length(); return "long";
                case 'V': length[0] = "void".length(); return "void";
                default: break;
            }
        }
        length[0] = len;
        return name.substring(start, start + len);
    }

    public ByteBuffer getData() {
        return data.asReadOnlyBuffer();
    }

    public void clearArray() {
        clearData();
    }

    protected void clearData() {
        if (data != null) {
            data.clear();
        }
    }

    protected String getTypeName() {
        // Placeholder for actual implementation
        return "";
    }
}

class JInt8Array extends JArray {
    @Override
    public int getLength() {
        return size;
    }

    public byte[] getData() {
        byte[] arr = new byte[size];
        data.position(0);
        data.get(arr);
        return arr;
    }
}

class JInt16Array extends JArray {
    @Override
    public int getLength() {
        return size / 2;
    }

    public short[] getData() {
        short[] arr = new short[size / 2];
        data.asShortBuffer().get(arr);
        return arr;
    }
}

class JUInt16Array extends JArray {
    public char[] getData() {
        char[] arr = new char[size / 2];
        data.asCharBuffer().get(arr);
        return arr;
    }
}

class JInt32Array extends JArray {
    @Override
    public int getLength() {
        return size / 4;
    }

    public int[] getData() {
        int[] arr = new int[size / 4];
        data.asIntBuffer().get(arr);
        return arr;
    }
}

class JFloatArray extends JArray {
    @Override
    public int getLength() {
        return size / 4;
    }

    public float[] getData() {
        float[] arr = new float[size / 4];
        data.asFloatBuffer().get(arr);
        return arr;
    }
}

class JInt64Array extends JArray {
    @Override
    public int getLength() {
        return size / 8;
    }

    public long[] getData() {
        long[] arr = new long[size / 8];
        data.asLongBuffer().get(arr);
        return arr;
    }
}

class JDoubleArray extends JArray {
    @Override
    public int getLength() {
        return size / 8;
    }

    public double[] getData() {
        double[] arr = new double[size / 8];
        data.asDoubleBuffer().get(arr);
        return arr;
    }
}

class JObjectArray extends JArray {
    @Override
    public int getLength() {
        return size / 4;
    }

    public JObject[] getData() {
        // Assuming JObject references are stored as int handles or pointers,
        // this needs to be adapted based on actual implementation.
        int count = size / 4;
        JObject[] arr = new JObject[count];
        IntBuffer intBuf = data.asIntBuffer();
        for (int i = 0; i < count; i++) {
            int handle = intBuf.get(i);
            arr[i] = JObject.fromHandle(handle);
        }
        return arr;
    }
}

// Placeholder classes for JType and JObject
class JType {
    public byte componentSize() {
        // Placeholder for actual implementation
        return 1;
    }
}

class JObject {
    public static JObject fromHandle(int handle) {
        // Placeholder for actual implementation
        return new JObject();
    }
}