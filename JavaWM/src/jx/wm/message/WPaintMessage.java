package jx.wm.message;

import jx.wm.WRegion;

public class WPaintMessage extends WMessage
{
	public WRegion	m_cRegion;
	
	public WPaintMessage (WRegion cRegion)
	{
		super (WMessage.PAINT);
		m_cRegion	= new WRegion (cRegion);
	}
        @Override
	public String toString ()
	{
		return "MSG(PAINT, " + m_cRegion.getBounds() + ")";
	}
}
