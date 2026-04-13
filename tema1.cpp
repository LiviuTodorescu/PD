#include <windows.h>
#include <iostream>
#include <tchar.h>

constexpr DWORD kValueNameBufferSize = 64;
constexpr DWORD kValueDataBufferSize = 256;

void PrintValueDetails(DWORD valueType, const BYTE* valueData) {
    switch (valueType) {
    case REG_SZ:
        _tprintf(L"\tType: REG_SZ\n\tValue: %ls", reinterpret_cast<const TCHAR*>(valueData));
        return;
    case REG_EXPAND_SZ:
        _tprintf(L"\tType: REG_EXPAND_SZ\n\tValue: %ls", reinterpret_cast<const TCHAR*>(valueData));
        return;
    case REG_DWORD:
        _tprintf(L"\tType: REG_DWORD\n\tValue: %lu\n", *reinterpret_cast<const DWORD*>(valueData));
        return;
    case REG_MULTI_SZ:
        _tprintf(L"\tType: REG_MULTI_SZ");
        return;
    case REG_BINARY:
        _tprintf(L"\tType: REG_BINARY");
        return;
    default:
        _tprintf(L"\tUnknown Type");
        return;
    }
}

void ListRegistryValues(LPCTSTR subKeyPath) {
    HKEY keyHandle = nullptr;
    const LONG openResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, subKeyPath, 0, KEY_READ, &keyHandle);
    if (openResult != ERROR_SUCCESS) {
        fwprintf(stderr, L"Error: Cannot open the registry key: %ls\n", subKeyPath);
        return;
    }

    DWORD valueCount = 0;
    DWORD longestValueName = 0;
    const LONG infoResult = RegQueryInfoKey(
        keyHandle, NULL, NULL, NULL, NULL, NULL, NULL, &valueCount, &longestValueName, NULL, NULL, NULL);

    if (infoResult != ERROR_SUCCESS) {
        fwprintf(stderr, L"Error querying key information.\n");
        RegCloseKey(keyHandle);
        return;
    }

    if (valueCount == 0) {
        _tprintf(L"The key '%ls' has no values.\n", subKeyPath);
        RegCloseKey(keyHandle);
        return;
    }

    WCHAR valueName[kValueNameBufferSize];
    BYTE valueData[kValueDataBufferSize];

    for (DWORD index = 0; index < valueCount; ++index) {
        DWORD nameLength = kValueNameBufferSize;
        DWORD dataLength = kValueDataBufferSize;
        DWORD valueType = 0;

        const LONG enumResult = RegEnumValue(
            keyHandle, index, valueName, &nameLength, NULL, &valueType, valueData, &dataLength);

        if (enumResult != ERROR_SUCCESS) {
            _tprintf(L"Error getting info about Value no %lu", index);
            continue;
        }

        _tprintf(L"\n\n%lu. %ls\n", index + 1, (nameLength == 0) ? L"(Default)" : valueName);
        PrintValueDetails(valueType, valueData);
    }

    RegCloseKey(keyHandle);
}

int main() {
    const LPCTSTR registryPath = L"SYSTEM\\CurrentControlSet\\Control\\Session Manager";
    ListRegistryValues(registryPath);
    return 0;
}
