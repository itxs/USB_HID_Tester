#include "usbhiddevice.h"

QList<USBHIDDevice::DevInfo> USBHIDDevice::devList;

USBHIDDevice::USBHIDDevice(QObject* parent, uint16_t vid, uint16_t pid, wchar_t* serial) : QObject(parent), asyncTimer(this)
{
    hid_init();
    _serial[0] = 0;
    setDeviceParams(vid, pid, serial);

    asyncTimer.setSingleShot(true);
    asyncTimer.setTimerType(Qt::TimerType::CoarseTimer);
    asyncTimer.setInterval(10);
    connect(&asyncTimer, SIGNAL(timeout()), this, SLOT(updateMsgReceivedFlag()));

    connectionTimer.setSingleShot(false);
    connectionTimer.setTimerType(Qt::TimerType::CoarseTimer);
    connectionTimer.setInterval(500);
    connect(&connectionTimer, SIGNAL(timeout()), this, SLOT(updateConnStatusFlag()));
}

bool USBHIDDevice::enumerate(uint16_t vid, uint16_t pid)
{
    bool status = false;
    hid_device_info* devStrings = hid_enumerate(vid, pid);

    if (devStrings != NULL)
        status = true;

    while(devStrings != NULL)
    {
        devList.push_back(DevInfo(devStrings));
        devStrings = devStrings->next;
    }
    hid_free_enumeration(devStrings);

    return status;
}

bool USBHIDDevice::checkDevConnected()
{
    wchar_t str[32];
    bool result = false;

    if (activeDevice != NULL)
        result = hid_get_product_string(activeDevice, str, 32) == 0;
    else
        result = enumerate(_vid, _pid);

    if (result)
    {
        if (wcslen(_serial) == 0)
            return true;
        else
        {
            hid_device* dev = hid_open(_vid, _pid, _serial);
            if (dev != NULL)
            {
                hid_close(dev);
                return true;
            }
            else return false;
        }
    }
    else return false;
}

bool USBHIDDevice::checkDevOpened()
{
    return activeDevice != NULL;
}

void USBHIDDevice::enableDevConnEvent(bool enabled)
{
    if (enabled)
        connectionTimer.start();
    else
        connectionTimer.stop();
}

QList<USBHIDDevice::DevInfo>& USBHIDDevice::getDevList()
{
    return devList;
}

bool USBHIDDevice::open(bool blocking)
{
    if (wcslen(_serial) == 0)
        activeDevice = hid_open(_vid, _pid, NULL);
    else
        activeDevice = hid_open(_vid, _pid, _serial);

    if (activeDevice)
    {
        if (!blocking)
        {
            hid_set_nonblocking(activeDevice, !blocking);
            blockingMode = blocking;
        }
    }
    return activeDevice != NULL;
}

void USBHIDDevice::close()
{
    if (activeDevice)
    {
        hid_close(activeDevice);
        activeDevice = NULL;
    }
}

void USBHIDDevice::read(uint8_t* data, uint16_t size)
{
    if (activeDevice)
    {
        if (blockingMode)
            hid_read(activeDevice, data, size);
        else
        {
            unblockingReadSize = size;
            unblockingMsgReceived = hid_read(activeDevice, inReportBuf, size) > 0;
            if (!unblockingMsgReceived)
                asyncTimer.start();
        }
    }
}

void USBHIDDevice::write(uint8_t* data, uint16_t size, uint8_t reportId)
{
    if (activeDevice)
    {
        outReportBuf[0] = reportId;
        memcpy(outReportBuf + 1, data, size);
        hid_write(activeDevice, outReportBuf, size + 1);
    }
}

void USBHIDDevice::getFeature(uint8_t* data, uint16_t size, uint8_t reportId)
{
    if (activeDevice)
    {
        inReportBuf[0] = reportId;
        hid_get_feature_report(activeDevice, inReportBuf, size + 1);
        if (blockingMode)
            memcpy(data, inReportBuf + 1, size);
    }
}

void USBHIDDevice::setFeature(uint8_t* data, uint16_t size, uint8_t reportId)
{
    if (activeDevice)
    {
        outReportBuf[0] = reportId;
        memcpy(outReportBuf + 1, data, size);
        hid_send_feature_report(activeDevice, outReportBuf, size + 1);
    }
}

uint8_t* USBHIDDevice::getAsyncReceivedData()
{
    return inReportBuf;
}

void USBHIDDevice::setAsyncTimeout(uint32_t ms)
{
    asyncTimer.setInterval(ms);
}

void USBHIDDevice::setDeviceParams(uint16_t vid, uint16_t pid, wchar_t* serial)
{
    _vid = vid;
    _pid = pid;
    if (serial != NULL)
       wcscpy(_serial, serial);
}

bool USBHIDDevice::isBusy()
{
    return false;
}

void USBHIDDevice::updateMsgReceivedFlag()
{
    if (!blockingMode && (unblockingReadSize != 0))
    {
        unblockingMsgReceived = hid_read(activeDevice, inReportBuf, unblockingReadSize) > 0;
        if (unblockingMsgReceived)
            emit asyncDataReady(inReportBuf, unblockingReadSize);
        else if (unblockingCheckCnt++ < unblockingMaxChecks)
                asyncTimer.start();
    }
}

void USBHIDDevice::updateConnStatusFlag()
{
    bool connected = checkDevConnected();
    if (connStatus != connected)
    {
        connStatus = connected;
        emit connectionChanged(connected);
    }
}
