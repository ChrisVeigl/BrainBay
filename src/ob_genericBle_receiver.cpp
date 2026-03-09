
/* -----------------------------------------------------------------------------

    BrainBay  -  OpenSource Biofeedback Software
    MODULE: OB_GENERICBLE_RECEIVER.CPP

    A simple BLE receiver that interprets incoming packets as IEEE-754
    single-precision floats (4 bytes each). The number of floats in the
    packet defines the number of channels (len/4). If more than 16 channels
    are received these channels are is discarded. 

    Note that this element is a singleton (can only be used once).
    This could be improved by encapsulating global data structures, 
    variables and methods in the object.

    Many thanks to: https://stackoverflow.com/q/67934095

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; See the
    GNU General Public License for more details.

-----------------------------------------------------------------------------*/

#include "brainBay.h"
#include "ob_genericBle_receiver.h"

#pragma once
#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include <queue>
#include <map>
#include <mutex>
#include <condition_variable>
#include <string>
#include <iomanip>
#include <optional>
#include <chrono>

#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Web.Syndication.h>
#include "winrt/Windows.Devices.Bluetooth.h"
#include "winrt/Windows.Devices.Bluetooth.GenericAttributeProfile.h"
#include "winrt/Windows.Devices.Enumeration.h"
#include "winrt/Windows.Storage.Streams.h"
#include <winrt/Windows.Devices.Bluetooth.Advertisement.h>
#pragma comment(lib, "windowsapp")


using namespace std;
using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Web::Syndication;
using namespace Windows::Devices::Bluetooth;
using namespace Windows::Devices::Bluetooth::GenericAttributeProfile;
using namespace Windows::Devices::Enumeration;
using namespace Windows::Storage::Streams;

#pragma region STRUCS AND ENUMS
#define LOG_ERROR(e) cout << e << endl;
#define MAX_CHANNELS 16
// default advertising name, serviceUUID and readCharacteristicUUID

float values[MAX_CHANNELS] = { 0 };
int num_channels = 1;  // default to 1 channel 
int num_samples = 10;  // 10 samples per channel

char ble_name[101] = "ESP32S3-BLE-Sender";  // default BLE device name
char serviceUUID[64] = "12345678-1234-5678-1234-56789abcdef0";
char readUUID[64] = "12345678-1234-5678-1234-56789abcdef1";

int waitTime = 500;     //Wait time in milliseconds between each check
bool hasSubscribed = false;
wstring connectedDeviceId = L"";
int bleThreadDone = 1;
int bleThreadRunning = 0;
DWORD bleStatId = 0;
uint32_t bleSampleCount = 0;

union to_guid
{
    uint8_t buf[16];
    guid guid;
};

const uint8_t BYTE_ORDER[] = { 3, 2, 1, 0, 5, 4, 7, 6, 8, 9, 10, 11, 12, 13, 14, 15 };

guid make_guid(const wchar_t* value)
{
    to_guid to_guid;
    memset(&to_guid, 0, sizeof(to_guid));
    int offset = 0;
    for (unsigned int i = 0; i < wcslen(value); i++) {
        if (value[i] >= '0' && value[i] <= '9')
        {
            uint8_t digit = value[i] - '0';
            to_guid.buf[BYTE_ORDER[offset / 2]] += offset % 2 == 0 ? digit << 4 : digit;
            offset++;
        }
        else if (value[i] >= 'A' && value[i] <= 'F')
        {
            uint8_t digit = 10 + value[i] - 'A';
            to_guid.buf[BYTE_ORDER[offset / 2]] += offset % 2 == 0 ? digit << 4 : digit;
            offset++;
        }
        else if (value[i] >= 'a' && value[i] <= 'f')
        {
            uint8_t digit = 10 + value[i] - 'a';
            to_guid.buf[BYTE_ORDER[offset / 2]] += offset % 2 == 0 ? digit << 4 : digit;
            offset++;
        }
        else
        {
            // skip char
        }
    }

    return to_guid.guid;
}

