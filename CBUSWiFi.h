/*
   CBUS Module Library - RasberryPi Pico SDK port
   Copyright (c) Kevin Kimber 2024

   Based on work by Duncan Greenwood
   Copyright (c) Duncan Greenwood 2017 (duncan_greenwood@hotmail.com)

   This work is licensed under the:
      Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
   To view a copy of this license, visit:
      http://creativecommons.org/licenses/by-nc-sa/4.0/
   or send a letter to Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.

   License summary:
    You are free to:
      Share, copy and redistribute the material in any medium or format
      Adapt, remix, transform, and build upon the material

    The licensor cannot revoke these freedoms as long as you follow the license terms.

    Attribution : You must give appropriate credit, provide a link to the license,
                  and indicate if changes were made. You may do so in any reasonable manner,
                  but not in any way that suggests the licensor endorses you or your use.

    NonCommercial : You may not use the material for commercial purposes. **(see note below)

    ShareAlike : If you remix, transform, or build upon the material, you must distribute
                 your contributions under the same license as the original.

    No additional restrictions : You may not apply legal terms or technological measures that
                                 legally restrict others from doing anything the license permits.

   ** For commercial use, please contact the original copyright holder(s) to agree licensing terms

    This software is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE

*/

#pragma once

#include <cstdint>
#include "ini.h"

///
/// Type to hold configuration info obtained from the INI
///

typedef struct
{
   const char *country; ///< WiFi country code
   const char *ssid;    ///< WiFi router or AP SSID
   const char *passwd;  ///< WiFi router or AP Password
   bool wpaAuth;        ///< True = Authentication is using WPA
   bool wpa2Auth;       ///< True = Authentication is using WPA2
   bool gcEnable;       ///< True = Grid Connect Server is enabled
   uint16_t gcPort;     ///< Grid Connect Server port
} config_t;

//
/// Class to provide WiFi connectivity to CBUS
//

class CBUSWiFi
{
public:
   CBUSWiFi();

   static bool ReadConfiguration(void);

   bool InitializeClient(void);
   bool InitializeAP(void);
   bool InitWebServer(void);

   // Inline Static Methods

   ///
   /// @brief Determine if the Grid Connect server should be enabled 
   /// 
   /// @return true the Grid Connect server should be enabled
   /// @return false the Grid Connect server should not be enabled
   ///
   static bool isGridConnectEnabled(void) { return m_config.gcEnable; };

   ///
   /// @brief Get the Grid Connect server port
   /// 
   /// @return uint16_t TCP port number to use for the Grid Connect server
   ///
   static uint16_t getGridConnectPort(void) { return m_config.gcPort; };

   ///
   /// @brief Get the Country Code 
   /// 
   /// @return char* Country code as configured in the config INI
   ///
   static const char* getCountryCode(void) { return m_config.country; };

   ///
   /// @brief Get the WPA authentication enable flag
   /// 
   /// @return true WPA authentication is enabled
   /// @return false WPA authentication is disabled
   ///
   static bool getWPAEnable(void) { return m_config.wpaAuth; };

   ///
   /// @brief Get the WPA2 autentication enable flag
   /// 
   /// @return true WPA2 authentication is enabled
   /// @return false WPA2 autentication is disabled
   ///
   static bool getWPA2Enable(void) { return m_config.wpa2Auth; };

private:
   /// Configuration data store
   static inline config_t m_config = {};

   static int parseINI(const char *filename, ini_handler handler, void *user);
};