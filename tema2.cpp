#include <windows.h>
#include <setupapi.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#pragma comment(lib, "setupapi.lib")

struct DeviceMetaData {
    std::wstring hardwareID;
    std::wstring description;
    std::wstring manufacturer;
    std::wstring friendlyName;
    std::wstring deviceClass;
};

std::wstring ReadDevicePropertyAsString(HDEVINFO devInfoSet, SP_DEVINFO_DATA& deviceInfo, DWORD propertyId) {
    DWORD dataType = 0;
    DWORD requiredSize = 0;

    SetupDiGetDeviceRegistryPropertyW(devInfoSet, &deviceInfo, propertyId, &dataType, NULL, 0, &requiredSize);
    if (requiredSize == 0) {
        return L"";
    }

    std::vector<BYTE> rawBuffer(requiredSize);
    if (SetupDiGetDeviceRegistryPropertyW(
            devInfoSet, &deviceInfo, propertyId, &dataType, rawBuffer.data(), requiredSize, NULL)) {
        return std::wstring(reinterpret_cast<wchar_t*>(rawBuffer.data()));
    }

    return L"";
}

DeviceMetaData BuildDeviceMetaData(HDEVINFO devInfoSet, SP_DEVINFO_DATA& deviceInfo) {
    DeviceMetaData meta;
    meta.hardwareID = ReadDevicePropertyAsString(devInfoSet, deviceInfo, SPDRP_HARDWAREID);
    meta.description = ReadDevicePropertyAsString(devInfoSet, deviceInfo, SPDRP_DEVICEDESC);
    meta.manufacturer = ReadDevicePropertyAsString(devInfoSet, deviceInfo, SPDRP_MFG);
    meta.friendlyName = ReadDevicePropertyAsString(devInfoSet, deviceInfo, SPDRP_FRIENDLYNAME);
    meta.deviceClass = ReadDevicePropertyAsString(devInfoSet, deviceInfo, SPDRP_CLASS);
    return meta;
}

void WriteDeviceBlock(HDEVINFO devInfoSet, SP_DEVINFO_DATA& deviceInfo, std::wofstream& outputFile) {
    const DeviceMetaData meta = BuildDeviceMetaData(devInfoSet, deviceInfo);

    outputFile << L"======\n";
    outputFile << L"Hardware ID: " << (meta.hardwareID.empty() ? L"N/A" : meta.hardwareID) << L"\n";
    outputFile << L"Description: " << (meta.description.empty() ? L"N/A" : meta.description) << L"\n";
    outputFile << L"Manufacturer: " << (meta.manufacturer.empty() ? L"N/A" : meta.manufacturer) << L"\n";
    outputFile << L"Friendly Name: " << (meta.friendlyName.empty() ? L"N/A" : meta.friendlyName) << L"\n";
    outputFile << L"Class: " << (meta.deviceClass.empty() ? L"N/A" : meta.deviceClass) << L"\n";
    outputFile << L"======\n";
}

void HandleDeviceScanning(const std::wstring& inputText) {
    int mode = 0;
    std::wstring busType = inputText;

    // Daca utilizatorul da Enter, folosim direct scanare completa.
    if (busType.empty()) {
        mode = 3;
    } else {
        // Normalizam la majuscule ca sa evitam diferentele de litere mici/mari.
        std::transform(busType.begin(), busType.end(), busType.begin(), ::towupper);

        HDEVINFO validationHandle =
            SetupDiGetClassDevsW(NULL, busType.c_str(), NULL, DIGCF_ALLCLASSES | DIGCF_PRESENT);

        if (validationHandle != INVALID_HANDLE_VALUE) {
            SP_DEVINFO_DATA probeData{};
            probeData.cbSize = sizeof(SP_DEVINFO_DATA);

            // Daca exista cel putin un dispozitiv, pastram scanarea filtrata.
            if (SetupDiEnumDeviceInfo(validationHandle, 0, &probeData)) {
                mode = 1;
            } else {
                mode = 2;
            }

            SetupDiDestroyDeviceInfoList(validationHandle);
        } else {
            mode = 2;
        }
    }

    HDEVINFO deviceHandle = INVALID_HANDLE_VALUE;
    if (mode == 1) {
        deviceHandle = SetupDiGetClassDevsW(NULL, busType.c_str(), NULL, DIGCF_ALLCLASSES | DIGCF_PRESENT);
    } else {
        deviceHandle = SetupDiGetClassDevsW(NULL, NULL, NULL, DIGCF_ALLCLASSES | DIGCF_PRESENT);
    }

    if (deviceHandle == INVALID_HANDLE_VALUE) {
        return;
    }

    std::wofstream outputFile(L"devices_output.txt");
    SP_DEVINFO_DATA deviceInfo{};
    deviceInfo.cbSize = sizeof(SP_DEVINFO_DATA);
    DWORD deviceCount = 0;

    // Parcurgem toate dispozitivele gasite si scriem metadatele in fisier.
    while (SetupDiEnumDeviceInfo(deviceHandle, deviceCount, &deviceInfo)) {
        WriteDeviceBlock(deviceHandle, deviceInfo, outputFile);
        ++deviceCount;
    }

    SetupDiDestroyDeviceInfoList(deviceHandle);
    outputFile.close();

    // Mesajul final depinde de modul ales mai sus.
    if (mode == 1) {
        std::wcout << L"Devices of TYPE: " << busType << L" were read.Number of devices found " << deviceCount << L".\n";
    } else if (mode == 2) {
        std::wcout << L"Devices of TYPE: " << busType
                   << L" not found.Scanning all types of devices.Number of devices found " << deviceCount << L".\n";
    } else if (mode == 3) {
        std::wcout << L"Scanning all types of devices.Number of devices found " << deviceCount << L".\n";
    }
}

int main() {
    std::wstring inputLine;
    // Citim tipul de bus dorit; input gol => scanare pentru toate tipurile.
    std::getline(std::wcin, inputLine);
    HandleDeviceScanning(inputLine);
    return 0;
}
