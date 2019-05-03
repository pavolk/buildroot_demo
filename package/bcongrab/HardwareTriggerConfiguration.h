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
 * @file    HardwareTriggerConfiguration.h
 *
 * @brief   An instant camera configuration for hardware trigger,
 *          This instant camera configuration is provided as header-only file. The code
 *          can be copied and modified for creating own configuration classes.
 *
 * @author  Rüdiger Köpke
 *
 * @date    16.03.2017
 *
 * @copyright (c) 2017-2018, Basler AG
 *
 * @license BSD 3-Clause License
 */


#ifndef INCLUDED_HARDWARETRIGGERCONFIGURATION_H_4655834
#define INCLUDED_HARDWARETRIGGERCONFIGURATION_H_4655834

#include <pylon/Platform.h>

#ifdef _MSC_VER
#   pragma pack(push, PYLON_PACKING)
#endif /* _MSC_VER */

#include <pylon/InstantCamera.h>

namespace Pylon
{
    /** \addtogroup Pylon_InstantCameraApiGeneric
     * @{
     */

    /*!
    \class  CHardwareTriggerConfiguration
    \brief  Changes the configuration of the camera so that the acquisition of frames is triggered by Hardware trigger.

        The %CHardwareTriggerConfiguration is provided as header-only file.
        The code can be copied and modified for creating own configuration classes.
    */
    class CHardwareTriggerConfiguration : public CConfigurationEventHandler
    {
    public:
        CHardwareTriggerConfiguration() :
            CConfigurationEventHandler(),
            m_TriggerActivation("RisingEdge")
        {};

        CHardwareTriggerConfiguration(const GENICAM_NAMESPACE::gcstring& TriggerActivation) :
            CConfigurationEventHandler(),
            m_TriggerActivation(TriggerActivation)
        {};

        /// Apply hardware trigger configuration.
        void ApplyConfiguration( GENAPI_NAMESPACE::INodeMap& nodemap)
        {
            using namespace GENAPI_NAMESPACE;

            // Disable all trigger types except the trigger type used for triggering the acquisition of
            // frames.
            {
                // Get required enumerations.
                CEnumerationPtr triggerSelector( nodemap.GetNode("TriggerSelector"));
                CEnumerationPtr triggerMode( nodemap.GetNode("TriggerMode"));

                // Check the available camera trigger mode(s) to select the appropriate one: acquisition start trigger mode
                // (used by older cameras, i.e. for cameras supporting only the legacy image acquisition control mode;
                // do not confuse with acquisition start command) or frame start trigger mode
                // (used by newer cameras, i.e. for cameras using the standard image acquisition control mode;
                // equivalent to the acquisition start trigger mode in the legacy image acquisition control mode).
                String_t triggerName( "FrameStart");
                if ( !IsAvailable( triggerSelector->GetEntryByName(triggerName)))
                {
                    triggerName = "AcquisitionStart";
                    if ( !IsAvailable( triggerSelector->GetEntryByName(triggerName)))
                    {
                        throw RUNTIME_EXCEPTION( "Could not select trigger. Neither FrameStart nor AcquisitionStart is available.");
                    }
                }

                // Get all enumeration entries of trigger selector.
                GENAPI_NAMESPACE::NodeList_t triggerSelectorEntries;
                triggerSelector->GetEntries( triggerSelectorEntries );

                // Turn trigger mode off for all trigger selector entries except for the frame trigger given by triggerName.
                for ( GENAPI_NAMESPACE::NodeList_t::iterator it = triggerSelectorEntries.begin(); it != triggerSelectorEntries.end(); ++it)
                {
                    // Set trigger mode to off if the trigger is available.
                    GENAPI_NAMESPACE::CEnumEntryPtr pEntry(*it);
                    if ( IsAvailable( pEntry))
                    {
                        String_t triggerNameOfEntry( pEntry->GetSymbolic());
                        triggerSelector->FromString( triggerNameOfEntry);
                        if ( triggerName == triggerNameOfEntry)
                        {
                            // Activate trigger.
                            triggerMode->FromString( "On");

                            //// The camera user's manual contains more information about available configurations.
                            //// The Basler pylon Viewer tool can be used to test the selected settings first.

                            // The trigger source must be set to the trigger input, e.g. 'Line3'.
                            CEnumerationPtr(nodemap.GetNode("TriggerSource"))->FromString("Line3");

                            // The trigger activation must be set to 'RisingEdge' or 'FallingEdge'.
                            CEnumerationPtr(nodemap.GetNode("TriggerActivation"))->FromString(m_TriggerActivation);
                        }
                        else
                        {
                            triggerMode->FromString( "Off");
                        }
                    }
                }
                // Finally select the frame trigger type (resp. acquisition start type
                // for older cameras). Issuing a hardware trigger will now trigger
                // the acquisition of a frame.
                triggerSelector->FromString(triggerName);
            }


            //Set acquisition mode to "continuous"
            CEnumerationPtr(nodemap.GetNode("AcquisitionMode"))->FromString("Continuous");
        }

        //Set basic camera settings.
        virtual void OnOpened( CInstantCamera& camera)
        {
            try
            {
                ApplyConfiguration( camera.GetNodeMap());
            }
            catch (const GenericException& e)
            {
                throw RUNTIME_EXCEPTION( "Could not apply configuration. Pylon::GenericException caught in OnOpened method msg=%hs", e.what());
            }
            catch (const std::exception& e)
            {
                throw RUNTIME_EXCEPTION( "Could not apply configuration. std::exception caught in OnOpened method msg=%hs", e.what());
            }
            catch (...)
            {
                throw RUNTIME_EXCEPTION( "Could not apply configuration. Unknown exception caught in OnOpened method.");
            }
        }

    private:
        GENICAM_NAMESPACE::gcstring m_TriggerActivation;
    };

    /**
     * @}
     */
}

#ifdef _MSC_VER
#   pragma pack(pop)
#endif /* _MSC_VER */

#endif /* INCLUDED_HARDWARETRIGGERCONFIGURATION_H_4655834 */