wstring CharToWString(const char* str) {
    int len = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
    if (len == 0) return L"";
    wstring result(len - 1, L'\0'); // -1 to exclude null terminator
    MultiByteToWideChar(CP_UTF8, 0, str, -1, &result[0], len);
    return result;
}

mutex subscribeLock;
condition_variable subscribeSignal;

mutex _mutexWrite;
condition_variable signalWrite;

struct CharacteristicCacheKey {
    wstring deviceId;
    wstring serviceId;
    wstring characteristicId;

    bool operator<(const CharacteristicCacheKey& other) const {
        if (deviceId != other.deviceId) return deviceId < other.deviceId;
        if (serviceId != other.serviceId) return serviceId < other.serviceId;
        return characteristicId < other.characteristicId;
    }
};

struct DeviceCacheEntry {
    BluetoothLEDevice device = nullptr;
    GattDeviceService service = nullptr;
    GattSession session = nullptr;
};

map<wstring, DeviceCacheEntry> cache;
map<CharacteristicCacheKey, std::optional<GattCharacteristic>> characteristicCache;

struct Subscription {
    GattCharacteristic::ValueChanged_revoker revoker;
};

struct BLEDeviceData {
    wstring id;
    wstring name;
    bool isConnectable = false;
    Subscription* subscription = NULL;
};
vector<BLEDeviceData> deviceList{};

mutex deviceListLock;
condition_variable deviceListSignal;


#pragma endregion

#pragma region CACHE FUNCTIONS

// Invalidate all cached BLE objects for a given device
void clearDeviceCache(const wstring& deviceId) {
    // Remove characteristic cache entries for this device
    auto it = characteristicCache.begin();
    while (it != characteristicCache.end()) {
        if (it->first.deviceId == deviceId)
            it = characteristicCache.erase(it);
        else
            ++it;
    }
    // Close GattSession and device before removing
    if (cache.count(deviceId)) {
        if (cache[deviceId].session != nullptr) {
            cache[deviceId].session.Close();
            cache[deviceId].session = nullptr;
        }
        if (cache[deviceId].service != nullptr) {
            cache[deviceId].service.Close();
            cache[deviceId].service = nullptr;
        }
        if (cache[deviceId].device != nullptr) {
            cache[deviceId].device.Close();
            cache[deviceId].device = nullptr;
        }
    }
    cache.erase(deviceId);
}

//Call this function to get a device from cache or async if it wasn't found.
// On Win11, we also create a GattSession with MaintainConnection=true to force
// the actual BLE radio connection, and wait for ConnectionStatus==Connected.
IAsyncOperation<BluetoothLEDevice> getDevice(wchar_t* deviceId) {
    if (cache.count(wstring(deviceId)) && cache[wstring(deviceId)].device) {
        // Check if still connected
        auto cachedDev = cache[wstring(deviceId)].device;
        if (cachedDev.ConnectionStatus() == BluetoothConnectionStatus::Connected)
            co_return cachedDev;
        // Device object exists but disconnected - clear and re-acquire
        LOG_ERROR("Cached device is disconnected, re-acquiring...")
        clearDeviceCache(wstring(deviceId));
    }

    BluetoothLEDevice result = co_await BluetoothLEDevice::FromIdAsync(deviceId);
    if (result == nullptr) {
        LOG_ERROR("Failed to get device from FromIdAsync.")
        co_return nullptr;
    }

    // Create a GattSession to force the BLE connection (critical for Win11)
    try {
        GattSession session = co_await GattSession::FromDeviceIdAsync(result.BluetoothDeviceId());
        if (session != nullptr) {
            session.MaintainConnection(true);
            LOG_ERROR("GattSession created with MaintainConnection=true")

            DeviceCacheEntry d;
            d.device = result;
            d.session = session;
            cache[wstring(deviceId)] = d;
        }
        else {
            LOG_ERROR("Warning: GattSession creation returned null")
            DeviceCacheEntry d;
            d.device = result;
            cache[wstring(deviceId)] = d;
        }
    }
    catch (hresult_error& ex) {
        LOG_ERROR("GattSession creation failed: " << to_string(ex.message().c_str()))
        DeviceCacheEntry d;
        d.device = result;
        cache[wstring(deviceId)] = d;
    }

    // Wait for the device to actually connect (up to 10 seconds)
    // On Win11, the connection is not immediate after FromIdAsync
    for (int waitAttempt = 0; waitAttempt < 20; waitAttempt++) {
        if (result.ConnectionStatus() == BluetoothConnectionStatus::Connected) {
            LOG_ERROR("Device connected after " << (waitAttempt * 500) << "ms")
            break;
        }
        if (waitAttempt == 19) {
            LOG_ERROR("Warning: Device did not reach Connected status within timeout")
        }
        co_await winrt::resume_after(std::chrono::milliseconds(500));
    }

    co_return cache[wstring(deviceId)].device;
}

