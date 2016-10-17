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
#include <tchar.h>
#include <iostream>
#include "resource.h"
#include <gpiosdk.h>  // This header is in the GuruCE SDK.

#define PADDING_ZEROS 14

// Notes
//  I might benefit from using QueryPerformanceCounter to create a more precision sleep.
//  Is it better to look for a file in the OS filesystem, or to build-in a resource?
// /Notes

int wmain(int argc, wchar_t *argv[])
{
	int err = 0;
	unsigned char *fpga_data;
	unsigned long rsrc_error = ERROR_SUCCESS; // error_success aka zero, represented as a ulong.
	HRSRC resource_handle = FindResource(NULL, MAKEINTRESOURCE(IDR_BINARY1), RT_RCDATA);
	if (!resource_handle) {
		err = GetLastError();
		std::cout << "Couldn't get resource handle, error #" << err << "." << std::endl;
		return 1;
	}
	HGLOBAL mem_handle = LoadResource(NULL, resource_handle);
	if (!mem_handle) {
		err = GetLastError();
		std::cout << "Couldn't allocate memory handle.  Error # " << err << "." << std::endl;
		return 1;
	}
	unsigned long fpga_size = SizeofResource(NULL, resource_handle);
	if (fpga_size == 0) {
		err = GetLastError();
		std::cout << "Sorry Cap, the FPGA resource size came back zero.  We're sunk!  I got error # "
			<< err << "." << std::endl;
		return 1;
	}
	void *locked_memory = LockResource(mem_handle);
	if (!locked_memory) {
		err = GetLastError();
		std::cout << "Could not lock resource handle.  It's over Cap.  Error #" << err << "." << std::endl;
		return 1;
	}
	// So, fpga_size is the actual size of our fpga bitmap, but to program, we actually need to pad it.
	//  The ICE40 wants 8 leading zeros, and ... bizarrely 100(!) trailing zeros.
	//  Since it doesn't add up evenly, I'm going with 14 extra bytes total FPGA size.  That's 112 total extra zeros.
	fpga_data = new unsigned char[fpga_size + PADDING_ZEROS];
	if (!fpga_data) {
		err = GetLastError();
		std::cout << "Something bad happened while allocating the fpga data array.  Error #" << err << std::endl;
		return 1;
	}

	// Zero the fpga data container, then copy the locked fpga data memory into it, offset by 1 so we keep our
	//  8 leading zeros, for size less the 8 bits we offset for (so we don't overrun by 8 bits).
	memset(fpga_data, 0, fpga_size);
	memcpy(fpga_data + 1, locked_memory, fpga_size);

	// maybe do some kind of check against the integrity of the read here?

	printf("This is the FPGA loader.\n");

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

	// load the file            <-------------------------------------------------TODO




	// fpga init dance - put the fpga in programming mode.
	printf("Performing FPGA programming incantation:\n");
	fpga_reset.Value = 0;
	fpga_cs.Value = 0;
	if (!GpioWritePin(&fpga_reset))
	{
		std::cout << "fpga reset pin clear failed.\n";
	}
	if (!GpioWritePin(&fpga_cs))
	{
		std::cout << "Chip select assert failed.\n";
	}
	printf("Chip Select low, chip reset low, clock high, wait 200 ns (minimum).\r\n");  // Need a shorter sleep.  1ms will do though.
	Sleep(1);
	fpga_reset.Value = 1;
	if (!GpioWritePin(&fpga_reset))
	{
		std::cout << "fpga reset pin did not set high.\r\n";
	}
	printf("Set fpga reset high.  Hold for 2ms.\n");
	Sleep(2);  // The Ice40 datasheet says 800 usec in one place, and 1500 usec in another.  Our observation is you need 2ms before writing.

	// pass the array to the feeder & start writing.    <-------------------------TODO




	// check FPGA DONE-ness.
	// gpioreadpin returns true on read success, or false on read fail, and modifies gpiopin.Value to the appropriate state.
	if (!GpioReadPin(&fpga_done)) {
		err = GetLastError();
		std::cout << "Could not read fpga_done.  GetLastError() =" << err << std::endl;
		// handle error?
	}
	// iCE40HX4K Handbook says CDONE will assert high when programming is completed.
	if (fpga_done.Value != 1) 
		std::cout << "fpga_done signal returned low.  FPGA configuration was not successful." << std::endl;
	else 
		std::cout << "fpga_done signal returned high.  The FPGA configuration is complete." << std::endl;

	
	// clean up

	return 0;

}
