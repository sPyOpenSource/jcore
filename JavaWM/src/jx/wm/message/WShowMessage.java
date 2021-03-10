package jx.wm.message;

public class WShowMessage extends WMessage
{
	public boolean m_bShow;
		
	public WShowMessage (boolean bShow)
	{
		super (WMessage.WINDOW_SHOW);
		m_bShow = bShow;
	}
        @Override
	public String toString ()
	{
		return "MSG(WINDOW_SHOW)";
	}
}