//Call this function to get a service from cache or async if it wasn't found.
// On Win11, GetGattServicesForUuidAsync throws ERROR_BAD_COMMAND (0x80070016),
// so we discover ALL services and manually filter by UUID.
IAsyncOperation<GattDeviceService> getService(wchar_t* deviceId, wchar_t* serviceId) {
    if (cache.count(wstring(deviceId)) && cache[wstring(deviceId)].service)
        co_return cache[wstring(deviceId)].service;
    auto device = co_await getDevice(deviceId);
    if (device == nullptr)
        co_return nullptr;

    // Discover ALL services (Uncached) - the only reliable method on Win11
    LOG_ERROR("Requesting all GATT services...")
    auto allServicesResult = co_await device.GetGattServicesAsync(BluetoothCacheMode::Uncached);
    if (allServicesResult.Status() != GattCommunicationStatus::Success) {
        LOG_ERROR("GATT service discovery failed. Status: " << (int)allServicesResult.Status())
        co_return nullptr;
    }
    LOG_ERROR("GATT service discovery successful, found " << allServicesResult.Services().Size() << " services")

    // Manually find the service matching the requested UUID
    guid targetGuid = make_guid(serviceId);
    for (uint32_t i = 0; i < allServicesResult.Services().Size(); i++) {
        auto svc = allServicesResult.Services().GetAt(i);
        if (svc.Uuid() == targetGuid) {
            LOG_ERROR("Found matching service UUID")
            if (cache.count(wstring(deviceId))) {
                cache[wstring(deviceId)].service = svc;
            }
            co_return svc;
        }
    }
    LOG_ERROR("No service found matching the target UUID")
    co_return nullptr;
}

//Call this function to get a characteristic from cache or async if it wasn't found.
// On Win11, GetCharacteristicsForUuidAsync throws ERROR_BAD_COMMAND (0x80070016),
// so we discover ALL characteristics and manually filter by UUID.
IAsyncOperation<GattCharacteristic> getCharacteristic(wchar_t* deviceId, wchar_t* serviceId, wchar_t* characteristicId) {
    try {
        CharacteristicCacheKey key;
        key.deviceId = wstring(deviceId);
        key.serviceId = wstring(serviceId);
        key.characteristicId = wstring(characteristicId);

        // Check if characteristic is in cache
        if (characteristicCache.count(key) && characteristicCache[key].has_value()) {
            co_return characteristicCache[key].value();
        }

        auto service = co_await getService(deviceId, serviceId);
        if (service == nullptr)
            co_return nullptr;

        // Discover ALL characteristics (Cached mode to avoid re-query, service is already connected)
        // Avoid GetCharacteristicsForUuidAsync which throws ERROR_BAD_COMMAND on Win11
        LOG_ERROR("Discovering all characteristics for service...")
        GattCharacteristicsResult result = co_await service.GetCharacteristicsAsync(BluetoothCacheMode::Cached);
        if (result.Status() != GattCommunicationStatus::Success) {
            // Retry with Uncached if Cached fails
            LOG_ERROR("Cached characteristic discovery failed (status " << (int)result.Status() << "), trying Uncached...")
            result = co_await service.GetCharacteristicsAsync(BluetoothCacheMode::Uncached);
        }
        if (result.Status() != GattCommunicationStatus::Success) {
            LOG_ERROR("Error discovering characteristics. Status: " << (int)result.Status())
            co_return nullptr;
        }

        LOG_ERROR("Found " << result.Characteristics().Size() << " characteristics in service")

        // Manually find the characteristic matching the requested UUID
        guid targetGuid = make_guid(characteristicId);
        for (uint32_t i = 0; i < result.Characteristics().Size(); i++) {
            auto ch = result.Characteristics().GetAt(i);
            if (ch.Uuid() == targetGuid) {
                LOG_ERROR("Found matching characteristic UUID")
                characteristicCache[key] = ch;
                co_return ch;
            }
        }
        LOG_ERROR("No characteristic found matching the target UUID")
        co_return nullptr;
    }
    catch (hresult_error& ex) {
        LOG_ERROR("Exception in getCharacteristic: " << to_string(ex.message().c_str()) << " (0x" << hex << (uint32_t)ex.code() << dec << ")")
        co_return nullptr;
    }
    catch (std::exception& ex) {
        LOG_ERROR("std::exception in getCharacteristic: " << ex.what())
        co_return nullptr;
    }
    catch (...) {
        LOG_ERROR("Unknown exception in getCharacteristic")
        co_return nullptr;
    }
}

