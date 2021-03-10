package jx.wm.message;

public class WWindowActivatedMessage extends WMessage
{
	public boolean m_bActivated;
		
	public WWindowActivatedMessage (boolean bActivated)
	{
		super (WMessage.WINDOW_ACTIVATED);
		m_bActivated = bActivated;
	}
        @Override
	public String toString ()
	{
		return "MSG(WINDOW_ACTIVATED)";
	}
}
