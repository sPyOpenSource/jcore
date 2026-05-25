
public class FlintUtf8 {
    private static final byte[] utf8ByteCount = {
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
        4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6
    };

    public static int utf8DecodeSizeOneChar(byte c) {
        return (c & 0x80) != 0 ? utf8ByteCount[((c & 0xFF) - 0xC0) & 0xFC] : 1;
    }

    public static int utf8EncodeSize(int c) {
        return (c < 0x80) ? 1 : ((c < 0x0800) ? 2 : 3);
    }

    public static int utf8DecodeOneChar(byte[] c, int offset) {
        if ((c[offset] & 0x80) != 0) {
            int byteCount = utf8DecodeSizeOneChar(c[offset]);
            int code = c[offset] & (0xFF >> (byteCount + 1));
            for (int i = 1; i < byteCount; i++) {
                code <<= 6;
                code |= c[offset + i] & 0x3F;
            }
            return code;
        }
        return c[offset] & 0xFF;
    }

    public static int utf8EncodeOneChar(int c, byte[] buff) {
        if (c < 0x80) {
            buff[0] = (byte) c;
            return 1;
        } else if (c < 0x0800) {
            buff[0] = (byte) (0xC0 | (c >> 6));
            buff[1] = (byte) (0x80 | (c & 0x3F));
            return 2;
        } else {
            buff[0] = (byte) (0xE0 | (c >> 12));
            buff[1] = (byte) (0x80 | ((c >> 6) & 0x3F));
            buff[2] = (byte) (0x80 | (c & 0x3F));
            return 3;
        }
    }

    public static int utf8StrLen(byte[] utf8) {
        int len = 0;
        int i = 0;
        while (i < utf8.length && utf8[i] != 0) {
            int size = utf8DecodeSizeOneChar(utf8[i]);
            i += size;
            len++;
        }
        return len;
    }
}
