//  ce40Loader.cpp : Defines the entry point for the console application.
//
// c. 2016 Technologic Systems   --- http://www.embeddedarm.com
//  Written by Michael D. Peters
// This program demonstrates a bit-banged approach to loading the bitmap file
//  for the TS-4900 ICE40 FPGA.
// This software requires a 3rd party SDK, visit http://www.guruce.com for the 
//  i.MX6 TS-4900 Windows Embedded Compact 2013 SDK.
//
// Information on the TS-4900 is available at http://www.embeddedarm.com/products/TS-4900

#include "stdafx.h"
#include <windows.h>
#include <tchar.h>
#include <iostream>
#include "resource.h"
#include <gpiosdk.h>  // This header is in the GuruCE SDK.
#include <i2csdk.h>	  // This header is in the GuruCE SDK.
#include <stdint.h>

#define PADDING_ZEROS 14

using std::cout;

// Notes
//  I might benefit from using QueryPerformanceCounter to create a more precision sleep.
//    ^^^^^  Actually *will*.  I tried it without.  Just over 2ms per bit means I *NEED*
//    a shorter delay function than Sleep() provides.
//  Is it better to look for a file in the OS filesystem, or to build-in a resource?
// /Notes

// does i2c using bit-bang technique.
int bangi2c(LPGPIO_CONFIG gpioCfg)
{
	int err = ERROR_SUCCESS;

	
	return err;
}


// Does i2c using the guruCE i2c sdk.
void do_i2c(void)
{
	HANDLE i2cbus = INVALID_HANDLE_VALUE;
	i2cbus = I2COpenHandle(I2C1_FID);

	if (INVALID_HANDLE_VALUE == i2cbus)
	{
		cout << "Could not open I2C driver!\r\n";
	}
	I2C_TRANSFER_BLOCK i2cBlock;
	I2C_PACKET i2cPacket[2];
	BYTE inDat[2], outDat[2];
	BOOL err = FALSE;
	int result1, result2;
	int *pResult = &result2;

	// Do some i2c setup stuff?
	if (!I2CSetMasterMode(i2cbus))
		cout << "Mastermode set returned error status.\r\n";
	else
		cout << "I2C Master Mode set.\r\n";
	I2CSetFrequency(i2cbus, I2C_MAX_FREQUENCY);

	// Addresses need to be 16 bits on i2c.  Little Endian.
	outDat[0] = 0x0; // ask for fpga ID register.
	outDat[1] = 0x51;
	inDat[0] = 0x0; // because I don't know.
	inDat[1] = 0x51;

	// compose the write packet:
	i2cPacket[0].byRW = I2C_RW_WRITE;
	i2cPacket[0].byAddr = 0x28;
	i2cPacket[0].pbyBuf = outDat;
	i2cPacket[0].lpiResult = (LPINT)&result1;
	i2cPacket[0].wLen = sizeof(outDat);

	// compose the read packet:
	i2cPacket[1].byRW = I2C_RW_READ;
	i2cPacket[1].byAddr = 0x28;
	i2cPacket[1].pbyBuf = inDat;
	i2cPacket[1].lpiResult = &result2;
	i2cPacket[1].wLen = sizeof(inDat);

	// set up transaction
	i2cBlock.pI2CPackets = i2cPacket;
	i2cBlock.iNumPackets = _countof(i2cPacket);

	// do the transaction
	if (!I2CTransfer(i2cbus, &i2cBlock))
	{
		printf("I2C:\r\n");
		printf("packet 0 write ID 0x%X outDat 0x%X, result 0x%x\r\n", i2cPacket[0].byAddr, outDat[1], result1);
		printf("packet 1 read ID 0x%X inDat 0x%X, result 0x%X\r\n", i2cPacket[1].byAddr, inDat[1], result2);
		cout << "I2C Transaction(s) complete." << std::endl;
	}
	else
	{
		printf("I2C Transfer failed, %d.\r\n", GetLastError());
	}

	if (INVALID_HANDLE_VALUE != i2cbus)
	{
		I2CCloseHandle(i2cbus);
		cout << "I2C Handle Closed.\r\n";
	}
}


/// hold:  Holds for n/queryperformancefrequency.
void hold(LARGE_INTEGER n) 
{
	LARGE_INTEGER now;
	LARGE_INTEGER end;
	// flyyyyyy me to the moon.
	if (QueryPerformanceCounter(&now)) {
		// let me plaaaaay among the stars.
		end.QuadPart += now.QuadPart + n.QuadPart;
		while (end.QuadPart > now.QuadPart)
			// show me what spring is like
			QueryPerformanceCounter(&now);
			// On a-Juuuuupiter and Mars
	}
}

