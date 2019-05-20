/** ----------------------------------------------------------------------------
 *
 * Basler dart BCON for LVDS Development Kit
 * http://www.baslerweb.com
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2017-2018, Basler AG
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * -----------------------------------------------------------------------------
 *
 * @file    bcongrab.cpp
 *
 * @brief   BCON Grab sample
 *
 * @author  Rüdiger Köpke
 *
 * @date    16.03.2017
 *
 * @copyright (c) 2017-2018, Basler AG
 *
 * @license BSD 3-Clause License
 */


// BCON Grab sample
/*
    Note: Before getting started, Basler recommends reading the Programmer's Guide topic
    in the pylon C++ API documentation that gets installed with pylon.
    If you are upgrading to a higher major version of pylon, Basler also
    strongly recommends reading the Migration topic in the pylon C++ API documentation.

    This sample illustrates how to grab and process images using the CInstantCamera class.
    The images are grabbed and processed asynchronously, i.e.,
    while the application is processing a buffer, the acquisition of the next buffer is done
    in parallel.

    The CInstantCamera class uses a pool of buffers to retrieve image data
    from the camera device. Once a buffer is filled and ready,
    the buffer can be retrieved from the camera object for processing. The buffer
    and additional image data are collected in a grab result. The grab result is
    held by a smart pointer after retrieval. The buffer is automatically reused
    when explicitly released or when the smart pointer object is destroyed.
*/

// Include files to use the PYLON API.
#include <pylon/PylonIncludes.h>

// Include local header for instant camera hardware trigger configuration.
#include "HardwareTriggerConfiguration.h"

// Include local header for BCON hardware trigger configuration.
#include "BconTriggerGenerator.h"

// Include header to use the BCON control.
#include <basler/bconctl.h>

#include <unistd.h>    // for getopt()

// Namespace for using pylon objects.
using namespace Pylon;

// Namespace for using cout.
using namespace std;

/******************************************************************************/
// Set the 3 board user LEDs to the binary presentation of num
static void set_leds(int num)
{
    int result;
    result = num & (1 << 0) ? bconctl_board_led_on(bconctl_led_user0) : bconctl_board_led_off(bconctl_led_user0);
    result = num & (1 << 1) ? bconctl_board_led_on(bconctl_led_user1) : bconctl_board_led_off(bconctl_led_user1);
    result = num & (1 << 2) ? bconctl_board_led_on(bconctl_led_user2) : bconctl_board_led_off(bconctl_led_user2);
    PYLON_UNUSED(result);
}

/******************************************************************************/
int main(int argc, char* argv[])
{
    // The exit code of the sample application.
    int exitCode = EXIT_SUCCESS;

    // Number of images to be grabbed.
    uint32_t countOfImagesToGrab = 100;

    // Use the trigger generator in the FPGA to trigger the camera with 10 fps default.
    BconTriggerGenerator trggen;
    int period_ms = 100;
    int duration_ms = 50;

    bool waitForEnterOnExit = false;

    /* Process command line arguments */
    int opt;
    while ((opt = getopt(argc, argv, "c:p:d:wh")) != -1) {
        switch (opt) {
        case 'c':
            countOfImagesToGrab = atoi(optarg);
            break;
        case 'p':
            period_ms = atoi(optarg);
            break;
        case 'd':
            duration_ms = atoi(optarg);
            break;
        case 'w':
            waitForEnterOnExit = true;
            break;
        default:
            exitCode = EXIT_FAILURE;
            // no break here!
        case 'h':
            fprintf(stderr, "Basler dart BCON for LVDS Development Kit\n");
            fprintf(stderr, "Grab images using the trigger generator in the FPGA.\n");
            fprintf(stderr, "Usage: %s [options]\n", argv[0]);
            fprintf(stderr, "  Options:\n");
            fprintf(stderr, "    -c image_cnt  count of images to grab (default: %d)\n", countOfImagesToGrab);
            fprintf(stderr, "    -p period     period of the trigger signal in ms (default: %d)\n", period_ms);
            fprintf(stderr, "    -d duration   duration of the active pulse trigger signal in ms (default: %d)\n", duration_ms);
            fprintf(stderr, "    -w            wait for ENTER on exit\n");
            fprintf(stderr, "    -h            this usage help\n\n");
            exit(exitCode);
        }
    }

    cout << "Grab " << countOfImagesToGrab << " images ";
    cout << "using trigger generator: period=" << period_ms << " ms, duration=" << duration_ms << " ms" << endl;
    if (!trggen.Start(period_ms, duration_ms))
        return EXIT_FAILURE;

    // Before using any pylon methods, the pylon runtime must be initialized.
    PylonInitialize();

    try
    {
        // Create an instant camera object with the camera device found first.
        CInstantCamera camera(CTlFactory::GetInstance().CreateFirstDevice());

        // Print the model name of the camera.
        cout << "Using device " << camera.GetDeviceInfo().GetModelName() << endl;

        set_leds(0);

        // Register the standard configuration event handler for enabling hardware triggering.
        // The hardware trigger configuration handler replaces the default configuration
        // as all currently registered configuration handlers are removed by setting the registration mode to RegistrationMode_ReplaceAll.
        camera.RegisterConfiguration( new CHardwareTriggerConfiguration("RisingEdge"), RegistrationMode_ReplaceAll, Cleanup_Delete);

        // The parameter MaxNumBuffer can be used to control the count of buffers
        // allocated for grabbing. The default value of this parameter is 10.
        camera.MaxNumBuffer = 5;

        // Start the grabbing of countOfImagesToGrab images.
        camera.StartGrabbing(countOfImagesToGrab);

        // This smart pointer will receive the grab result data.
        CGrabResultPtr ptrGrabResult;

        // Camera.StopGrabbing() is called automatically by the RetrieveResult() method
        // when countOfImagesToGrab images have been retrieved.
        while (camera.IsGrabbing())
        {
            // Wait for an image and then retrieve it. A timeout of 5000 ms is used.
            camera.RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);

            // Image grabbed successfully?
            if (ptrGrabResult->GrabSucceeded())
            {
                set_leds(ptrGrabResult->GetID());

                // Access the image data.
                cout << ptrGrabResult->GetID() << ". Size: " << ptrGrabResult->GetWidth() << "x" << ptrGrabResult->GetHeight() << ", ";
                const uint8_t *pImageBuffer = (uint8_t *) ptrGrabResult->GetBuffer();
                cout << "Gray value of first pixel: " << (uint32_t) pImageBuffer[0] << endl;
            }
            else
            {
                cerr << "Error: " << ptrGrabResult->GetErrorCode() << " " << ptrGrabResult->GetErrorDescription() << endl;
            }
        }
    }
    catch (const GenericException &e)
    {
        // Error handling.
        cerr << "An exception occurred." << endl << e.GetDescription() << endl;
        exitCode = EXIT_FAILURE;
    }

    cout << endl << "Attention: The camera is still in TriggerMode 'On'." << endl;
    cout << "  For further proceeding you may change this by using" << endl;
    cout << "  the pylon Viewer or bconctl reset" << endl << endl;

    if (waitForEnterOnExit)
    {
        cerr << endl << "Press Enter to exit." << endl;
        while (cin.get() != '\n');
    }

    set_leds(0);

    // Releases all pylon resources.
    PylonTerminate();

    if (!trggen.Stop())
        exitCode = EXIT_FAILURE;

    return exitCode;
}

