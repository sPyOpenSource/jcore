package jx.wm.message;

public class WRequestCloseMessage extends WMessage
{
	public WRequestCloseMessage ()
	{
		super (WMessage.REQUEST_CLOSE);
	}
        @Override
	public String toString ()
	{
		return "MSG(REQUEST_CLOSE)";
	}
}
