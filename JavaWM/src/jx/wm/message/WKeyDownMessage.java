package jx.wm.message;

import jx.wm.Keycode;
import jx.wm.Qualifiers;

public class WKeyDownMessage extends WMessage
{
	public Keycode m_eKeyCode;
	public Keycode m_eRawCode;
	public Qualifiers m_eQual;
	
	public WKeyDownMessage (Keycode eKeyCode, Keycode eRawCode, Qualifiers eQual)
	{
		super (WMessage.KEY_DOWN);
		m_eKeyCode = new Keycode (eKeyCode);
		m_eRawCode = new Keycode (eRawCode);
		m_eQual	   = new Qualifiers (eQual);
	}
        @Override
	public String toString ()
	{
		return "MSG(KEY_DOWN)";
	}
}
