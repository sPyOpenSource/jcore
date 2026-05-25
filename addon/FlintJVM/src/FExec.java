import jCPU.JavaVM.VmStackFrame;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicLong;
import jx.classfile.MethodData;

public class FExec extends ListNode {
    public void throwNew(JClass findClass, String file_close_failed) {
        throw new UnsupportedOperationException("Not supported yet."); // Generated from nbfs://nbhost/SystemFileSystem/Templates/Classes/Code/GeneratedMethodBody
    }

    public void throwNew(JClass excpCls) {
        throw new UnsupportedOperationException("Not supported yet."); // Generated from nbfs://nbhost/SystemFileSystem/Templates/Classes/Code/GeneratedMethodBody
    }

    public void throwNew(JClass excpCls, String could_not_find_the_field_ss, String clsName, String name) {
        throw new UnsupportedOperationException("Not supported yet."); // Generated from nbfs://nbhost/SystemFileSystem/Templates/Classes/Code/GeneratedMethodBody
    }

    private List<Integer> stack;
    private int stackLength;
    private int[] opcodes;
    private int lr;
    private int sp;
    private int startSp;
    private int peakSp;
    private JThread onwerThread;
    public Object excp; // Assuming excp is some kind of exception object
    private int pc;
    private MethodData method;

    private static final int FLOAT_NAN = 0x7FC00000;
    private static final long DOUBLE_NAN = 0x7FF8000000000000L;

    private static final Object[] opcodeLabelsStop = null;
    private static final Object[] opcodeLabelsExit = null;

    public FExec(JThread onwer, int stackSize) {
        super();
        this.stackLength = stackSize / Integer.BYTES;
        this.stack = new ArrayList<>(stackLength);
        // Initialize stack with zeros
        for (int i = 0; i < stackLength; i++) {
            stack.add(0);
        }
        this.opcodes = null;
        this.lr = -1;
        this.sp = -1;
        this.startSp = sp;
        this.peakSp = sp;
        this.onwerThread = onwer;
        this.excp = null;
    }

    public void stackPushInt32(int value) {
        sp++;
        ensureStackSize(sp);
        stack.set(sp, value);
        peakSp = sp;
    }

    public void stackPushInt64(long value) {
        sp++;
        ensureStackSize(sp);
        stack.set(sp, (int)(value & 0xFFFFFFFFL));
        sp++;
        ensureStackSize(sp);
        stack.set(sp, (int)((value >>> 32) & 0xFFFFFFFFL));
        peakSp = sp;
    }

    public void stackPushFloat(float value) {
        int intBits = Float.floatToIntBits(value);
        sp++;
        ensureStackSize(sp);
        stack.set(sp, intBits);
        peakSp = sp;
    }

    public void stackPushDouble(double value) {
        long longBits = Double.doubleToLongBits(value);
        sp++;
        ensureStackSize(sp);
        stack.set(sp, (int)(longBits & 0xFFFFFFFFL));
        sp++;
        ensureStackSize(sp);
        stack.set(sp, (int)((longBits >>> 32) & 0xFFFFFFFFL));
        peakSp = sp;
    }

    public void stackPushObject(JObject obj) {
        sp++;
        ensureStackSize(sp);
        stack.set(sp, obj != null ? obj.hashCode() : 0); // Using hashCode as placeholder for pointer
        peakSp = sp;
        if (obj != null && (obj.getProtected() & 0x02) != 0) {
            Flint.clearProtLv2(obj);
        }
    }

    public int stackPopInt32() {
        int value = stack.get(sp);
        sp--;
        return value;
    }

    public long stackPopInt64() {
        int low = stack.get(sp);
        sp--;
        int high = stack.get(sp);
        sp--;
        return ((long)high << 32) | (low & 0xFFFFFFFFL);
    }

    public float stackPopFloat() {
        int intBits = stack.get(sp);
        sp--;
        return Float.intBitsToFloat(intBits);
    }

    public double stackPopDouble() {
        int low = stack.get(sp);
        sp--;
        int high = stack.get(sp);
        sp--;
        long longBits = ((long)high << 32) | (low & 0xFFFFFFFFL);
        return Double.longBitsToDouble(longBits);
    }

    public JObject stackPopObject() {
        int objRef = stack.get(sp);
        sp--;
        // Assuming a method to get JObject from reference (hashCode placeholder)
        return JObject.fromRef(objRef);
    }

    public int getStackTrace(VmStackFrame stackTrace, int traceSp) {
        if (traceSp < 4) return -1;
        int tracePc = stack.get(traceSp - 2);
        MethodData traceMethod = (MethodData) (Object) stack.get(traceSp - 3); // Unsafe cast, placeholder
        stackTrace.init(tracePc, stack.get(traceSp), traceMethod);
        return stack.get(traceSp);
    }

    public boolean getStackTrace(int index, VmStackFrame stackTrace, AtomicInteger isEndStack) {
        if (index == 0) {
            stackTrace.init(pc, startSp, method);
            if (isEndStack != null) isEndStack.set(startSp < 4 ? 1 : 0);
            return true;
        } else {
            int traceSp = startSp;
            do {
                traceSp = getStackTrace(stackTrace, traceSp);
                if (traceSp < 0) return false;
                if (stackTrace.pc != 0xFFFFFFFF) index--;
            } while (stackTrace.pc == 0xFFFFFFFF || index > 0);
            if (isEndStack != null) isEndStack.set(traceSp < 4 ? 1 : 0);
            return true;
        }
    }

    public boolean readLocal(int stackIndex, int localIndex, AtomicInteger value, AtomicInteger isObject) {
        VmStackFrame stackTrace = new VmStackFrame();
        if (!getStackTrace(stackIndex, stackTrace, null)) return false;
        int val = stack.get(stackTrace.baseSp + 1 + localIndex);
        value.set(val);
        if (isObject != null) {
            isObject.set(Flint.isObject(val) ? 1 : 0);
        }
        return true;
    }

    public boolean readLocal(int stackIndex, int localIndex, AtomicLong value) {
        VmStackFrame stackTrace = new VmStackFrame();
        if (!getStackTrace(stackIndex, stackTrace, null)) return false;
        int low = stack.get(stackTrace.baseSp + 1 + localIndex);
        int high = stack.get(stackTrace.baseSp + 2 + localIndex);
        long val = ((long)high << 32) | (low & 0xFFFFFFFFL);
        value.set(val);
        return true;
    }

    private void ensureStackSize(int index) {
        while (stack.size() <= index) {
            stack.add(0);
        }
    }

    // Placeholder for stackPushArgs method and other methods not fully provided in the snippet
    
    private boolean hasTerminateRequest() {
        throw new UnsupportedOperationException("Not supported yet."); // Generated from nbfs://nbhost/SystemFileSystem/Templates/Classes/Code/GeneratedMethodBody
    }
    