#pragma endregion

#pragma region SCAN DEVICES FUNCTIONS

DeviceWatcher deviceWatcher{ nullptr };
mutex deviceWatcherLock;
DeviceWatcher::Added_revoker deviceWatcherAddedRevoker;
DeviceWatcher::Updated_revoker deviceWatcherUpdatedRevoker;
DeviceWatcher::Removed_revoker deviceWatcherRemovedRevoker;
DeviceWatcher::EnumerationCompleted_revoker deviceWatcherCompletedRevoker;
void BLE_StopDeviceScan();
void BLE_ScanDevices();

//This function would be called when a new BLE device is detected
void DeviceWatcher_Added(DeviceWatcher sender, DeviceInformation deviceInfo) {
    BLEDeviceData deviceData;
    deviceData.id = wstring(deviceInfo.Id().c_str());
    deviceData.name = wstring(deviceInfo.Name().c_str());
    if (deviceInfo.Properties().HasKey(L"System.Devices.Aep.Bluetooth.Le.IsConnectable")) {
        deviceData.isConnectable = unbox_value<bool>(deviceInfo.Properties().Lookup(L"System.Devices.Aep.Bluetooth.Le.IsConnectable"));
    }
    deviceList.push_back(deviceData);
}

//This function would be called when an existing BLE device is updated
void DeviceWatcher_Updated(DeviceWatcher sender, DeviceInformationUpdate deviceInfoUpdate) {
    wstring deviceData = wstring(deviceInfoUpdate.Id().c_str());
    for (int i = 0; i < deviceList.size(); i++) {
        if (deviceList[i].id == deviceData) {
            if (deviceInfoUpdate.Properties().HasKey(L"System.Devices.Aep.Bluetooth.Le.IsConnectable")) {
                deviceList[i].isConnectable = unbox_value<bool>(deviceInfoUpdate.Properties().Lookup(L"System.Devices.Aep.Bluetooth.Le.IsConnectable"));
            }
            break;
        }
    }
}

void DeviceWatcher_Removed(DeviceWatcher sender, DeviceInformationUpdate deviceInfoUpdate) {

}

void DeviceWatcher_EnumerationCompleted(DeviceWatcher sender, IInspectable const&) {
    LOG_ERROR("Enumeration completed.")
    BLE_StopDeviceScan();
    // Only restart scanning if we haven't subscribed yet
    if (bleThreadRunning && !hasSubscribed) BLE_ScanDevices();
}

