package jx.devices.fb;

import jx.devices.*;

public class FramebufferConfiguration extends DeviceConfiguration {
    public int xresolution;
    public int yresolution;
    public ColorSpace colorSpace;

    public FramebufferConfiguration() {}
    public FramebufferConfiguration(int xresolution, int yresolution, ColorSpace colorSpace) {
	this.xresolution = xresolution;
	this.yresolution = yresolution;
	this.colorSpace  = colorSpace;
    }
}
