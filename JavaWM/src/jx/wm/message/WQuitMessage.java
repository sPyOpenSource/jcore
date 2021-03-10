package jx.wm.message;

public class WQuitMessage extends WMessage
{
	public WQuitMessage ()
	{
		super (WMessage.QUIT);
	}
        @Override
	public String toString ()
	{
		return "MSG(QUIT)";
	}
}