//Call this function to scan async all BLE devices
void BLE_ScanDevices() {
    try {
        LOG_ERROR("Starting BLE Scan ...")
        lock_guard lock(deviceWatcherLock);
        IVector<hstring> requestedProperties = single_threaded_vector<hstring>({ L"System.Devices.Aep.DeviceAddress", L"System.Devices.Aep.IsConnected", L"System.Devices.Aep.Bluetooth.Le.IsConnectable" });
        hstring aqsFilter = L"(System.Devices.Aep.ProtocolId:=\"{bb7bb05e-5972-42b5-94fc-76eaa7084d49}\")"; // list Bluetooth LE devices
        deviceWatcher = DeviceInformation::CreateWatcher(aqsFilter, requestedProperties, DeviceInformationKind::AssociationEndpoint);
        deviceWatcherAddedRevoker = deviceWatcher.Added(auto_revoke, &DeviceWatcher_Added);
        deviceWatcherUpdatedRevoker = deviceWatcher.Updated(auto_revoke, &DeviceWatcher_Updated);
        deviceWatcherRemovedRevoker = deviceWatcher.Removed(auto_revoke, &DeviceWatcher_Removed);
        deviceWatcherCompletedRevoker = deviceWatcher.EnumerationCompleted(auto_revoke, &DeviceWatcher_EnumerationCompleted);
        deviceWatcher.Start();
        LOG_ERROR("BLE Watcher started.")
    }
    catch (exception e) {
        LOG_ERROR(e.what())
    }
}

void BLE_StopDeviceScan() {
    scoped_lock lock(deviceListLock, deviceWatcherLock);
    if (deviceWatcher != nullptr) {
        deviceWatcherAddedRevoker.revoke();
        deviceWatcherUpdatedRevoker.revoke();
        deviceWatcherRemovedRevoker.revoke();
        deviceWatcherCompletedRevoker.revoke();
        deviceWatcher.Stop();
        deviceWatcher = nullptr;
    }
    deviceListSignal.notify_one();
}

#pragma endregion

#pragma region SUBSCRIBE/READ FUNCTIONS

//On this function you can read all data from the specified characteristic
void Characteristic_ValueChanged(GattCharacteristic const& characteristic, GattValueChangedEventArgs args)
{
    // LOG_ERROR(">>> ValueChanged callback triggered! <<<")
    try {
        IBuffer buffer = args.CharacteristicValue();
        uint32_t dataLength = buffer.Length();

        if (dataLength != num_channels * num_samples * sizeof(float)) {
            LOG_ERROR("Received data length does not match expected size. Expected: " << num_channels * num_samples * sizeof(float) << " bytes, Received: " << dataLength << " bytes.")
            return;
        }

        // currentChannels = dataLength / sizeof(float);
        if (num_channels > MAX_CHANNELS) num_channels = MAX_CHANNELS;

        // Get the data from the buffer
        DataReader reader = DataReader::FromBuffer(buffer);
        reader.ByteOrder(ByteOrder::LittleEndian); // ESP32 sends little-endian

        // LOG_ERROR("Data received: " << dataLength << " bytes" << " - channels:" << num_channels)

        for (int i=0; i<num_samples; i++) {

            // Read the float values for each channel
            for (uint8_t i = 0; i < num_channels; i++) {
                values[i] = reader.ReadSingle(); // ReadSingle() reads a float32
            }
            /*
            // Print the data
            cout << fixed << setprecision(4);
            cout << "Packet #" << bleSampleCount << " - ";
            cout << "Ch1: " << setw(7) << values[0] << " | ";
            cout << "Ch2: " << setw(7) << values[1] << " | ";
            cout << "Ch3: " << setw(7) << values[2] << " | ";
            cout << "Ch4: " << setw(7) << values[3] << endl;
            */
            process_packets(); // trigger signal processing workers!
            bleSampleCount++;
        }
    }
    catch (hresult_error& ex) {
        LOG_ERROR("Error reading characteristic value: " << to_string(ex.message().c_str()))
    }
    catch (...) {
        LOG_ERROR("Unknown error in Characteristic_ValueChanged")
    }
}

