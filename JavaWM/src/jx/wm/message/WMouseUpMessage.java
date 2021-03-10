package jx.wm.message;

import jx.devices.fb.PixelPoint;

public class WMouseUpMessage extends WMessage
{
	public PixelPoint	m_cScreenPos;
	public PixelPoint	m_cPos;
	public int		m_nButton;
		
	public WMouseUpMessage (PixelPoint cScreenPos, PixelPoint cPos, int nButton)
	{
		super (WMessage.MOUSE_UP);
		m_cScreenPos 	= new PixelPoint (cScreenPos);
		m_cPos		= new PixelPoint (cPos);
		m_nButton	= nButton;
	}
        @Override
	public String toString ()
	{
		return "MSG(MOUSE_UP)";
	}
}
