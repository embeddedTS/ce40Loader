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
	printf("This is the FPGA loader.  Thanks, I'll take it from here.\n");

	// set up the GPIO
	GPIO fpga_cs, mosi, miso, spi_clk, fpga_done, fpga_reset;
	GPIO_CONFIG fpga_csConfig, mosiConfig, misoConfig, spi_clkConfig, fpga_doneConfig, fpga_resetConfig;

	if (GpioInit()) {
		// FPGA Chip Select is CSI0_DAT16 -> L4 -> gpio port 6 pin 2.
		fpga_cs.Port = GPIO_PORT_6;
		fpga_cs.Pin = GPIO_PIN_2;
		fpga_cs.Value = 1;
		fpga_csConfig.Port = fpga_cs.Port;
		fpga_csConfig.Pin = fpga_cs.Pin;
		fpga_csConfig.Direction = GPIO_DIR_OUT;
		fpga_csConfig.Hysteresis = GPIO_HYSTERESIS_DISABLE;
		fpga_csConfig.Function = GPIO_FUNCTION_ALT5;
		fpga_csConfig.OpenDrain = GPIO_OPENDRAIN_DISABLE;
		fpga_csConfig.Loopback = GPIO_LOOPBACK_DISABLE;
		fpga_csConfig.Pull = GPIO_PULL_UP_100K;
		fpga_csConfig.Slew = GPIO_SLEW_FAST;
		fpga_csConfig.Drive = GPIO_DRIVE_34_OHM;
		if (!GpioSetConfig(&fpga_csConfig)) {
			// do error thing
		};

		// SPI MOSI is ???
		mosi.Port = ;
		mosi.Pin = ;
		mosi.Value = ;
		mosiConfig.Port = mosi.Port;
		mosiConfig.Pin = mosi.Pin;
		mosiConfig.Direction = GPIO_DIR_OUT;
		mosiConfig.Hysteresis = GPIO_HYSTERESIS_DISABLE;
		mosiConfig.Function = GPIO_FUNCTION_ALT5;
		mosiConfig.OpenDrain = GPIO_OPENDRAIN_DISABLE;
		mosiConfig.Loopback = GPIO_LOOPBACK_DISABLE;
		mosiConfig.Pull = GPIO_PULL_UP_100K;
		mosiConfig.Slew = GPIO_SLEW_FAST;
		mosiConfig.Drive = GPIO_DRIVE_34_OHM;
		if (!GpioSetConfig(&mosiConfig)) {
			// do error thing
		}

		// SPI CLOCK is ???
		spi_clk.Port = ;
		spi_clk.Pin = ;
		spi_clk.Value = ;
		spi_clkConfig.Direction = GPIO_DIR_OUT;
		spi_clkConfig.Hysteresis = GPIO_HYSTERESIS_DISABLE;
		spi_clkConfig.Function = GPIO_FUNCTION_ALT5; // api says this is ignored for setup of gpio.
		spi_clkConfig.OpenDrain = GPIO_OPENDRAIN_DISABLE;
		spi_clkConfig.Loopback = GPIO_LOOPBACK_DISABLE;
		spi_clkConfig.Pull = GPIO_PULL_UP_100K;  // Weak pull up?  Maybe need stronger.  CLK should idle high.
		spi_clkConfig.Slew = GPIO_SLEW_FAST;
		spi_clkConfig.Drive = GPIO_DRIVE_34_OHM;
		if (!GpioSetConfig(&spi_clkConfig)) {
			// do error thing
		}

		// FPGA Done is GPIO Port 5, pin 20
		fpga_done.Port = GPIO_PORT_5;
		fpga_done.Pin = GPIO_PIN_20;
		fpga_done.Value = 0;
		fpga_doneConfig.Direction = GPIO_DIR_IN;
		fpga_doneConfig.Hysteresis = GPIO_HYSTERESIS_DISABLE;
		fpga_doneConfig.Function = GPIO_FUNCTION_ALT5;
		fpga_doneConfig.OpenDrain = GPIO_OPENDRAIN_DISABLE; 
		fpga_doneConfig.Loopback = GPIO_LOOPBACK_DISABLE;
		fpga_doneConfig.Pull = GPIO_PULL_UP_100K; // weak pull.  I want the fpga to drive this low.
		fpga_doneConfig.Slew = GPIO_SLEW_FAST;
		fpga_doneConfig.Drive = GPIO_DRIVE_DISABLED;  // this is an input, don't drive it.
		if (!GpioSetConfig(&fpga_doneConfig)) {
			// do error thing
		}

		// FPGA Reset is GPIO port 5, pin 21
		fpga_reset.Port = GPIO_PORT_5;
		fpga_reset.Pin = GPIO_PIN_21;
		fpga_reset.Value = 1; 
		fpga_resetConfig.Direction = GPIO_DIR_OUT;
		fpga_resetConfig.Hysteresis = GPIO_HYSTERESIS_DISABLE;
		fpga_resetConfig.Function = GPIO_FUNCTION_ALT5;
		fpga_resetConfig.OpenDrain = GPIO_OPENDRAIN_DISABLE;
		fpga_resetConfig.Loopback = GPIO_LOOPBACK_DISABLE;
		fpga_resetConfig.Pull = GPIO_PULL_UP_47K;
		fpga_resetConfig.Slew = GPIO_SLEW_FAST;
		fpga_resetConfig.Drive = GPIO_DRIVE_34_OHM;
		if (!GpioSetConfig(&fpga_resetConfig)) {
			// do error thing
		}
	}

	// load the file

	// pass the array to the feeder

	// check FPGA status
	
	// clean up

	return 0;

}
