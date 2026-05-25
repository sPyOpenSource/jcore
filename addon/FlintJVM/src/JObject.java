import java.util.Arrays;
import jx.classfile.FieldData;

public class JObject extends ListNode {

    static JObject fromRef(int objRef) {
        throw new UnsupportedOperationException("Not supported yet."); // Generated from nbfs://nbhost/SystemFileSystem/Templates/Classes/Code/GeneratedMethodBody
    }
    
    private int size;
    private byte prot = 0x02;
    private JClass type;
    private int monitorCount = 0;
    private int ownId = 0;
    private byte[] data;

    public JObject(int size, JClass type) {
        super();
        this.size = size;
        this.type = type;
        this.data = new byte[size];
    }

    public String getTypeName() {
        if (type == null)
            return "java/lang/Class";
        return type.getTypeName();
    }

    public boolean initFields(FExec ctx, ClassLoader loader) {
        FieldData field = new FieldData(data);
        return field.init(ctx, loader, false);
    }

    private static void throwNoSuchFieldError(FExec ctx, String clsName, String name) {
        JClass excpCls = Flint.findClass(ctx, "java/lang/NoSuchFieldError");
        ctx.throwNew(excpCls, "Could not find the field %s.%s", clsName, name);
    }

    public FieldValue getField(FExec ctx, ConstField field) {
        FieldData fieldsData = new FieldData(data);
        FieldValue ret = fieldsData.getField(field);
        if (ret == null && ctx != null)
            throwNoSuchFieldError(ctx, field.className, field.nameAndType.name);
        return ret;
    }

    public FieldValue getField(FExec ctx, String name) {
        FieldData fieldsData = new FieldData(data);
        FieldValue ret = fieldsData.getField(name);
        if (ret == null && ctx != null)
            throwNoSuchFieldError(ctx, getTypeName(), name);
        return ret;
    }

    public FieldValue getFieldByIndex(int index) {
        FieldData fieldsData = new FieldData(data);
        return fieldsData.getFieldByIndex(index);
    }

    public void clearData() {
        Arrays.fill(data, (byte) 0);
    }

    public boolean isArray() {
        return (type != null && type.isArray());
    }

    public void clearProtected() {
        prot = 0;
    }

    public void setProtected() {
        prot = 1;
    }

    public byte getProtected() {
        return prot;
    }

    @Override
    protected void finalize() throws Throwable {
        try {
            if (type == null || !type.isArray()) {
                FieldData fieldsData = new FieldData(data);
                fieldsData.destroy();
            }
        } finally {
            super.finalize();
        }
    }
}
