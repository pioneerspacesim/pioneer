// Source file for any OS-specific code.
// Currently there are only functions for Windows, but
//   functions for other OS's can be added here.

#ifdef WIN32

#include <string>
#include <algorithm> // std::remove

#include "sha1.h" // SHA-1 hashing algorithm
#include "curl/curl.h" // libcurl, for sending http requests
#include <iphlpapi.h>
#pragma comment(lib, "IPHLPAPI.lib")
#include <iptypes.h>

namespace SYSTEM
{

std::string GetGUID(void)
{
    // Get the GUID.
    GUID guid;
    CoCreateGuid(&guid);

    // Turn the GUID into an ASCII string.
    RPC_CSTR str;
    UuidToStringA((UUID*)&guid, &str);

    // Get a standard string out of that.
    std::string result = (LPSTR)str;

    // Remove any hyphens.
    result.erase(std::remove(result.begin(), result.end(), '-'), result.end());

    return result;
}

std::string GetUniqueUserID(void)
{
    std::string result;

    // Prepare an array to get info for up to 16 network adapters.
    // It's a little hacky, but it's way complicated to do otherwise,
    //   and the chances of someone having more than 16 are pretty crazy.
    IP_ADAPTER_INFO info[16];

    // Get info for all the network adapters.
    DWORD dwBufLen = sizeof(info);
    DWORD dwStatus = GetAdaptersInfo(info, &dwBufLen);
    if (dwStatus != ERROR_SUCCESS)
    {
        // ---------------------- //
        // ------INCOMPLETE------ //
        // ---------------------- //
        /* deal with error */
    }
    else
    {
        PIP_ADAPTER_INFO pAdapterInfo = info;

        // Iterate through the adapter array until we find one.
        // (Will most likely be the first.)
        while (pAdapterInfo && pAdapterInfo == 0)
        {
            pAdapterInfo = pAdapterInfo->Next;
        }

        if (!pAdapterInfo)
        {
            // Can't get the network adapter, using a GUID instead.
            result = GetGUID();
        }
        else
        {
            // Get a hash of the MAC address using SHA1.
            unsigned char hash[20];
            char hexstring[41];
            sha1::calc(pAdapterInfo->Address, pAdapterInfo->AddressLength, hash);
            sha1::toHexString(hash, hexstring);
            result = hexstring;

            // Remove any hyphens.
            result.erase(std::remove(result.begin(), result.end(), '-'), result.end());
        }
    }

    return result;
}

}

#endif
