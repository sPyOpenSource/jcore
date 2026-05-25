import java.io.*;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public class FlintFileUtils {
    private static final int FILE_NAME_BUFF_SIZE = 1024;

    private static int append(char[] buff, int index, String str, int len) {
        if (index >= FILE_NAME_BUFF_SIZE) return -1;
        int i = 0;
        while (i < str.length() && len-- > 0) {
            if (index >= FILE_NAME_BUFF_SIZE) return -1;
            buff[index++] = str.charAt(i++);
        }
        if (index < FILE_NAME_BUFF_SIZE) {
            buff[index] = 0;
        }
        return index;
    }

    private static int append(char[] buff, int index, String str) {
        return append(buff, index, str, 0xFFFF);
    }

    private static char getPathSeparatorChar() {
        return File.separatorChar;
    }

    private static int combinePath(char[] buff, String folder, int folderLen, String clsName, int clsLen) {
        int index = 0;
        index = append(buff, index, folder, folderLen);
        if (index == -1) return -1;
        char separatorChar = getPathSeparatorChar();
        if (buff[index - 1] != separatorChar) {
            if (index >= FILE_NAME_BUFF_SIZE) return -1;
            buff[index++] = separatorChar;
        }
        index = append(buff, index, clsName, clsLen);
        if (index == -1) return -1;
        return append(buff, index, ".class");
    }

    private static String resolvePath(String fileName, int length) {
        // Placeholder for ResolvePath logic, returning the fileName as is
        return fileName.substring(0, Math.min(length, fileName.length()));
    }

    private static String getClassPaths() {
        // Placeholder for Flint.getClassPaths()
        return System.getProperty("java.class.path");
    }

    public static InputStream fopen(String fileName, int length) throws IOException {
        String resolvedPath = resolvePath(fileName, length);
        String filePath = resolvedPath + ".class";
        File file = new File(filePath);
        if (file.exists() && file.isFile()) {
            return new FileInputStream(file);
        }

        String jdks = getClassPaths();
        if (jdks != null) {
            String[] paths = jdks.split(";");
            for (String path : paths) {
                char[] buff = new char[FILE_NAME_BUFF_SIZE];
                int res = combinePath(buff, path, path.length(), fileName, length);
                if (res == -1) return null;
                String combinedPath = new String(buff, 0, res);
                File f = new File(combinedPath);
                if (f.exists() && f.isFile()) {
                    return new FileInputStream(f);
                }
            }
        }
        return null;
    }

    public static boolean fread(FExec ctx, InputStream file, byte[] buff, int size) throws IOException {
        int read = file.read(buff, 0, size);
        if (read != size) {
            if (ctx != null) {
                ctx.throwNew(Flint.findClass(ctx, "java/io/IOException"), "File read failed");
            }
            return false;
        }
        return true;
    }

    public static boolean freadUInt8(FExec ctx, InputStream file, byte[] value) throws IOException {
        return fread(ctx, file, value, 1);
    }

    public static boolean freadUInt16(FExec ctx, InputStream file, short[] value) throws IOException {
        byte[] buff = new byte[2];
        if (!fread(ctx, file, buff, 2)) return false;
        ByteBuffer bb = ByteBuffer.wrap(buff).order(ByteOrder.BIG_ENDIAN);
        value[0] = bb.getShort();
        return true;
    }

    public static boolean freadUInt32(FExec ctx, InputStream file, int[] value) throws IOException {
        byte[] buff = new byte[4];
        if (!fread(ctx, file, buff, 4)) return false;
        ByteBuffer bb = ByteBuffer.wrap(buff).order(ByteOrder.BIG_ENDIAN);
        value[0] = bb.getInt();
        return true;
    }

    public static boolean freadUInt64(FExec ctx, InputStream file, long[] value) throws IOException {
        byte[] buff = new byte[8];
        if (!fread(ctx, file, buff, 8)) return false;
        ByteBuffer bb = ByteBuffer.wrap(buff).order(ByteOrder.BIG_ENDIAN);
        value[0] = bb.getLong();
        return true;
    }

    public static boolean fseek(FExec ctx, RandomAccessFile file, long offset) throws IOException {
        try {
            file.seek(offset);
            return true;
        } catch (IOException e) {
            if (ctx != null) {
                ctx.throwNew(Flint.findClass(ctx, "java/io/IOException"), "File seek failed");
            }
            return false;
        }
    }

    public static boolean foffset(FExec ctx, RandomAccessFile file, long offset) throws IOException {
        try {
            long current = file.getFilePointer();
            return fseek(ctx, file, current + offset);
        } catch (IOException e) {
            if (ctx != null) {
                ctx.throwNew(Flint.findClass(ctx, "java/io/IOException"), "File offset failed");
            }
            return false;
        }
    }

    public static boolean fclose(FExec ctx, Closeable file) {
        if (file == null) return true;
        try {
            file.close();
            return true;
        } catch (IOException e) {
            if (ctx != null) {
                ctx.throwNew(Flint.findClass(ctx, "java/io/IOException"), "File close failed");
            }
            return false;
        }
    }
}