//Function used to subscribe async to the specific device
// Includes retry logic: clears stale cache and retries with delay on failure (Win11 compat)
fire_and_forget SubscribeCharacteristicAsync(wstring deviceId, wstring serviceId, wstring characteristicId, bool* result) {
    const int MAX_RETRIES = 5;
    const int RETRY_DELAY_MS = 2000;

    for (int attempt = 1; attempt <= MAX_RETRIES; attempt++) {
        try {
            if (attempt > 1) {
                LOG_ERROR("Retry attempt " << attempt << "/" << MAX_RETRIES << " - clearing cache and waiting...")
                // Clear stale cached BLE objects before retrying
                clearDeviceCache(deviceId);
                // Wait before retry to let the BLE stack settle
                co_await winrt::resume_after(std::chrono::milliseconds(RETRY_DELAY_MS));
            }

            auto characteristic = co_await getCharacteristic(&deviceId[0], &serviceId[0], &characteristicId[0]);
            if (characteristic != nullptr) {
                auto status = co_await characteristic.WriteClientCharacteristicConfigurationDescriptorAsync(GattClientCharacteristicConfigurationDescriptorValue::Notify);
                if (status != GattCommunicationStatus::Success) {
                    LOG_ERROR("Error subscribing to characteristic. Status: " << (int)status)
                    // Retry on Unreachable or ProtocolError
                    if (attempt < MAX_RETRIES) continue;
                }
                else {
                    LOG_ERROR("Successfully subscribed to characteristic!")
                    for (int i = 0; i < deviceList.size(); i++) {
                        if (deviceList[i].id == deviceId) {
                            deviceList[i].subscription = new Subscription();
                            deviceList[i].subscription->revoker = characteristic.ValueChanged(auto_revoke, &Characteristic_ValueChanged);
                            break;
                        }
                    }
                    if (result != 0)
                        *result = true;
                    break; // Success - exit retry loop
                }
            }
            else {
                LOG_ERROR("Failed to get characteristic for subscription (attempt " << attempt << "/" << MAX_RETRIES << ")")
                if (attempt < MAX_RETRIES) continue;
            }
        }
        catch (hresult_error& ex)
        {
            LOG_ERROR("SubscribeCharacteristicAsync error: " << to_string(ex.message().c_str()))
            for (int i = 0; i < deviceList.size(); i++) {
                if (deviceList[i].id == deviceId && deviceList[i].subscription) {
                    delete deviceList[i].subscription;
                    deviceList[i].subscription = NULL;
                    break;
                }
            }
            if (attempt < MAX_RETRIES) continue;
        }
    }
    subscribeSignal.notify_one();
}

//Call this function to subscribe to the specific device so you can read data from it
bool SubscribeCharacteristic(wstring deviceId, wstring serviceId, wstring characteristicId) {
    unique_lock<mutex> lock(subscribeLock);
    bool result = false;
    SubscribeCharacteristicAsync(deviceId, serviceId, characteristicId, &result);
    subscribeSignal.wait(lock);
    return result;
}

#pragma endregion