    void exec(boolean initOpcodeLabels) {
    opcodeLabelsStop = (const void)opcodeLabelsStop;
    opcodeLabelsExit = (const void)opcodeLabelsExit;

    FDbg dbg = Flint.getDebugger();
    if(initOpcodeLabels) opcodes = (void)opcodeLabels;

    uint8_t code = this->code;

    if(method.loader->getStaticInitStatus() == UNINITIALIZED) {
        invokeStaticCtor(method.loader);
        if(excp != null) goto exception_handler;
        code = this->code;
        break;
    }

    switch opcodes[code[pc]]{
        case dbg_stop: {
            if(dbg && !dbg.checkStop(this)) {
                if(hasTerminateRequest()) {
                    // TODO - ERROR
                    return;
                }
                opcodes = (void volatile)opcodeLabels;
            }
            goto opcodeLabels[code[pc]];
        }
    case op_nop:
        pc++;
        break;
    case op_iconst_m1:
        stackPushInt32(-1);
        pc++;
        break;
    case op_iconst_0:
    case op_aconst_null:
        stackPushInt32(0);
        pc++;
        break;
    case op_iconst_1:
        stackPushInt32(1);
        pc++;
        break;
    case op_iconst_2:
        stackPushInt32(2);
        pc++;
        break;
    case op_iconst_3:
        stackPushInt32(3);
        pc++;
        break;
    case op_iconst_4:
        stackPushInt32(4);
        pc++;
        break;
    case op_iconst_5:
        stackPushInt32(5);
        pc++;
        break;
    case op_lconst_0:
        stackPushInt64(0);
        pc++;
        break;
    case op_lconst_1:
        stackPushInt64(1);
        pc++;
        break;
    case op_fconst_0:
        stackPushFloat(0);
        pc++;
        break;
    case op_fconst_1:
        stackPushFloat(1);
        pc++;
        break;
    case op_fconst_2:
        stackPushFloat(2);
        pc++;
        break;
    case op_dconst_0:
        stackPushDouble(0);
        pc++;
        break;
    case op_dconst_1:
        stackPushDouble(1);
        pc++;
        break;
    case op_bipush:
        stackPushInt32((byte)code[pc + 1]);
        pc += 2;
        break;
    case op_sipush:
        stackPushInt32(ARRAY_TO_INT16(code[pc + 1]));
        pc += 3;
        break;
    case op_ldc: {
        uint8_t poolIndex = code[pc + 1];
        ClassLoader loader = method.loader;
        switch(loader.getConstPoolTag(poolIndex)) {
            case CONST_INTEGER:
                stackPushInt32(loader.getConstInteger(poolIndex));
                pc += 2;
                break;
            case CONST_FLOAT:
                stackPushFloat(loader.getConstFloat(poolIndex));
                pc += 2;
                break;
            case CONST_STRING: {
                JString str = loader.getConstString(this, poolIndex);
                if(str == null) goto exception_handler;
                stackPushObject(str);
                pc += 2;
                break;
            }
            case CONST_CLASS: {
                JClass cls = loader.getConstClass(this, poolIndex);
                if(cls == null) goto exception_handler;
                stackPushObject(cls);
                pc += 2;
                break;
            }
            case CONST_METHOD_TYPE:
                // TODO
                pc += 2;
                break;
            case CONST_METHOD_HANDLE:
                // TODO
                pc += 2;
                break;
            default: {
                JClass excpCls = Flint.findClass(this, "java/lang/ClassFormatError");
                throwNew(excpCls, "Constant pool tag value (%u) is invalid in class %s", loader.getConstPoolTag(poolIndex), loader.getName());
                return;
            }
        }
    }
    case op_ldc_w: {
        ushort poolIndex = ARRAY_TO_INT16(code[pc + 1]);
        ClassLoader loader = method.loader;
        switch(loader.getConstPoolTag(poolIndex)) {
            case CONST_INTEGER:
                stackPushInt32(loader.getConstInteger(poolIndex));
                pc += 3;
                break;
            case CONST_FLOAT:
                stackPushFloat(loader.getConstFloat(poolIndex));
                pc += 3;
                break;
            case CONST_STRING: {
                JString str = loader.getConstString(this, poolIndex);
                if(str == null) goto exception_handler;
                stackPushObject(str);
                pc += 3;
                break;
            }
            case CONST_CLASS: {
                JClass cls = loader.getConstClass(this, poolIndex);
                if(cls == null) goto exception_handler;
                stackPushObject(cls);
                pc += 3;
                break;
            }
            case CONST_METHOD_TYPE:
                // TODO
                pc += 3;
                break;
            case CONST_METHOD_HANDLE:
                // TODO
                pc += 3;
                break;
            default: {
                JClass excpCls = Flint.findClass(this, "java/lang/ClassFormatError");
                throwNew(excpCls, "Constant pool tag value (%u) is invalid in class %s", loader.getConstPoolTag(poolIndex), loader.getName());
                return;
            }
        }
    }
    case op_ldc2_w: {
        ushort poolIndex = ARRAY_TO_INT16(code[pc + 1]);
        ClassLoader loader = method.loader;
        switch(loader.getConstPoolTag(poolIndex)) {
            case CONST_LONG:
                stackPushInt64(loader.getConstLong(poolIndex));
                pc += 3;
                break;
            case CONST_DOUBLE:
                stackPushDouble(loader.getConstDouble(poolIndex));
                pc += 3;
                break;
            default: {
                JClass excpCls = Flint.findClass(this, "java/lang/ClassFormatError");
                throwNew(excpCls, "Constant pool tag value (%u) is invalid in class %s", loader.getConstPoolTag(poolIndex), loader.getName());
                return;
            }
        }
    }
    case op_iload:
    case op_fload:
        stackPushInt32(locals[code[pc + 1]]);
        pc += 2;
        break;
    case op_iload_0:
    case op_fload_0:
        stackPushInt32(locals[0]);
        pc++;
        break;
    case op_iload_1:
    case op_fload_1:
        stackPushInt32(locals[1]);
        pc++;
        break;
    case op_iload_2:
    case op_fload_2:
        stackPushInt32(locals[2]);
        pc++;
        break;
    case op_iload_3:
    case op_fload_3:
        stackPushInt32(locals[3]);
        pc++;
        break;
    case op_lload:
    case op_dload:
        stackPushInt64((long)locals[code[pc + 1]]);
        pc += 2;
        break;
    case op_lload_0:
    case op_dload_0:
        stackPushInt64((long)locals[0]);
        pc++;
        break;
    case op_lload_1:
    case op_dload_1:
        stackPushInt64((long)locals[1]);
        pc++;
        break;
    case op_lload_2:
    case op_dload_2:
        stackPushInt64((long)locals[2]);
        pc++;
        break;
    case op_lload_3:
    case op_dload_3:
        stackPushInt64((long)locals[3]);
        pc++;
        break;
    case op_aload:
        stackPushObject((JObject)locals[code[pc + 1]]);
        pc += 2;
        break;
    case op_aload_0:
        stackPushObject((JObject)locals[0]);
        pc++;
        break;
    case op_aload_1:
        stackPushObject((JObject)locals[1]);
        pc++;
        break;
    case op_aload_2:
        stackPushObject((JObject)locals[2]);
        pc++;
        break;
    case op_aload_3:
        stackPushObject((JObject)locals[3]);
        pc++;
        break;
    case op_iaload:
    case op_faload: {
        int index = stackPopInt32();
        JObject obj = stackPopObject();
        if(obj == null)
            goto load_null_array_excp;
        else if(index < 0 || index >= (obj.size / sizeof(int))) {
            JClass excpCls = Flint.findClass(this, "java/lang/ArrayIndexOutOfBoundsException");
            throwNew(excpCls, "Index %d out of bounds for length %d", index, (obj.size / sizeof(int)));
            goto exception_handler;
        }
        stackPushInt32(((int)obj.data)[index]);
        pc++;
        break;
    }
    case op_laload:
    case op_daload: {
        int index = stackPopInt32();
        JObject obj = stackPopObject();
        if(obj == null)
            goto load_null_array_excp;
        else if(index < 0 || index >= (obj.size / sizeof(long))) {
            JClass excpCls = Flint.findClass(this, "java/lang/ArrayIndexOutOfBoundsException");
            throwNew(excpCls, "Index %d out of bounds for length %d", index, (obj.size / sizeof(long)));
            goto exception_handler;
        }
        stackPushInt64(((long)obj.data)[index]);
        pc++;
        break;
    }
    case op_aaload: {
        int index = stackPopInt32();
        JObject obj = stackPopObject();
        if(obj == null)
            goto load_null_array_excp;
        else if(index < 0 || index >= (obj.size / sizeof(int))) {
            JClass excpCls = Flint.findClass(this, "java/lang/ArrayIndexOutOfBoundsException");
            throwNew(excpCls, "Index %d out of bounds for length %d", index, (obj.size / sizeof(int)));
            goto exception_handler;
        }
        stackPushObject(((JObject)obj.data)[index]);
        pc++;
        break;
    }
    case op_baload: {
        int index = stackPopInt32();
        JObject obj = stackPopObject();
        if(obj == null)
            goto load_null_array_excp;
        else if(index < 0 || index >= (obj.size / sizeof(byte))) {
            JClass excpCls = Flint::findClass(this, "java/lang/ArrayIndexOutOfBoundsException");
            throwNew(excpCls, "Index %d out of bounds for length %d", index, (obj.size / sizeof(byte)));
            goto exception_handler;
        }
        stackPushInt32(((byte)obj.data)[index]);
        pc++;
        break;
    }
    case op_caload:
    case op_saload: {
        int index = stackPopInt32();
        JObject obj = stackPopObject();
        if(obj == null)
            goto load_null_array_excp;
        else if(index < 0 || index >= (obj.size / sizeof(short))) {
            JClass excpCls = Flint::findClass(this, "java/lang/ArrayIndexOutOfBoundsException");
            throwNew(excpCls, "Index %d out of bounds for length %d", index, (obj.size / sizeof(short)));
            goto exception_handler;
        }
        stackPushInt32(((short)obj.data)[index]);
        pc++;
        break;
    }
    case op_istore:
    case op_fstore: {
        uint index = code[pc + 1];
        locals[index] = stackPopInt32();
        pc += 2;
        break;
    }
    case op_lstore:
    case op_dstore: {
        uint index = code[pc + 1];
        (ulong)locals[index] = stackPopInt64();
        pc += 2;
        break;
    }
    case op_astore: {
        uint index = code[pc + 1];
        locals[index] = stackPopInt32();
        pc += 2;
        break;
    }
    case op_istore_0:
    case op_fstore_0: {
        locals[0] = stackPopInt32();
        pc++;
        break;
    }
    case op_istore_1:
    case op_fstore_1: {
        locals[1] = stackPopInt32();
        pc++;
        break;
    }
    case op_istore_2:
    case op_fstore_2: {
        locals[2] = stackPopInt32();
        pc++;
        break;
    }
    case op_istore_3:
    case op_fstore_3: {
        locals[3] = stackPopInt32();
        pc++;
        break;
    }
    case op_lstore_0:
    case op_dstore_0: {
        (ulong)locals[0] = stackPopInt64();
        pc++;
        break;
    }
    case op_lstore_1:
    case op_dstore_1: {
        (ulong)locals[1] = stackPopInt64();
        pc++;
        break;
    }
    case op_lstore_2:
    case op_dstore_2: {
        (ulong)locals[2] = stackPopInt64();
        pc++;
        break;
    }
    case op_lstore_3:
    case op_dstore_3: {
        (ulong)locals[3] = stackPopInt64();
        pc++;
        break;
    }
    case op_astore_0: {
        locals[0] = stackPopInt32();
        pc++;
        break;
    }
    case op_astore_1: {
        locals[1] = stackPopInt32();
        pc++;
        break;
    }
    case op_astore_2: {
        locals[2] = stackPopInt32();
        pc++;
        break;
    }
    case op_astore_3: {
        locals[3] = stackPopInt32();
        pc++;
        break;
    }
    case op_iastore:
    case op_fastore:
    case op_aastore: {
        int value = stackPopInt32();
        int index = stackPopInt32();
        JObject obj = stackPopObject();
        if(obj == null)
            goto store_null_array_excp;
        else if((index < 0) || (index >= (obj.size / sizeof(int)))) {
            JClass excpCls = Flint.findClass(this, "java/lang/ArrayIndexOutOfBoundsException");
            throwNew(excpCls, "Index %d out of bounds for length %d", index, (obj.size / sizeof(int)));
            goto exception_handler;
        }
        ((int)obj.data)[index] = value;
        pc++;
        break;
    }
    case op_lastore:
    case op_dastore: {
        long value = stackPopInt64();
        int index = stackPopInt32();
        JObject obj = stackPopObject();
        if(obj == null)
            goto store_null_array_excp;
        else if((index < 0) || (index >= (obj.size / sizeof(long)))) {
            JClass excpCls = Flint.findClass(this, "java/lang/ArrayIndexOutOfBoundsException");
            throwNew(excpCls, "Index %d out of bounds for length %d", index, (obj.size / sizeof(long)));
            goto exception_handler;
        }
        ((long)obj.data)[index] = value;
        pc++;
        break;
    }
    case op_bastore: {
        int value = stackPopInt32();
        int index = stackPopInt32();
        JObject obj = stackPopObject();
        if(obj == null)
            goto store_null_array_excp;
        else if((index < 0) || (index >= (obj.size / sizeof(byte)))) {
            JClass excpCls = Flint.findClass(this, "java/lang/ArrayIndexOutOfBoundsException");
            throwNew(excpCls, "Index %d out of bounds for length %d", index, (obj.size / sizeof(byte)));
            goto exception_handler;
        }
        ((byte)obj.data)[index] = value;
        pc++;
        break;
    }
    case op_castore:
    case op_sastore: {
        int value = stackPopInt32();
        int index = stackPopInt32();
        JObject obj = stackPopObject();
        if(obj == null)
            goto store_null_array_excp;
        else if((index < 0) || (index >= (obj.size / sizeof(short)))) {
            JClass excpCls = Flint.findClass(this, "java/lang/ArrayIndexOutOfBoundsException");
            throwNew(excpCls, "Index %d out of bounds for length %d", index, (obj.size / sizeof(short)));
            goto exception_handler;
        }
        ((short)obj.data)[index] = value;
        pc++;
        break;
    }
    case op_pop:
        stackPopInt32();
        pc++;
        break;
    case op_pop2:
        stackPopInt64();
        pc++;
        break;
    case op_dup: {
        int value = GET_STACK_VALUE(sp);
        stackPushInt32(value);
        pc++;
        break;
    }
    case op_dup_x1: {
        int value2 = GET_STACK_VALUE(sp - 1);
        int value1 = GET_STACK_VALUE(sp - 0);
        stackPushInt32(value1);
        SET_STACK_VALUE(sp - 1, value2);
        SET_STACK_VALUE(sp - 2, value1);
        pc++;
        break;
    }
    case op_dup_x2: {
        int value3 = GET_STACK_VALUE(sp - 2);
        int value2 = GET_STACK_VALUE(sp - 1);
        int value1 = GET_STACK_VALUE(sp - 0);
        stackPushInt32(value1);
        SET_STACK_VALUE(sp - 1, value2);
        SET_STACK_VALUE(sp - 2, value3);
        SET_STACK_VALUE(sp - 3, value1);
        pc++;
        break;
    }
    case op_dup2: {
        int value2 = GET_STACK_VALUE(sp - 1);
        int value1 = GET_STACK_VALUE(sp - 0);
        stackPushInt32(value2);
        stackPushInt32(value1);
        pc++;
        break;
    }
    case op_dup2_x1: {
        int value3 = GET_STACK_VALUE(sp - 2);
        int value2 = GET_STACK_VALUE(sp - 1);
        int value1 = GET_STACK_VALUE(sp - 0);
        stackPushInt32(value2);
        stackPushInt32(value1);
        SET_STACK_VALUE(sp - 2, value3);
        SET_STACK_VALUE(sp - 3, value1);
        SET_STACK_VALUE(sp - 4, value2);
        pc++;
        break;
    }
    case op_dup2_x2: {
        int value4 = GET_STACK_VALUE(sp - 3);
        int value3 = GET_STACK_VALUE(sp - 2);
        int value2 = GET_STACK_VALUE(sp - 1);
        int value1 = GET_STACK_VALUE(sp - 0);
        stackPushInt32(value2);
        stackPushInt32(value1);
        SET_STACK_VALUE(sp - 2, value3);
        SET_STACK_VALUE(sp - 3, value4);
        SET_STACK_VALUE(sp - 4, value1);
        SET_STACK_VALUE(sp - 5, value2);
        pc++;
        break;
    }
    case op_iadd: {
        int value2 = stackPopInt32();
        int value1 = stackPopInt32();
        stackPushInt32(value1 + value2);
        pc++;
        break;
    }
    case op_ladd: {
        long value2 = stackPopInt64();
        long value1 = stackPopInt64();
        stackPushInt64(value1 + value2);
        pc++;
        break;
    }
    case op_fadd: {
        float value2 = stackPopFloat();
        float value1 = stackPopFloat();
        stackPushFloat(value1 + value2);
        pc++;
        break;
    }
    case op_dadd: {
        double value2 = stackPopDouble();
        double value1 = stackPopDouble();
        stackPushDouble(value1 + value2);
        pc++;
        break;
    }
    case op_isub: {
        int value2 = stackPopInt32();
        int value1 = stackPopInt32();
        stackPushInt32(value1 - value2);
        pc++;
        break;
    }
    case op_lsub: {
        long value2 = stackPopInt64();
        long value1 = stackPopInt64();
        stackPushInt64(value1 - value2);
        pc++;
        break;
    }
    case op_fsub: {
        float value2 = stackPopFloat();
        float value1 = stackPopFloat();
        stackPushFloat(value1 - value2);
        pc++;
        break;
    }
    case op_dsub: {
        double value2 = stackPopDouble();
        double value1 = stackPopDouble();
        stackPushDouble(value1 - value2);
        pc++;
        break;
    }
    case op_imul: {
        int value2 = stackPopInt32();
        int value1 = stackPopInt32();
        stackPushInt32(value1 * value2);
        pc++;
        break;
    }
    case op_lmul: {
        long value2 = stackPopInt64();
        long value1 = stackPopInt64();
        stackPushInt64(value1 * value2);
        pc++;
        break;
    }
    case op_fmul: {
        float value2 = stackPopFloat();
        float value1 = stackPopFloat();
        stackPushFloat(value1 * value2);
        pc++;
        break;
    }
    case op_dmul: {
        double value2 = stackPopDouble();
        double value1 = stackPopDouble();
        stackPushDouble(value1 * value2);
        pc++;
        break;
    }
    case op_idiv: {
        int value2 = stackPopInt32();
        int value1 = stackPopInt32();
        if(value2 == 0)
            goto divided_by_zero_excp;
        stackPushInt32(value1 / value2);
        pc++;
        break;
    }
    case op_ldiv: {
        long value2 = stackPopInt64();
        long value1 = stackPopInt64();
        if(value2 == 0)
            goto divided_by_zero_excp;
        stackPushInt64(value1 / value2);
        pc++;
        break;
    }
    case op_fdiv: {
        float value2 = stackPopFloat();
        float value1 = stackPopFloat();
        stackPushFloat(value1 / value2);
        pc++;
        break;
    }
    case op_ddiv: {
        double value2 = stackPopDouble();
        double value1 = stackPopDouble();
        stackPushDouble(value1 / value2);
        pc++;
        break;
    }
    case op_irem: {
        int value2 = stackPopInt32();
        int value1 = stackPopInt32();
        if(value2 == 0)
            goto divided_by_zero_excp;
        stackPushInt32(value1 % value2);
        pc++;
        break;
    }
    case op_lrem: {
        long value2 = stackPopInt64();
        long value1 = stackPopInt64();
        if(value2 == 0)
            goto divided_by_zero_excp;
        stackPushInt64(value1 % value2);
        pc++;
        break;
    }
    case op_frem: {
        float value2 = stackPopFloat();
        float value1 = stackPopFloat();
        int temp = (int)(value1 / value2);
        stackPushFloat(value1 - (temp * value2));
        pc++;
        break;
    }
    case op_drem: {
        double value2 = stackPopDouble();
        double value1 = stackPopDouble();
        long temp = (long)(value1 / value2);
        stackPushDouble(value1 - (temp * value2));
        pc++;
        break;
    }
    case op_ineg:
        stack[sp] = -stack[sp];
        pc++;
        break;
    case op_lneg:
        stackPushInt64(-stackPopInt64());
        pc++;
        break;
    case op_fneg:
        stackPushFloat(-stackPopFloat());
        pc++;
        break;
    case op_dneg:
        stackPushDouble(-stackPopDouble());
        pc++;
        break;
    case op_ishl: {
        int position = stackPopInt32();
        int value = stackPopInt32();
        stackPushInt32(value << position);
        pc++;
        break;
    }
    case op_lshl: {
        int position = stackPopInt32();
        long value = stackPopInt64();
        stackPushInt64(value << position);
        pc++;
        break;
    }
    case op_ishr: {
        int position = stackPopInt32();
        int value = stackPopInt32();
        stackPushInt32(value >> position);
        pc++;
        break;
    }
    case op_lshr: {
        int position = stackPopInt32();
        long value = stackPopInt64();
        stackPushInt64(value >> position);
        pc++;
        break;
    }
    case op_iushr: {
        int position = stackPopInt32();
        uint value = stackPopInt32();
        stackPushInt32(value >> position);
        pc++;
        break;
    }
    case op_lushr: {
        int position = stackPopInt32();
        ulong value = stackPopInt64();
        stackPushInt64(value >> position);
        pc++;
        break;
    }
    case op_iand: {
        int value2 = stackPopInt32();
        int value1 = stackPopInt32();
        stackPushInt32(value1 & value2);
        pc++;
        break;
    }
    case op_land: {
        long value2 = stackPopInt64();
        long value1 = stackPopInt64();
        stackPushInt64(value1 & value2);
        pc++;
        break;
    }
    case op_ior: {
        int value2 = stackPopInt32();
        int value1 = stackPopInt32();
        stackPushInt32(value1 | value2);
        pc++;
        break;
    }
    case op_lor: {
        long value2 = stackPopInt64();
        long value1 = stackPopInt64();
        stackPushInt64(value1 | value2);
        pc++;
        break;
    }
    case op_ixor: {
        int value2 = stackPopInt32();
        int value1 = stackPopInt32();
        stackPushInt32(value1 ^ value2);
        pc++;
        break;
    }
    case op_lxor: {
        long value2 = stackPopInt64();
        long value1 = stackPopInt64();
        stackPushInt64(value1 ^ value2);
        pc++;
        break;
    }
    case op_iinc:
        locals[code[pc + 1]] += (byte)code[pc + 2];
        pc += 3;
        break;
    case op_i2l:
        stackPushInt64(stackPopInt32());
        pc++;
        break;
    case op_i2f: {
        float value = stack[sp];
        stack[sp] = (int)value;
        pc++;
        break;
    }
    case op_i2d:
        stackPushDouble(stackPopInt32());
        pc++;
        break;
    case op_l2i:
        stackPushInt32(stackPopInt64());
        pc++;
        break;
    case op_l2f:
        stackPushFloat(stackPopInt64());
        pc++;
        break;
    case op_l2d:
        stackPushDouble(stackPopInt64());
        pc++;
        break;
    case op_f2i:
        stack[sp] = (float)stack[sp];
        pc++;
        break;
    case op_f2l:
        stackPushInt64(stackPopFloat());
        pc++;
        break;
    case op_f2d:
        stackPushDouble(stackPopFloat());
        pc++;
        break;
    case op_d2i:
        stackPushInt32(stackPopDouble());
        pc++;
        break;
    case op_d2l:
        stackPushInt64(stackPopDouble());
        pc++;
        break;
    case op_d2f:
        stackPushFloat(stackPopDouble());
        pc++;
        break;
    case op_i2b:
        stack[sp] = (byte)stack[sp];
        pc++;
        break;
    case op_i2c:
    case op_i2s:
        stack[sp] = (short)stack[sp];
        pc++;
        break;
    case op_lcmp: {
        long value2 = stackPopInt64();
        long value1 = stackPopInt64();
        stackPushInt32((value1 == value2) ? 0 : ((value1 < value2) ? -1 : 1));
        pc++;
        break;
    }
    case op_fcmpl:
    case op_fcmpg: {
        float value2 = stackPopFloat();
        float value1 = stackPopFloat();
        if(((uint)value1 == FLOAT_NAN) || ((uint)value2 == FLOAT_NAN))
            stackPushInt32((code[pc] == OP_FCMPL) ? -1 : 1);
        else if(value1 > value2)
            stackPushInt32(1);
        else if(value1 == value2)
            stackPushInt32(0);
        else
            stackPushInt32(-1);
        pc++;
        break;
    }
    case op_dcmpl:
    case op_dcmpg: {
        double value2 = stackPopDouble();
        double value1 = stackPopDouble();
        if(((ulong)value1 == DOUBLE_NAN) || ((ulong)value2 == DOUBLE_NAN))
            stackPushInt32((code[pc] == OP_DCMPL) ? -1 : 1);
        else if(value1 > value2)
            stackPushInt32(1);
        else if(value1 == value2)
            stackPushInt32(0);
        else
            stackPushInt32(-1);
        pc++;
        break;
    }
    case op_ifeq:
    case op_ifnull:
        pc += (!stackPopInt32()) ? ARRAY_TO_INT16(code[pc + 1]) : 3;
        break;
    case op_ifne:
    case op_ifnonnull:
        pc += stackPopInt32() ? ARRAY_TO_INT16(code[pc + 1]) : 3;
        break;
    case op_iflt:
        pc += (stackPopInt32() < 0) ? ARRAY_TO_INT16(code[pc + 1]) : 3;
        break;
    case op_ifge:
        pc += (stackPopInt32() >= 0) ? ARRAY_TO_INT16(code[pc + 1]) : 3;
        break;
    case op_ifgt:
        pc += (stackPopInt32() > 0) ? ARRAY_TO_INT16(code[pc + 1]) : 3;
        break;
    case op_ifle:
        pc += (stackPopInt32() <= 0) ? ARRAY_TO_INT16(code[pc + 1]) : 3;
        break;
    case op_if_icmpeq:
    case op_if_acmpeq: {
        int value2 = stackPopInt32();
        int value1 = stackPopInt32();
        pc += (value1 == value2) ? ARRAY_TO_INT16(code[pc + 1]) : 3;
        break;
    }
    case op_if_icmpne:
    case op_if_acmpne: {
        int value2 = stackPopInt32();
        int value1 = stackPopInt32();
        pc += (value1 != value2) ? ARRAY_TO_INT16(code[pc + 1]) : 3;
        break;
    }
    case op_if_icmplt: {
        int value2 = stackPopInt32();
        int value1 = stackPopInt32();
        pc += (value1 < value2) ? ARRAY_TO_INT16(code[pc + 1]) : 3;
        break;
    }
    case op_if_icmpge: {
        int value2 = stackPopInt32();
        int value1 = stackPopInt32();
        pc += (value1 >= value2) ? ARRAY_TO_INT16(code[pc + 1]) : 3;
        break;
    }
    case op_if_icmpgt: {
        int value2 = stackPopInt32();
        int value1 = stackPopInt32();
        pc += (value1 > value2) ? ARRAY_TO_INT16(code[pc + 1]) : 3;
        break;
    }
    case op_if_icmple: {
        int value2 = stackPopInt32();
        int value1 = stackPopInt32();
        pc += (value1 <= value2) ? ARRAY_TO_INT16(code[pc + 1]) : 3;
        break;
    }
    case op_goto:
        pc += ARRAY_TO_INT16(code[pc + 1]);
        break;
    case op_goto_w:
        pc += ARRAY_TO_INT32(code[pc + 1]);
        break;
    case op_jsr:
        stackPushInt32(pc + 3);
        pc += ARRAY_TO_INT16(code[pc + 1]);
        break;
    case op_jsrw:
        stackPushInt32(pc + 5);
        pc += ARRAY_TO_INT32(code[pc + 1]);
        break;
    case op_ret:
        pc = locals[code[pc + 1]];
        break;
    case op_tableswitch: {
        int index = stackPopInt32();
        uint8_t padding = (4 - ((pc + 1) % 4)) % 4;
        uint8_t table = code[pc + padding + 1];
        int low = ARRAY_TO_INT32(&table[4]);
        int height = ARRAY_TO_INT32(table[8]);
        if(index < low || index > height) {
            int defaultOffset = ARRAY_TO_INT32(table);
            pc += defaultOffset;
            break;
        }
        table = table[12 + (index - low) * 4];
        pc += ARRAY_TO_INT32(table);
        break;
    }
    case op_lookupswitch: {
        int key = stackPopInt32();
        uint8_t padding = (4 - ((pc + 1) % 4)) % 4;
        uint8_t table = code[pc + padding + 1];
        int defaultPc = ARRAY_TO_INT32(table);
        int npairs = ARRAY_TO_INT32(table[4]);
        table = table[8];
        while(npairs--) {
            int pairs = ARRAY_TO_INT32(table);
            if(key == pairs) {
                pc += ARRAY_TO_INT32(table[4]);
                break;
            }
            table = table[8];
        }
        pc += defaultPc;
        break;
    }
    case op_ireturn:
    case op_freturn: {
        int retVal = stackPopInt32();
        restoreContext();
        code = this->code;
        stackPushInt32(retVal);
        pc = lr;
        break;
    }
    case op_lreturn:
    case op_dreturn: {
        long retVal = stackPopInt64();
        restoreContext();
        code = this->code;
        stackPushInt64(retVal);
        pc = lr;
        break;
    }
    case op_areturn: {
        int retVal = (int)stackPopObject();
        restoreContext();
        code = this->code;
        stackPushObject((JObject)retVal);
        pc = lr;
        break;
    }
    case op_return: {
        restoreContext();
        code = this->code;
        peakSp = sp;
        pc = lr;
        break;
    }
    case op_getstatic: {
        ConstField constField = method.loader->getConstField(this, ARRAY_TO_INT16(code[pc + 1]));
        if(constField == null) goto exception_handler;
        ClassLoader clsLoader = constField.loader;
        if(clsLoader == null) {
            clsLoader = Flint.findLoader(this, constField.className);
            if(clsLoader == null) goto exception_handler;
            constField.loader = clsLoader;
        }
        StaticInitStatus initStatus = clsLoader.getStaticInitStatus();
        if(initStatus == INITIALIZED || (initStatus == INITIALIZING && clsLoader.monitorOwnId == (uint)this)) {
            FieldValue fieldValue = clsLoader.getStaticField(this, constField);
            if(fieldValue == null) goto exception_handler;
            switch(constField.nameAndType->desc[0]) {
                case 'J':
                case 'D': {
                    stackPushInt64(fieldValue.getInt64());
                    pc += 3;
                    break;
                }
                case 'L':
                case '[': {
                    stackPushObject(fieldValue.getObj());
                    pc += 3;
                    break;
                }
                default: {
                    stackPushInt32(fieldValue.getInt32());
                    pc += 3;
                    break;
                }
            }
        }
        else if(initStatus == UNINITIALIZED) {
            invokeStaticCtor(clsLoader);
            if(excp != null) goto exception_handler;
            code = this->code;
            break;
        }
        FlintAPI.Thread.yield();
        if(hasTerminateRequest()) {
            // TODO - ERROR
            return;
        }
        goto opcodeLabels[OP_GETSTATIC];
    }
    case op_putstatic: {
        ConstField constField = method.loader->getConstField(this, ARRAY_TO_INT16(code[pc + 1]));
        if(constField == null) goto exception_handler;
        ClassLoader clsLoader = constField.loader;
        if(clsLoader == null) {
            clsLoader = Flint.findLoader(this, constField.className);
            if(clsLoader == null) goto exception_handler;
            constField.loader = clsLoader;
        }
        StaticInitStatus initStatus = clsLoader->getStaticInitStatus();
        if(initStatus == INITIALIZED || (initStatus == INITIALIZING && clsLoader.monitorOwnId == (uint)this)) {
            FieldValue fieldValue = clsLoader.getStaticField(this, constField);
            if(fieldValue == null) goto exception_handler;
            switch(constField.nameAndType->desc[0]) {
                case 'Z':
                case 'B': {
                    fieldValue.setInt32((byte)stackPopInt32());
                    pc += 3;
                    break;
                }
                case 'C':
                case 'S': {
                    fieldValue.setInt32((short)stackPopInt32());
                    pc += 3;
                    break;
                }
                case 'J':
                case 'D': {
                    fieldValue.setInt64(stackPopInt64());
                    pc += 3;
                    break;
                }
                case 'L':
                case '[': {
                    fieldValue.setObj(stackPopObject());
                    pc += 3;
                    break;
                }
                default: {
                    fieldValue.setInt32(stackPopInt32());
                    pc += 3;
                    break;
                }
            }
        }
        else if(initStatus == UNINITIALIZED) {
            invokeStaticCtor(clsLoader);
            if(excp != null) goto exception_handler;
            code = this->code;
            break;
        }
        FlintAPI.Thread.yield();
        if(hasTerminateRequest()) {
            // TODO - ERROR
            return;
        }
        goto opcodeLabels[OP_PUTSTATIC];
    }
    case op_getfield: {
        ConstField constField = method.loader->getConstField(this, ARRAY_TO_INT16(code[pc + 1]));
        if(constField == null) goto exception_handler;
        JObject obj = stackPopObject();
        if(obj == null) {
            JClass excpCls = Flint.findClass(this, "java/lang/NullPointerException");
            throwNew(excpCls, "Cannot access field %s.%s from null object", constField.className, constField.nameAndType->name);
            goto exception_handler;
        }
        FieldValue fieldValue = obj.getField(this, constField);
        if(fieldValue == null) goto exception_handler;
        switch(constField.nameAndType->desc[0]) {
            case 'J':
            case 'D': {
                stackPushInt64(fieldValue.getInt64());
                pc += 3;
                break;
            }
            case 'L':
            case '[': {
                stackPushObject(fieldValue.getObj());
                pc += 3;
                break;
            }
            default: {
                stackPushInt32(fieldValue.getInt32());
                pc += 3;
                break;
            }
        }
    }
    case op_putfield: {
        ConstField constField = method.loader->getConstField(this, ARRAY_TO_INT16(&code[pc + 1]));
        if(constField == null) goto exception_handler;
        switch(constField.nameAndType->desc[0]) {
            case 'Z':
            case 'B': {
                int value = stackPopInt32();
                JObject obj = stackPopObject();
                if(obj == null) {
                    JClass excpCls = Flint.findClass(this, "java/lang/NullPointerException");
                    throwNew(excpCls, "Cannot access field %s.%s from null object", constField.className, constField->nameAndType->name);
                    goto exception_handler;
                }
                FieldValue fieldValue = obj.getField(this, constField);
                if(fieldValue == null) goto exception_handler;
                fieldValue.setInt32((byte)value);
                pc += 3;
                break;
            }
            case 'C':
            case 'S': {
                int value = stackPopInt32();
                JObject obj = stackPopObject();
                if(obj == null) {
                    JClass excpCls = Flint.findClass(this, "java/lang/NullPointerException");
                    throwNew(excpCls, "Cannot access field %s.%s from null object", constField.className, constField->nameAndType->name);
                    goto exception_handler;
                }
                FieldValue fieldValue = obj.getField(this, constField);
                if(fieldValue == null) goto exception_handler;
                fieldValue.setInt32((short)value);
                pc += 3;
                break;
            }
            case 'J':
            case 'D': {
                long value = stackPopInt64();
                JObject obj = stackPopObject();
                if(obj == null) {
                    JClass excpCls = Flint.findClass(this, "java/lang/NullPointerException");
                    throwNew(excpCls, "Cannot access field %s.%s from null object", constField.className, constField->nameAndType->name);
                    goto exception_handler;
                }
                FieldValue fieldValue = obj.getField(this, constField);
                if(fieldValue == null) goto exception_handler;
                fieldValue.setInt64(value);
                pc += 3;
                break;
            }
            case 'L':
            case '[': {
                JObject value = stackPopObject();
                JObject obj = stackPopObject();
                if(obj == null) {
                    JClass excpCls = Flint.findClass(this, "java/lang/NullPointerException");
                    throwNew(excpCls, "Cannot access field %s.%s from null object", constField.className, constField->nameAndType->name);
                    goto exception_handler;
                }
                FieldValue fieldValue = obj.getField(this, constField);
                if(fieldValue == null) goto exception_handler;
                fieldValue.setObj(value);
                pc += 3;
                break;
            }
            default: {
                int value = stackPopInt32();
                JObject obj = stackPopObject();
                if(obj == null) {
                    JClass excpCls = Flint.findClass(this, "java/lang/NullPointerException");
                    throwNew(excpCls, "Cannot access field %s.%s from null object", constField.className, constField->nameAndType->name);
                    goto exception_handler;
                }
                FieldValue fieldValue = obj.getField(this, constField);
                if(fieldValue == null) goto exception_handler;
                fieldValue->setInt32(value);
                pc += 3;
                break;
            }
        }
    }
    case op_invokevirtual: {
        ConstMethod constMethod = method.loader->getConstMethod(this, ARRAY_TO_INT16(code[pc + 1]));
        if(constMethod == null) goto exception_handler;
        invokeVirtual(constMethod);
        if(excp != null) goto exception_handler;
        code = this->code;
        break;
    }
    case op_invokespecial: {
        ConstMethod constMethod = method.loader->getConstMethod(this, ARRAY_TO_INT16(code[pc + 1]));
        if(constMethod == null) goto exception_handler;
        invokeSpecial(constMethod);
        if(excp != null) goto exception_handler;
        code = this->code;
        break;
    }
    case op_invokestatic: {
        ConstMethod constMethod = method.loader->getConstMethod(this, ARRAY_TO_INT16(code[pc + 1]));
        if(constMethod == null) goto exception_handler;
        invokeStatic(constMethod);
        if(excp != null) goto exception_handler;
        code = this.code;
        break;
    }
    case op_invokeinterface: {
        ConstInterfaceMethod interfaceMethod = method.loader->getConstInterfaceMethod(this, ARRAY_TO_INT16(code[pc + 1]));
        if(interfaceMethod == null) goto exception_handler;
        uint8_t count = code[pc + 3];
        invokeInterface(interfaceMethod, count);
        if(excp != null) goto exception_handler;
        code = this.code;
        break;
    }
    case op_invokedynamic: {
        // TODO
        // break;
        JClass excpCls = Flint.findClass(this, "java/lang/UnsupportedOperationException");
        throwNew(excpCls, "Invokedynamic instructions are not supported");
        goto exception_handler;
    }
    case op_new: {
        JClass cls = method.loader->getConstClass(this, ARRAY_TO_INT16(code[pc + 1]));
        JObject obj = Flint.newObject(this, cls);
        if(obj == null) goto exception_handler;
        stackPushObject(obj);
        pc += 3;
        break;
    }
    case op_newarray: {
        static constexpr const char primArrayTypeName[] = {"[Z", "[C", "[F", "[D", "[B", "[S", "[I", "[J"};
        int count = stackPopInt32();
        if(count < 0)
            goto negative_array_size_excp;
        uint8_t atype = code[pc + 1];
        JObject obj = Flint.newArray(this, Flint.findClass(this, primArrayTypeName[atype - 4]), count);
        if(obj == null) goto exception_handler;
        obj.clearData();
        stackPushObject(obj);
        pc += 2;
        break;
    }
    case op_anewarray: {
        int count = stackPopInt32();
        if(count < 0)
            goto negative_array_size_excp;
        JClass cls = method.loader->getConstClass(this, ARRAY_TO_INT16(code[pc + 1]));
        if(cls == null) goto exception_handler;
        cls = Flint::findClassOfArray(this, cls.getTypeName(), 1);
        JObject array = Flint::newArray(this, cls, count);
        if(array == null) goto exception_handler;
        array.clearData();
        stackPushObject(array);
        pc += 3;
        break;
    }
    case op_arraylength: {
        JObject obj = stackPopObject();
        if(obj == null) {
            throwNew(Flint.findClass(this, "java/lang/NullPointerException"), "Cannot read the array length from null object");
            goto exception_handler;
        }
        stackPushInt32(obj.size / obj.type->componentSize());
        pc++;
        break;
    }
    case op_athrow: {
        excp = (JThrowable)stackPopObject();
        if(excp == null)
            throwNew(Flint.findClass(this, "java/lang/NullPointerException"), "Cannot throw exception by null object");
        goto exception_handler;
    }
    exception_handler: {
        uint tracePc = pc;
        int traceStartSp = startSp;
        MethodInfo traceMethod = method;
        JObject obj = excp;
        if(((uint)obj & 0x01) != 0) return; /* Is a fatal error, not a throwable, Cannot be handled */
        if(dbg && dbg.exceptionIsEnabled())
            dbg.caughtException(this);
        while(true) {
            ushort exceptionLength = traceMethod.getExceptionLength();
            for(ushort i = 0; i < exceptionLength; i++) {
                ExceptionTable exception = traceMethod.getException(i);
                if(exception.startPc <= tracePc && tracePc < exception.endPc) {
                    boolean isMatch = false;
                    if(exception.catchType == 0)
                        isMatch = true;
                    else {
                        JClass catchType = traceMethod.loader->getConstClass(this, exception.catchType);
                        if(catchType == null) {
                            while(startSp > traceStartSp) restoreContext();
                            code = this->code;
                            sp = startSp + traceMethod.getMaxLocals();
                            pc = exception.handlerPc;
                            goto exception_handler;
                        }
                        isMatch = Flint.isInstanceof(this, obj, catchType);
                        if(isMatch == false && excp != obj) {
                            while(startSp > traceStartSp) restoreContext();
                            code = this->code;
                            sp = startSp + traceMethod.getMaxLocals();
                            pc = exception.handlerPc;
                            goto exception_handler;
                        }
                    }
                    if(isMatch) {
                        while(startSp > traceStartSp) restoreContext();
                        code = this->code;
                        sp = startSp + traceMethod.getMaxLocals();
                        pc = exception.handlerPc;
                        stackPushObject(obj);
                        excp = null;
                        break;
                    }
                }
            }
            if(traceStartSp < 4) {
                if(dbg && !dbg.exceptionIsEnabled())
                    dbg.caughtException(this);
                return;
            }
            traceMethod = (MethodInfo)stack[traceStartSp - 3];
            tracePc = stack[traceStartSp - 2];
            traceStartSp = stack[traceStartSp];
            if(tracePc == 0xFFFFFFFF) return;
        }
    }
    case op_checkcast: {
        JObject obj = (JObject)stack[sp];
        if(obj != 0) {
            JClass catchType = method.loader.getConstClass(this, ARRAY_TO_INT16(code[pc + 1]));
            boolean isIns = Flint.isInstanceof(this, obj, catchType);
            if(isIns == false) {
                if(excp == null) {
                    JClass excpCls = Flint.findClass(this, "java/lang/ClassCastException");
                    throwNew(excpCls, "Class %s cannot be cast to class %s", obj.getTypeName(), catchType.getTypeName());
                }
                goto exception_handler;
            }
        }
        pc += 3;
        break;
    }
    case op_instanceof: {
        JObject obj = stackPopObject();
        JClass type = method.loader.getConstClass(this, ARRAY_TO_INT16(code[pc + 1]));
        boolean isIns = Flint.isInstanceof(this, obj, type);
        if(isIns == true)
            stackPushInt32(1);
        else {
            if(excp != null) goto exception_handler;
            stackPushInt32(0);
        }
        pc += 3;
        break;
    }
    case op_monitorenter: {
        JObject obj = (JObject)GET_STACK_VALUE(sp);
        if(obj == null) {
            throwNew(Flint.findClass(this, "java/lang/NullPointerException"), "Cannot enter synchronized block by null object");
            goto exception_handler;
        }
        if(lockObject(obj) == false) {
            if(excp != null) goto exception_handler;
            FlintAPI.Thread.yield();
            break;
        }
        stackPopObject();
        pc++;
        break;
    }
    case op_monitorexit: {
        JObject obj = stackPopObject();
        unlockObject(obj);
        pc++;
        break;
    }
    case op_wide: {
        switch((FlintOpCode)code[pc + 1]) {
            case OP_IINC: {
                ushort index = ARRAY_TO_INT16(code[pc + 2]);
                locals[index] += ARRAY_TO_INT16(code[pc + 4]);
                pc += 6;
                break;
            }
            case OP_ALOAD: {
                ushort index = ARRAY_TO_INT16(code[pc + 2]);
                stackPushObject((JObject)locals[index]);
                pc += 4;
                break;
            }
            case OP_FLOAD:
            case OP_ILOAD: {
                ushort index = ARRAY_TO_INT16(code[pc + 2]);
                stackPushInt32(locals[index]);
                pc += 4;
                break;
            }
            case OP_LLOAD:
            case OP_DLOAD: {
                ushort index = ARRAY_TO_INT16(code[pc + 2]);
                stackPushInt64((long)locals[index]);
                pc += 4;
                break;
            }
            case OP_ASTORE: {
                ushort index = ARRAY_TO_INT16(code[pc + 2]);
                locals[index] = stackPopInt32();
                pc += 4;
                break;
            }
            case OP_FSTORE:
            case OP_ISTORE: {
                ushort index = ARRAY_TO_INT16(code[pc + 2]);
                locals[index] = stackPopInt32();
                pc += 4;
                break;
            }
            case OP_LSTORE:
            case OP_DSTORE: {
                ushort index = ARRAY_TO_INT16(code[pc + 2]);
                (ulong)locals[index] = stackPopInt64();
                pc += 4;
                break;
            }
            case OP_RET: {
                ushort index = ARRAY_TO_INT16(code[pc + 2]);
                pc = locals[index];
                break;
            }
            default:
                goto op_unknow;
        }
    }
    case op_multianewarray: {
        JClass cls = method.loader.getConstClass(this, ARRAY_TO_INT16(code[pc + 1]));
        uint8_t dimensions = code[pc + 3];
        for(int i = 0; i < dimensions; i++) {
            if(stack[sp + i] < 0)
                goto negative_array_size_excp;
        }
        JObject array = Flint.newMultiArray(this, cls, stack[sp - dimensions + 1], dimensions);
        if(array == null) goto exception_handler;
        sp -= dimensions;
        stackPushObject(array);
        pc += 4;
        break;
    }
    case op_breakpoint: {
        if(!dbg) {
            pc++;
            break;
        }
        dbg.hitBreakpoint(this);
        if(hasTerminateRequest()) {
            // TODO - ERROR
            return;
        }
        uint8_t op = code[pc];
        if(op == OP_BREAKPOINT) { /* Check opcode again, maybe breakpoint was deleted by user */
            op = dbg.getSavedOpcode(pc, method);
            if(op == OP_UNKNOW)
                op = code[pc];
            else if(op == OP_BREAKPOINT) {
                pc++;
                goto opcodes[op];
            }
        }
        goto opcodeLabels[op];
    }
    case op_breakpoint_dummy: {
        uint8_t op = code[pc];
        if(op == OP_BREAKPOINT_DUMMY) {
            ((uint8_t)code)[pc] = OP_BREAKPOINT;
            op = dbg.getSavedOpcode(pc, method);
        }
        goto opcodeLabels[op];
    }
    case op_unknow: {
        throwNew(Flint.findClass(this, "java/lang/ClassFormatError"), "Invalid opcode %u", code[pc]);
        return;
    }
    divided_by_zero_excp: {
        throwNew(Flint.findClass(this, "java/lang/ArithmeticException"), "Divided by zero");
        goto exception_handler;
    }
    negative_array_size_excp: {
        throwNew(Flint.findClass(this, "java/lang/NegativeArraySizeException"), "Size of the array is a negative number");
        goto exception_handler;
    }
    load_null_array_excp: {
        throwNew(Flint.findClass(this, "java/lang/NullPointerException"), "Cannot load from null array object");
        goto exception_handler;
    }
    store_null_array_excp: {
        throwNew(Flint.findClass(this, "java/lang/NullPointerException"), "Cannot store to null array object");
        goto exception_handler;
    }
    case op_exit:
        return;
    }
    }
    
    void runTask(FExec exec) {
        exec.exec(true);
        if(exec.excp != null) {
            if(((uint)exec.excp & 0x01) == 0) { /* Is throwable object */
                JString str = exec.excp->getDetailMessage();
                if(str != null) {
                    System.out.print(exec.excp.getTypeName());
                    System.out.print(": ");
                    System.out.println(str);
                }
                else
                    Flint::println(exec.excp.getTypeName());
            }
            else {  /* Is not throwable object, it is error message */
                const char msg = (char)((uint)exec.excp & 0xFFFFFFFC);
                System.out.println(msg);
            }
            Flint.terminateRequest();
        }
        while(exec.startSp > 3) exec.restoreContext();
        exec.peakSp = -1;
        Flint.freeExecution(exec);
        FlintAPI.Thread.terminate(0);
    }
}
