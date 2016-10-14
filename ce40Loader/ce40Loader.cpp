//  ce40Loader.cpp : Defines the entry point for the console application.
//
// c. 2016 Technologic Systems   --- http://www.embeddedarm.com
//  Written by Michael D. Peters
// This program demonstrates a bit-banged approach to loading the bitmap file
//  for the TS-4900 ICE40 FPGA.
// This software requires a 3rd party SDK, visit www.guruce.com for the 
//  i.MX6 TS-4900 Windows Embedded Compact 2013 SDK.
//
// Information on the TS-4900 is available at http://www.embeddedarm.com/products/TS-4900

#include "stdafx.h"
#include <windows.h>
#include <iostream>
#include <gpiosdk.h>  // This header is in the GuruCE SDK.

// I need three functions:
//  Function 1:  Clock out 8 data bits to SPI2_MOSI.
//    Needs GPIO for SPI2_CLK and SPI2_MOSI.
//    Needs an array of 8 bits.
//    Return an error status if one of the GPIO isn't working.
//
//  Function 2:  Feed func. 1 a big array 8 bits at a time.
//    Needs a big a rray full of data to push out.
//    Needs to hold the correct chip select low until done.
//    Also get the GPIO structures going...
//
//  Function 3:  Bang out this little "Secret Handshake" 
//   to put the FPGA in program mode.

int wmain(int argc, wchar_t *argv[])
{
	int err = 0;

	printf("This is the FPGA loader.  Thanks, I'll take it from here.\n");

	// set up the GPIO
	GPIO fpga_cs, mosi, miso, spi_clk, fpga_done, fpga_reset;
	GPIO_CONFIG gpioConfig;
	memset(&gpioConfig, 0, sizeof(gpioConfig));
	gpioConfig.Interrupt = GPIO_INTR_NONE;
	gpioConfig.Hysteresis = GPIO_HYSTERESIS_DISABLE;
	gpioConfig.Function = GPIO_FUNCTION_ALT5;
	gpioConfig.OpenDrain = GPIO_OPENDRAIN_DISABLE;
	gpioConfig.Loopback = GPIO_LOOPBACK_DISABLE;
	gpioConfig.Slew = GPIO_SLEW_FAST;

	if (GpioInit()) {
		// FPGA Chip Select is CSI0_DAT16 -> L4 -> gpio port 6 pin 2.
		fpga_cs.Port = GPIO_PORT_6;
		fpga_cs.Pin = GPIO_PIN_2;
		fpga_cs.Value = 1;
		gpioConfig.Port = fpga_cs.Port;
		gpioConfig.Pin = fpga_cs.Pin;
		gpioConfig.Direction = GPIO_DIR_OUT;
		gpioConfig.Pull = GPIO_PULL_UP_100K;
		gpioConfig.Drive = GPIO_DRIVE_34_OHM;
		if (!GpioSetConfig(&gpioConfig)) {
			// do error thing
		};

		// SPI MOSI is CSI0_DAT9 -> GPIO5_IO27
		mosi.Port = GPIO_PORT_5;
		mosi.Pin = GPIO_PIN_27;
		mosi.Value = 0;
		gpioConfig.Port = mosi.Port;
		gpioConfig.Pin = mosi.Pin;
		gpioConfig.Direction = GPIO_DIR_OUT;
		gpioConfig.Pull = GPIO_PULL_UP_100K;
		gpioConfig.Drive = GPIO_DRIVE_34_OHM;
		if (!GpioSetConfig(&gpioConfig)) {
			// do error thing
		}

		// SPI MISO is CSI0_DAT10 -> GPIO5_IO28   <--- Do I even NEED MISO?
		miso.Port = GPIO_PORT_5;
		miso.Pin = GPIO_PIN_28;
		miso.Value = 0;  // it's an input.
		gpioConfig.Port = miso.Port;
		gpioConfig.Pin = miso.Pin;
		gpioConfig.Direction = GPIO_DIR_IN;
		gpioConfig.Pull = GPIO_PULL_INVALID; // I don't think we want to put voltage on this pin.
		gpioConfig.Drive = GPIO_DRIVE_DISABLED;  // Don't drive this pin.
		if (!GpioSetConfig(&gpioConfig)) {
			// do error thing
		}

		// SPI CLOCK is CSI0_DAT8 -> GPIO5_IO26
		spi_clk.Port = GPIO_PORT_5;
		spi_clk.Pin = GPIO_PIN_26;
		spi_clk.Value = 1;
		gpioConfig.Port = spi_clk.Port;
		gpioConfig.Pin = spi_clk.Pin;
		gpioConfig.Direction = GPIO_DIR_OUT;
		gpioConfig.Pull = GPIO_PULL_UP_100K;  // Weak pull up?  Maybe need stronger.  CLK should idle high.
		gpioConfig.Drive = GPIO_DRIVE_34_OHM;
		if (!GpioSetConfig(&gpioConfig)) {
			// do error thing
		}

		// FPGA Done is GPIO Port 5, pin 20
		fpga_done.Port = GPIO_PORT_5;
		fpga_done.Pin = GPIO_PIN_20;
		fpga_done.Value = 0;
		gpioConfig.Port = fpga_done.Port;
		gpioConfig.Pin = fpga_done.Pin;
		gpioConfig.Direction = GPIO_DIR_IN;
		gpioConfig.Pull = GPIO_PULL_UP_100K; // weak pull.  I want the fpga to drive this low.
		gpioConfig.Drive = GPIO_DRIVE_DISABLED;  // this is an input, don't drive it.
		if (!GpioSetConfig(&gpioConfig)) {
			// do error thing
		}

		// FPGA Reset is GPIO port 5, pin 21
		fpga_reset.Port = GPIO_PORT_5;
		fpga_reset.Pin = GPIO_PIN_21;
		fpga_reset.Value = 1; 
		gpioConfig.Port = fpga_reset.Port;
		gpioConfig.Pin = fpga_reset.Pin;
		gpioConfig.Direction = GPIO_DIR_OUT;
		gpioConfig.Pull = GPIO_PULL_UP_47K;
		gpioConfig.Drive = GPIO_DRIVE_34_OHM;
		if (!GpioSetConfig(&gpioConfig)) {
			// do error thing
		}
	}

#define TEST
#ifdef TEST
	err = GetLastError();
	// Check gpio pin functions.
	if (!GpioWritePin(&fpga_reset))
		std::cout << "Couldn't write fpga reset." << std::endl;
	if (!GpioReadPin(&fpga_done) && (err = GetLastError()))
		std::cout << "Couldn't read FPGA done, error code " << err << "." << std::endl;
	if (!GpioReadPin(&miso) && (err = GetLastError()))
		std::cout << "Couldn't read miso, error code " << err << "." << std::endl;
	if (!GpioWritePin(&mosi))
		std::cout << "Couldn't set mosi." << std::endl;
	if (!GpioWritePin(&fpga_cs))
		std::cout << "Couldn't set fpga chip select." << std::endl;
	if (!GpioWritePin(&spi_clk))
		std::cout << "Couldn't tick spi clock." << std::endl;
#endif

	// load the file

	// pass the array to the feeder

	// check FPGA status
	
	// clean up

	return 0;

}
