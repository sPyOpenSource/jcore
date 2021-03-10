package jx.wm.message;

import jx.devices.fb.PixelPoint;

public class WMouseMovedMessage extends WMessage
{
	public PixelPoint	m_cScreenPos;
	public PixelPoint	m_cPos;
        public PixelPoint       m_cDelta;
	public int		m_nTransit;
		
	public WMouseMovedMessage (PixelPoint cScreenPos, PixelPoint cPos, int nTransit)
	{
		super (WMessage.MOUSE_MOVED);
		m_cScreenPos 	= new PixelPoint (cScreenPos);
		m_cPos		= new PixelPoint (cPos);
		m_nTransit	= nTransit;
	}
        @Override
	public String toString ()
	{
		return "MSG(MOUSE_MOVED)";
	}
}
