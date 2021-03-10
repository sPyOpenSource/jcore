package jx.wm;

import jx.zero.*;
import jx.devices.fb.*;

public interface WindowManager extends Portal
{
    WWindowInterface createWindow (String cName, PixelRect cFrame, WindowFlags nFlags);
    void postInputEvent (jx.wm.message.WMessage cEvent);
    void handleKeyUp (int nKeyCode);
    void handleKeyDown (int nKeyCode);
    void handleMouseDown (int nButton);
    void handleMouseUp (int nButton);
    void handleMouseMoved (int nDeltaX, int nDeltaY);
    void handleMousePosition (int nPosX, int nPosY);
    int getDisplayWidth();
    int getDisplayHeight();
    void registerHotkeyPlugin(HotkeyPlugin plugin, int keycode) throws TooManyPluginsException;
}