int wmain(int argc, wchar_t *argv[])
{
	int err = 0;
	LARGE_INTEGER counter;

	// check if we can use queryperformancecounter
	if (!QueryPerformanceFrequency(&counter)) {
		std::cout << "Sorry, no high resolution timers available here.";
		return -5;
	}
	else
		cout << "Yay!  We can do high performance counters!  My counter frequency is " << counter.QuadPart << std::endl;

	///////////////////////////////////////////////////////////////////////////////////////////////
	//                                RESOURCE LOAD BLOCK                                        //
	///////////////////////////////////////////////////////////////////////////////////////////////

	unsigned char *fpga_data;
	unsigned long rsrc_error = ERROR_SUCCESS; // error_success aka zero, represented as a ulong.
	HRSRC resource_handle = FindResource(NULL, MAKEINTRESOURCE(IDR_RCDATA1), RT_RCDATA);
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
	// So, fpga_size is the actual size of our fpga bitmap, but to program, we actually need to pad the transmission.
	//  The ICE40 wants 8 leading zeros, and ... bizarrely 100(!) trailing zeros.
	//  Since it doesn't add up evenly, I'm going with +14 bytes.  That's 112 total extra zeros. 8 to start, 104 to end.
	fpga_data = new unsigned char[fpga_size + PADDING_ZEROS];
	if (!fpga_data) {
		err = GetLastError();
		std::cout << "Something bad happened while allocating the fpga data array.  Error #" << err << std::endl;
		return 1;
	}

	// Zero the fpga data container, then copy the locked fpga data memory into it, offset by 1 so we keep our
	//  8 leading zeros.  Since fpga_size is 14 bytes smaller than fpga_data, we're not worried about overruns.
	memset(fpga_data, 0, fpga_size + PADDING_ZEROS);
	memcpy(fpga_data + 1, locked_memory, fpga_size);

	// maybe do some kind of check against the integrity of the read here?

	///////////////////////////////////////////////////////////////////////////////////////////////
	//                                  GPIO SETUP BLOCK                                         //
	///////////////////////////////////////////////////////////////////////////////////////////////

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

	///////////////////////////////////////////////////////////////////////////////////////////////
	//                  fpga init dance - put the fpga in programming mode.                      //
	///////////////////////////////////////////////////////////////////////////////////////////////

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
	printf("Chip Select low, chip reset low, clock high, wait 200 ns (minimum, 1 ms actual).\r\n");  // 1ms will do.
	Sleep(1);
	fpga_reset.Value = 1;
	if (!GpioWritePin(&fpga_reset))
	{
		std::cout << "fpga reset pin did not set high.\r\n";
	}
	printf("Set fpga reset high.  Hold for 2ms.\n");
	Sleep(2);  // The Ice40 datasheet says 800 usec in one place, and 1500 usec in another.  Our observation is you need 2ms before writing.

	///////////////////////////////////////////////////////////////////////////////////////////////
	//                             BANG OUT ARRAY ON SPI PINS                                    //
	///////////////////////////////////////////////////////////////////////////////////////////////

	printf("Starting SPI send burst.\n");

	int percent, percent_new = 0;
	uint8_t stage, step = 0;
	LARGE_INTEGER hold_time;
	hold_time.QuadPart = 3;  // Hold time is 17/396000000-ths of a second.
	// Step through the data array one byte at a time transmitting data.
	for (int bookmark = 0; bookmark < fpga_size + PADDING_ZEROS; bookmark++) {
		stage = fpga_data[bookmark];

		// MSB first, step across the byte and transmit 1 or 0.
		for (step = 0x80; step; step >>= 1) {
			spi_clk.Value = 0;
			GpioWritePin(&spi_clk);
			if (stage & step) {
				mosi.Value = 1;
			}
			else {
				mosi.Value = 0;
			}
			GpioWritePin(&mosi);
			hold(hold_time);
			spi_clk.Value = 1;
			GpioWritePin(&spi_clk);
			hold(hold_time);
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	//                                      WRAP UP                                              //
	///////////////////////////////////////////////////////////////////////////////////////////////

	// check FPGA DONE-ness.
	// gpioreadpin returns true on read success, or false on read fail, and modifies gpiopin.Value to the appropriate state.
	if (!GpioReadPin(&fpga_done)) {
		err = GetLastError();
		std::cout << "Could not read fpga_done.  GetLastError() =" << err << std::endl;
		// handle error?
	}
	// iCE40HX4K Handbook says CDONE will assert high when programming is completed.
	if (fpga_done.Value != 1) {
		std::cout << "fpga_done signal returned low.  FPGA configuration was not successful." << std::endl;
		err = -2;
	}
	else {
		err = 0;
		std::cout << "fpga_done signal returned high.  The FPGA configuration is complete." << std::endl;
		do_i2c();
	}
	
	// clean up
	delete fpga_data;
	if (!GpioDeinit()) {
		printf("For what it's worth, I probably just leaked GPIO handles because GpioDeinit() returned false.  Error #%d", GetLastError());
		return 1;
	}
	return err;
}
