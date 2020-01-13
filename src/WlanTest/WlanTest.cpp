// From https://docs.microsoft.com/en-us/windows/win32/api/wlanapi/nf-wlanapi-wlanqueryinterface

#include <windows.h>
#include <wlanapi.h>
#include <Windot11.h>           // for DOT11_SSID struct
#include <objbase.h>
#include <wtypes.h>

//#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>

// Need to link with Wlanapi.lib and Ole32.lib
#pragma comment(lib, "wlanapi.lib")
#pragma comment(lib, "ole32.lib")

int main()
{
    // Declare and initialize variables.

    HANDLE hClient = NULL;
    DWORD dwMaxClient = 2;      //    
    DWORD dwCurVersion = 0;
    DWORD dwResult = 0;
    DWORD dwRetVal = 0;
    int iRet = 0;

    WCHAR GuidString[39] = { 0 };

    unsigned int i, k;

    // variables used for WlanEnumInterfaces

    PWLAN_INTERFACE_INFO_LIST pIfList = NULL;
    PWLAN_INTERFACE_INFO pIfInfo = NULL;

    // variables used for WlanQueryInterfaces for opcode = wlan_intf_opcode_current_connection
    PWLAN_CONNECTION_ATTRIBUTES pConnectInfo = NULL;
    DWORD connectInfoSize = sizeof(WLAN_CONNECTION_ATTRIBUTES);
    WLAN_OPCODE_VALUE_TYPE opCode = wlan_opcode_value_type_invalid;

    dwResult = WlanOpenHandle(dwMaxClient, NULL, &dwCurVersion, &hClient);
    if (dwResult != ERROR_SUCCESS) {
        wprintf(L"WlanOpenHandle failed with error: %u\n", dwResult);
        return 1;
        // You can use FormatMessage here to find out why the function failed
    }

    dwResult = WlanEnumInterfaces(hClient, NULL, &pIfList);
    if (dwResult != ERROR_SUCCESS) {
        wprintf(L"WlanEnumInterfaces failed with error: %u\n", dwResult);
        return 1;
        // You can use FormatMessage here to find out why the function failed
    }
    else {
        wprintf(L"Num Entries: %lu\n", pIfList->dwNumberOfItems);
        wprintf(L"Current Index: %lu\n", pIfList->dwIndex);
        for (i = 0; i < (int)pIfList->dwNumberOfItems; i++) {
            pIfInfo = (WLAN_INTERFACE_INFO*)&pIfList->InterfaceInfo[i];
            wprintf(L"  Interface Index[%u]:\t %lu\n", i, i);
            iRet =
                StringFromGUID2(pIfInfo->InterfaceGuid, (LPOLESTR)&GuidString,
                    sizeof(GuidString) / sizeof(*GuidString));
            // For c rather than C++ source code, the above line needs to be
            // iRet = StringFromGUID2(&pIfInfo->InterfaceGuid, (LPOLESTR) &GuidString, 
            //     sizeof(GuidString)/sizeof(*GuidString)); 
            if (iRet == 0)
                wprintf(L"StringFromGUID2 failed\n");
            else {
                wprintf(L"  InterfaceGUID[%d]:\t %ws\n", i, GuidString);
            }
            wprintf(L"  Interface Description[%d]: %ws", i, pIfInfo->strInterfaceDescription);
            wprintf(L"\n");
            wprintf(L"  Interface State[%d]:\t ", i);
            switch (pIfInfo->isState) {
            case wlan_interface_state_not_ready:
                wprintf(L"Not ready\n");
                break;
            case wlan_interface_state_connected:
                wprintf(L"Connected\n");
                break;
            case wlan_interface_state_ad_hoc_network_formed:
                wprintf(L"First node in a ad hoc network\n");
                break;
            case wlan_interface_state_disconnecting:
                wprintf(L"Disconnecting\n");
                break;
            case wlan_interface_state_disconnected:
                wprintf(L"Not connected\n");
                break;
            case wlan_interface_state_associating:
                wprintf(L"Attempting to associate with a network\n");
                break;
            case wlan_interface_state_discovering:
                wprintf(L"Auto configuration is discovering settings for the network\n");
                break;
            case wlan_interface_state_authenticating:
                wprintf(L"In process of authenticating\n");
                break;
            default:
                wprintf(L"Unknown state %ld\n", pIfInfo->isState);
                break;
            }
            wprintf(L"\n");

            // If interface state is connected, call WlanQueryInterface
            // to get current connection attributes
            if (pIfInfo->isState == wlan_interface_state_connected) {
                dwResult = WlanQueryInterface(hClient,
                    &pIfInfo->InterfaceGuid,
                    wlan_intf_opcode_current_connection,
                    NULL,
                    &connectInfoSize,
                    (PVOID*)&pConnectInfo,
                    &opCode);

                if (dwResult != ERROR_SUCCESS) {
                    wprintf(L"WlanQueryInterface failed with error: %u\n", dwResult);
                    dwRetVal = 1;
                    // You can use FormatMessage to find out why the function failed
                }
                else {
                    wprintf(L"  WLAN_CONNECTION_ATTRIBUTES for this interface\n");

                    wprintf(L"  Interface State:\t ");
                    switch (pConnectInfo->isState) {
                    case wlan_interface_state_not_ready:
                        wprintf(L"Not ready\n");
                        break;
                    case wlan_interface_state_connected:
                        wprintf(L"Connected\n");
                        break;
                    case wlan_interface_state_ad_hoc_network_formed:
                        wprintf(L"First node in a ad hoc network\n");
                        break;
                    case wlan_interface_state_disconnecting:
                        wprintf(L"Disconnecting\n");
                        break;
                    case wlan_interface_state_disconnected:
                        wprintf(L"Not connected\n");
                        break;
                    case wlan_interface_state_associating:
                        wprintf(L"Attempting to associate with a network\n");
                        break;
                    case wlan_interface_state_discovering:
                        wprintf
                        (L"Auto configuration is discovering settings for the network\n");
                        break;
                    case wlan_interface_state_authenticating:
                        wprintf(L"In process of authenticating\n");
                        break;
                    default:
                        wprintf(L"Unknown state %ld\n", pIfInfo->isState);
                        break;
                    }

                    wprintf(L"  Connection Mode:\t ");
                    switch (pConnectInfo->wlanConnectionMode) {
                    case wlan_connection_mode_profile:
                        wprintf(L"A profile is used to make the connection\n");
                        break;
                    case wlan_connection_mode_temporary_profile:
                        wprintf(L"A temporary profile is used to make the connection\n");
                        break;
                    case wlan_connection_mode_discovery_secure:
                        wprintf(L"Secure discovery is used to make the connection\n");
                        break;
                    case wlan_connection_mode_discovery_unsecure:
                        wprintf(L"Unsecure discovery is used to make the connection\n");
                        break;
                    case wlan_connection_mode_auto:
                        wprintf
                        (L"connection initiated by wireless service automatically using a persistent profile\n");
                        break;
                    case wlan_connection_mode_invalid:
                        wprintf(L"Invalid connection mode\n");
                        break;
                    default:
                        wprintf(L"Unknown connection mode %ld\n",
                            pConnectInfo->wlanConnectionMode);
                        break;
                    }

                    wprintf(L"  Profile name used:\t %ws\n", pConnectInfo->strProfileName);

                    wprintf(L"  Association Attributes for this connection\n");
                    wprintf(L"    SSID:\t\t ");
                    if (pConnectInfo->wlanAssociationAttributes.dot11Ssid.uSSIDLength == 0)
                        wprintf(L"\n");
                    else {
                        for (k = 0;
                            k < pConnectInfo->wlanAssociationAttributes.dot11Ssid.uSSIDLength;
                            k++) {
                            wprintf(L"%c",
                                (int)pConnectInfo->wlanAssociationAttributes.dot11Ssid.
                                ucSSID[k]);
                        }
                        wprintf(L"\n");
                    }

                    wprintf(L"    BSS Network type:\t ");
                    switch (pConnectInfo->wlanAssociationAttributes.dot11BssType) {
                    case dot11_BSS_type_infrastructure:
                        wprintf(L"Infrastructure\n");
                        break;
                    case dot11_BSS_type_independent:
                        wprintf(L"Infrastructure\n");
                        break;
                    default:
                        wprintf(L"Other = %lu\n",
                            pConnectInfo->wlanAssociationAttributes.dot11BssType);
                        break;
                    }

                    wprintf(L"    MAC address:\t ");
                    for (k = 0; k < sizeof(pConnectInfo->wlanAssociationAttributes.dot11Bssid);
                        k++) {
                        if (k == 5)
                            wprintf(L"%.2X\n",
                                pConnectInfo->wlanAssociationAttributes.dot11Bssid[k]);
                        else
                            wprintf(L"%.2X-",
                                pConnectInfo->wlanAssociationAttributes.dot11Bssid[k]);
                    }

                    wprintf(L"    PHY network type:\t ");
                    switch (pConnectInfo->wlanAssociationAttributes.dot11PhyType) {
                    case dot11_phy_type_fhss:
                        wprintf(L"Frequency-hopping spread-spectrum (FHSS)\n");
                        break;
                    case dot11_phy_type_dsss:
                        wprintf(L"Direct sequence spread spectrum (DSSS)\n");
                        break;
                    case dot11_phy_type_irbaseband:
                        wprintf(L"Infrared (IR) baseband\n");
                        break;
                    case dot11_phy_type_ofdm:
                        wprintf(L"Orthogonal frequency division multiplexing (OFDM)\n");
                        break;
                    case dot11_phy_type_hrdsss:
                        wprintf(L"High-rate DSSS (HRDSSS) = \n");
                        break;
                    case dot11_phy_type_erp:
                        wprintf(L"Extended rate PHY type\n");
                        break;
                    case dot11_phy_type_ht:
                        wprintf(L"802.11n PHY type\n");
                        break;
                    case dot11_phy_type_vht:
                        wprintf(L"802.11ac PHY type\n");
                        break;
                    case dot11_phy_type_dmg:
                        wprintf(L"802.11ad PHY type\n");
                        break;
                    case dot11_phy_type_he:
                        wprintf(L"802.11ax PHY type\n");
                        break;
                    default:
                        wprintf(L"Unknown = %lu\n",
                            pConnectInfo->wlanAssociationAttributes.dot11PhyType);
                        break;
                    }

                    wprintf(L"    PHY index:\t\t %u\n",
                        pConnectInfo->wlanAssociationAttributes.uDot11PhyIndex);

                    wprintf(L"    Signal Quality:\t %d\n",
                        pConnectInfo->wlanAssociationAttributes.wlanSignalQuality);

                    wprintf(L"    Receiving Rate:\t %ld\n",
                        pConnectInfo->wlanAssociationAttributes.ulRxRate);

                    wprintf(L"    Transmission Rate:\t %ld\n",
                        pConnectInfo->wlanAssociationAttributes.ulTxRate);
                    wprintf(L"\n");

                    wprintf(L"  Security Attributes for this connection\n");

                    wprintf(L"    Security enabled:\t ");
                    if (pConnectInfo->wlanSecurityAttributes.bSecurityEnabled == 0)
                        wprintf(L"No\n");
                    else
                        wprintf(L"Yes\n");

                    wprintf(L"    802.1X enabled:\t ");
                    if (pConnectInfo->wlanSecurityAttributes.bOneXEnabled == 0)
                        wprintf(L"No\n");
                    else
                        wprintf(L"Yes\n");

                    wprintf(L"    Authentication Algorithm: ");
                    switch (pConnectInfo->wlanSecurityAttributes.dot11AuthAlgorithm) {
                    case DOT11_AUTH_ALGO_80211_OPEN:
                        wprintf(L"802.11 Open\n");
                        break;
                    case DOT11_AUTH_ALGO_80211_SHARED_KEY:
                        wprintf(L"802.11 Shared\n");
                        break;
                    case DOT11_AUTH_ALGO_WPA:
                        wprintf(L"WPA\n");
                        break;
                    case DOT11_AUTH_ALGO_WPA_PSK:
                        wprintf(L"WPA-PSK\n");
                        break;
                    case DOT11_AUTH_ALGO_WPA_NONE:
                        wprintf(L"WPA-None\n");
                        break;
                    case DOT11_AUTH_ALGO_RSNA:
                        wprintf(L"RSNA\n");
                        break;
                    case DOT11_AUTH_ALGO_RSNA_PSK:
                        wprintf(L"RSNA with PSK\n");
                        break;
                    default:
                        wprintf(L"Other (%lu)\n", pConnectInfo->wlanSecurityAttributes.dot11AuthAlgorithm);
                        break;
                    }

                    wprintf(L"    Cipher Algorithm:\t ");
                    switch (pConnectInfo->wlanSecurityAttributes.dot11CipherAlgorithm) {
                    case DOT11_CIPHER_ALGO_NONE:
                        wprintf(L"None\n");
                        break;
                    case DOT11_CIPHER_ALGO_WEP40:
                        wprintf(L"WEP-40\n");
                        break;
                    case DOT11_CIPHER_ALGO_TKIP:
                        wprintf(L"TKIP\n");
                        break;
                    case DOT11_CIPHER_ALGO_CCMP:
                        wprintf(L"CCMP\n");
                        break;
                    case DOT11_CIPHER_ALGO_WEP104:
                        wprintf(L"WEP-104\n");
                        break;
                    case DOT11_CIPHER_ALGO_WEP:
                        wprintf(L"WEP\n");
                        break;
                    default:
                        wprintf(L"Other (0x%x)\n", pConnectInfo->wlanSecurityAttributes.dot11CipherAlgorithm);
                        break;
                    }
                    wprintf(L"\n");
                }
            }
        }

    }
    if (pConnectInfo != NULL) {
        WlanFreeMemory(pConnectInfo);
        pConnectInfo = NULL;
    }

    if (pIfList != NULL) {
        WlanFreeMemory(pIfList);
        pIfList = NULL;
    }

    DWORD bufferSize = 100;
    wchar_t lpBuffer[100] = { 0 };
    GetComputerName(lpBuffer, &bufferSize);
    wprintf(L"\n %s", lpBuffer);

    wchar_t lpBuffer2[100] = { 0 };
    bufferSize = 100;
    GetUserName(lpBuffer2, &bufferSize);
    wprintf(L"\n %s", lpBuffer2);

    return dwRetVal;
}