DWORD WINAPI BleProc(LPVOID lpv)
{
    bleThreadRunning = 1;
    bleThreadDone = 0;
    bleSampleCount = 0;

    LOG_ERROR("Trying to connect device: " << ble_name) // XIAO-ESP32S3-WebBT

    //To start scanning just call this function
    BLE_ScanDevices();

    //It will be executed always
    while (!bleThreadDone) {
        //Then every device and their info updated would be in this vector
        for (int i = 0; i < deviceList.size(); i++) {
            //If the device is connectable we will try to connect if we aren't subscribed yet
            if (deviceList[i].isConnectable) {
                // Look for the target device by name
                string deviceName = to_string(deviceList[i].name);
                if (!hasSubscribed && deviceName.find(ble_name) != string::npos) {
                    cout << "Found device: " << deviceName << endl;
                    cout << "Device ID: " << to_string(deviceList[i].id) << endl;

                    // Small delay after discovery before attempting GATT operations
                    LOG_ERROR("Device found, waiting briefly before connection attempt...")
                    Sleep(500);

                    LOG_ERROR("Attempting to subscribe to notifications...");

                    connectedDeviceId = deviceList[i].id; // Save the device ID
                    bool subscribed = SubscribeCharacteristic(deviceList[i].id, CharToWString(serviceUUID), CharToWString(readUUID));
                    if (subscribed) {
                        hasSubscribed = true;
                        LOG_ERROR("Subscription successful! Waiting for data...")
                        // Wait a bit for subscription to settle
                        Sleep(500);
                        GLOBAL.bleReceiver_available = 1;
                    }
                    else {
                        LOG_ERROR("Subscription failed after all retries!")
                        // Clear caches so next attempt starts fresh
                        clearDeviceCache(deviceList[i].id);
                    }
                }
            }
        }
        Sleep(waitTime);
    }
   bleThreadRunning = 0;
   GLOBAL.bleReceiver_available = 0;

   LOG_ERROR("BLE thread stopped!");
   return(0);
}


// <------------------------------------------------------------------------------------


int GENERIC_BLE_RECEIVEROBJ::connect(void) {
    if (!bleThreadRunning) {
        LOG_ERROR("\nStarting BLE Thread!");
        CreateThread(NULL, 1000, (LPTHREAD_START_ROUTINE)BleProc, 0, 0, &bleStatId);
    } else LOG_ERROR("\nBLE Thread already running!");
    return(1);
}

int GENERIC_BLE_RECEIVEROBJ::close(void) {
    if (bleThreadRunning) {
        LOG_ERROR("shutting down BLE thread!");
        bleThreadDone = 1; // Signal thread to stop

        // Wait for thread to finish
        int timeout = 0;
        while (bleThreadRunning && timeout < 30) {
            Sleep(100);
            timeout++;
        }

        // Clean up
        for (int i = 0; i < deviceList.size(); i++) {
            if (deviceList[i].subscription) {
                delete deviceList[i].subscription;
                deviceList[i].subscription = NULL;
            }
        }

        // Properly close all cached BLE objects
        for (auto& entry : cache) {
            if (entry.second.session != nullptr) {
                entry.second.session.MaintainConnection(false);
                entry.second.session.Close();
            }
            if (entry.second.service != nullptr) {
                entry.second.service.Close();
            }
            if (entry.second.device != nullptr) {
                entry.second.device.Close();
            }
        }

        hasSubscribed = false;
        connectedDeviceId = L"";
        deviceList.clear();
        cache.clear();
        characteristicCache.clear();

        LOG_ERROR("BLE connection closed and reset!");
    }
    else {
        LOG_ERROR("BLE thread not running ..");
    }
    return(0);
}



LRESULT CALLBACK GenericBleDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

GENERIC_BLE_RECEIVEROBJ::GENERIC_BLE_RECEIVEROBJ(int num) : BASE_CL()
{
    int i;
    inports = 0;
    outports = MAX_CHANNELS; 
    width = 100; height = CON_START + outports * CON_HEIGHT + 5;

    for (i = 0; i < outports; i++) {
        sprintf(out_ports[i].out_name, "Chn%d", i + 1);
        sprintf(out_ports[i].out_desc, "channel %d", i + 1);
    }
}

GENERIC_BLE_RECEIVEROBJ::~GENERIC_BLE_RECEIVEROBJ()
{
    close();
    printf("BLE-receiver element terminated\n");
}

void GENERIC_BLE_RECEIVEROBJ::make_dialog(void)
{
    display_toolbox(hDlg = CreateDialog(hInst, (LPCTSTR)IDD_GENERICBLEBOX, ghWndStatusbox, (DLGPROC)GenericBleDlgHandler));
}

void GENERIC_BLE_RECEIVEROBJ::load(HANDLE hFile)
{
    load_object_basics(this);
    load_property("ble_name", P_STRING, ble_name);
    load_property("serviceUUID", P_STRING, serviceUUID);
    load_property("readUUID", P_STRING, readUUID);
    load_property("num_channels", P_INT, &num_channels);
    load_property("num_samples", P_INT, &num_samples);

}

