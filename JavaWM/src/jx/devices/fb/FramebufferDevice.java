package jx.devices.fb;

import jx.zero.*;
import jx.devices.*;

/**
 * Each display device must implement this interface.
 * @author Michael Golm 
 */
public interface FramebufferDevice extends Device, Portal {
	public int getWidth();
    	public int getHeight();
	public int setMode (int nWidth, int nHeight, ColorSpace eColorSpace);
	public void startFrameBufferUpdate();
	public void endFrameBufferUpdate();
	public void startUpdate();
	public void endUpdate();
	public DeviceMemory getFrameBuffer();
	public int getFrameBufferOffset();
	public int getBytesPerLine();
	public ColorSpace getColorSpace();
	public int drawLine(PixelRect cDraw, PixelRect cClipped, PixelColor cColor, DrawingMode nDrawingMode);
	public int fillRect(PixelRect cRect[], int nCount, PixelColor cColor, DrawingMode nDrawingMode);
	public int bitBlt(PixelRect acOldPos[], PixelRect acNewPos[], int nCount);
        @Override
    	public void open(DeviceConfiguration conf);
        @Override
	DeviceConfigurationTemplate[] getSupportedConfigurations();
        @Override
    	public void close();
}
