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

// I need two functions:
//  Function 1:  Clock out 8 data bits to SPI2_MOSI.
//    Needs GPIO for SPI2_CLK and SPI2_MOSI.
//    Needs an array of 8 bits.
//    Return an error status if one of the GPIO isn't working.
//
//  Function 2:  Feed func. 1 a big array 8 bits at a time.
//    Needs a big a rray full of data to push out.
//    Needs to hold the correct chip select low until done.
//    Also get the GPIO structures going...

int wmain(int argc, wchar_t *argv[])
{
	printf("This is the FPGA loader.  Thanks, I'll take it from here.\n");
	return 0;

}