void GENERIC_BLE_RECEIVEROBJ::save(HANDLE hFile)
{
    save_object_basics(hFile, this);
    save_property(hFile, "ble_name", P_STRING, ble_name);
    save_property(hFile, "serviceUUID", P_STRING, serviceUUID);
    save_property(hFile, "readUUID", P_STRING, readUUID);
    save_property(hFile, "num_channels", P_INT, &num_channels);
    save_property(hFile, "num_samples", P_INT, &num_samples);
}

void GENERIC_BLE_RECEIVEROBJ::work(void)
{
    if ((bleSampleCount % 100 == 0) && (hDlg == ghWndToolbox)) {
        char szdata[100];
        sprintf(szdata, "%d Samples read\n", bleSampleCount);
        add_to_listbox(hDlg, IDC_LIST, szdata);
    }

    // send float values
    for (int i = 0; i < num_channels; i++) {
        pass_values(i, values[i]);
    }
}

void GENERIC_BLE_RECEIVEROBJ::session_start(void)
{
   connect();
}

void GENERIC_BLE_RECEIVEROBJ::session_stop(void)
{
   close();
}


// Dialog Handler
LRESULT CALLBACK GenericBleDlgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    GENERIC_BLE_RECEIVEROBJ* st;
    st = (GENERIC_BLE_RECEIVEROBJ*)actobject;
    if ((st == NULL) || (st->type != OB_GENERIC_BLE_RECEIVER)) {
        return(0);
    }

    switch (message)
    {
    case WM_INITDIALOG:
        if (st) {
            SetDlgItemText(hDlg, IDC_BLE_NAME, ble_name);
            SetDlgItemText(hDlg, IDC_SERVICE_UUID, serviceUUID);
            SetDlgItemText(hDlg, IDC_READ_UUID, readUUID);
            SetDlgItemInt(hDlg, IDC_NUM_CHANNELS, num_channels, FALSE);
            SetDlgItemInt(hDlg, IDC_NUM_SAMPLES, num_samples, FALSE);

            if (GLOBAL.bleReceiver_available) {
                SendDlgItemMessage(hDlg, IDC_OPENED, BM_SETCHECK, BST_CHECKED,  0);
                add_to_listbox(hDlg, IDC_LIST, "BLE devcie connected.");
            }
            else {
                SendDlgItemMessage(hDlg, IDC_OPENED, BM_SETCHECK, BST_UNCHECKED, 0);
                add_to_listbox(hDlg, IDC_LIST, "BLE device disconnected");
            }
        }
        return TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_CONNECT:
            GetDlgItemText(hDlg, IDC_BLE_NAME, ble_name, sizeof(ble_name));
            GetDlgItemText(hDlg, IDC_SERVICE_UUID, serviceUUID, sizeof(serviceUUID));
            GetDlgItemText(hDlg, IDC_READ_UUID, readUUID, sizeof(readUUID));
            num_channels = GetDlgItemInt(hDlg, IDC_NUM_CHANNELS, NULL, FALSE);
            num_samples = GetDlgItemInt(hDlg, IDC_NUM_SAMPLES, NULL, FALSE);
            st->connect();
            add_to_listbox(hDlg, IDC_LIST, "Trying to connect BLE device");
            // MessageBox(hDlg, "Could connect to BLE device", "BLE Receiver", MB_OK | MB_ICONERROR);
            return TRUE;
        case IDC_CLOSE:
            if (st) { 
                st->close(); 
                SendDlgItemMessage(hDlg, IDC_OPENED, BM_SETCHECK, BST_UNCHECKED, 0);
                add_to_listbox(hDlg, IDC_LIST, "BLE device disconnected");
            }
            return TRUE;
        }
        return TRUE;
    case WM_CLOSE:
        EndDialog(hDlg, LOWORD(wParam));
        return TRUE;
    }
    return FALSE;
}
