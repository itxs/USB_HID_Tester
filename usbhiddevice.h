#ifndef USBHIDDEVICE_H
#define USBHIDDEVICE_H

#include "hidapi.h"
#include "stdint.h"
#include "QString"
#include "QList"
#include "QTimer"

class USBHIDDevice : public QObject
{
    Q_OBJECT
public:
    struct DevInfo
    {
        DevInfo(hid_device_info* info)
        {
            path = QString::fromUtf8(info->path);
            vid = info->vendor_id;
            pid = info->vendor_id;
            serialNum = info->vendor_id;
            releaseNum = info->vendor_id;
            manufacturerStr = QString::fromWCharArray(info->manufacturer_string);
            productStr = QString::fromWCharArray(info->product_string);
            usage_page = info->vendor_id;
            usage = info->vendor_id;
            interfaceNum = info->vendor_id;
        }
        QString path;           // Platform-specific device path
        uint16_t vid;           // Device Vendor ID
        uint16_t pid;           // Device Product ID
        QString serialNum;      // Serial Number
        uint16_t releaseNum;    // Device Release Number in BCD
        QString manufacturerStr;// Manufacturer String
        QString productStr;     // Product string
        uint16_t usage_page;    // Usage Page for this Device/Interface (Windows/Mac only).
        uint16_t usage;         // Usage for this Device/Interface (Windows/Mac only).
        uint16_t interfaceNum;  // The USB interface which this logical device represents.
    };
    USBHIDDevice(QObject* parent, uint16_t vid, uint16_t pid, wchar_t* serial = NULL);
    virtual ~USBHIDDevice() {};
    static bool enumerate(uint16_t vid = 0, uint16_t pid = 0);
    bool checkDevConnected();
    bool checkDevOpened();
    void enableDevConnEvent(bool enabled);
    static QList<DevInfo>& getDevList();
    bool open(bool blocking = false);
    void close();
    void read(uint8_t* data, uint16_t size);
    void write(uint8_t* data, uint16_t size, uint8_t reportId = 0);
    void setFeature(uint8_t* data, uint16_t size, uint8_t reportId = 0);
    void getFeature(uint8_t* data, uint16_t size, uint8_t reportId = 0);
    uint8_t* getAsyncReceivedData();
    void setAsyncTimeout(uint32_t ms);
    void setDeviceParams(uint16_t vid, uint16_t pid, wchar_t* serial = NULL);
    bool isBusy();
signals:
    void asyncDataReady(uint8_t* data, uint8_t size);
    void connectionChanged(bool connected);
private slots:
    void updateMsgReceivedFlag();
    void updateConnStatusFlag();
private:
    static QList<DevInfo> devList;
    hid_device* activeDevice = NULL;
    uint8_t outReportBuf[65];
    uint8_t inReportBuf[65];
    bool blockingMode = false;
    bool unblockingMsgReceived = false;
    QTimer asyncTimer;
    QTimer connectionTimer;
    uint8_t unblockingReadSize = 0;
    static constexpr uint8_t unblockingMaxChecks = 10;
    uint8_t unblockingCheckCnt = 0;
    bool connStatus = false;
    uint16_t _vid, _pid;
    wchar_t _serial[32] = {0};
};

#endif // USBHIDDEVICE_H
