package jx.wm.message;

import jx.devices.fb.PixelRect;

public class WWindowFrameChangedMessage extends WMessage
{
	public PixelRect m_cFrame;
		
	public WWindowFrameChangedMessage (PixelRect cFrame)
	{
		super (WMessage.WINDOW_FRAME_CHANGED);
		m_cFrame = new PixelRect (cFrame);
	}
        @Override
	public String toString ()
	{
		return "MSG(WINDOW_FRAME_CHANGED)";
	}
}
