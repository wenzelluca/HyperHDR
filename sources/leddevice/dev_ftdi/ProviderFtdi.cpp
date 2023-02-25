// LedDevice includes
#include <leddevice/LedDevice.h>
#include "ProviderFtdi.h"

#include <ftdi.h>
#include <thread>

#define ANY_FTDI_VENDOR 0x0
#define ANY_FTDI_PRODUCT 0x0

ProviderFtdi::ProviderFtdi(const QJsonObject &deviceConfig)
	: LedDevice(deviceConfig),
	  _ftdic(NULL),
	  _baudRate_Hz(1000000)
{
	_baudRate_Hz = deviceConfig["rate"].toInt(_baudRate_Hz);
}

bool ProviderFtdi::init(const QJsonObject &deviceConfig)
{
	bool isInitOK = false;

	if (LedDevice::init(deviceConfig))
	{
		_baudRate_Hz = deviceConfig["rate"].toInt(_baudRate_Hz);

		Debug(_log, "_baudRate_Hz [%d]", _baudRate_Hz);

		isInitOK = true;
	}
	return isInitOK;
}

int ProviderFtdi::open()
{
	int rc = 0;

	Debug(_log, "Opening FTDI device");

	if ((_ftdic = ftdi_new()) == NULL)
	{
		setInError(ftdi_get_error_string(_ftdic));
		return -1;
	}
	struct ftdi_device_list *devlist;
	int devices_found = 0;

	if ((devices_found = ftdi_usb_find_all(_ftdic, &devlist, ANY_FTDI_VENDOR, ANY_FTDI_PRODUCT)) < 0)
	{
		setInError(ftdi_get_error_string(_ftdic));
		return -1;
	}
	if (devices_found < 1)
	{
		setInError("No FTDI devices detected");
		return -1;
	}

	if ((rc = ftdi_usb_open_dev(_ftdic, devlist[0].dev)) < 0)
	{
		ftdi_list_free(&devlist);
		setInError(ftdi_get_error_string(_ftdic));
		return rc;
	}

	ftdi_list_free(&devlist);

	/* doing this disable resets things if they were in a bad state */
	if ((rc = ftdi_disable_bitbang(_ftdic)) < 0)
	{
		setInError(ftdi_get_error_string(_ftdic));
		return rc;
	}
	if ((rc = ftdi_setflowctrl(_ftdic, SIO_DISABLE_FLOW_CTRL)) < 0)
	{
		setInError(ftdi_get_error_string(_ftdic));
		return rc;
	}
	if ((rc = ftdi_set_bitmode(_ftdic, 0x00, BITMODE_RESET)) < 0)
	{
		setInError(ftdi_get_error_string(_ftdic));
		return rc;
	}

	if ((rc = ftdi_set_bitmode(_ftdic, 0xff, BITMODE_MPSSE)) < 0)
	{
		setInError(ftdi_get_error_string(_ftdic));
		return rc;
	}

	double reference_clock = 60e6;
	int divisor = (reference_clock / 2 / _baudRate_Hz) - 1;

	if ((rc = writeByte(DIS_DIV_5)) != 1)
	{
		return rc;
	}
	if ((rc = writeByte(TCK_DIVISOR)) != 1)
	{
		return rc;
	}
	if ((rc = writeByte(divisor)) != 1)
	{
		return rc;
	}
	if ((rc = writeByte(divisor >> 8)) != 1)
	{
		return rc;
	}
	if ((rc = writeByte(SET_BITS_LOW)) != 1)
	{
		return rc;
	}

	if ((rc = writeByte(0b00000000)) != 1)
	{
		return rc;
	}
	if ((rc = writeByte(0b00000011)) != 1)
	{
		return rc;
	}
	_isDeviceReady = true;
	return rc;
}

int ProviderFtdi::close()
{
	if (_ftdic != NULL)
	{
		// allow to clock out remaining data from powerOff()->writeBlack()
		std::this_thread::sleep_for(std::chrono::milliseconds(15));
		Debug(_log, "Closing FTDI device");
		ftdi_set_bitmode(_ftdic, 0x00, BITMODE_RESET);
		ftdi_usb_close(_ftdic);
		ftdi_free(_ftdic);

		_ftdic = NULL;
	}
	return LedDevice::close();
}

void ProviderFtdi::setInError(const QString &errorMsg)
{
	close();

	LedDevice::setInError(errorMsg);
}

int ProviderFtdi::writeByte(uint8_t data)
{
	int rc = ftdi_write_data(_ftdic, &data, 1);
	if (rc != 1)
	{
		setInError(ftdi_get_error_string(_ftdic));
	}
	return rc;
}
int ProviderFtdi::writeBytes(const qint64 size, const uint8_t *data)
{
	int rc = 0;

	int count_arg = size - 1;

	if ((rc = writeByte(MPSSE_DO_WRITE | MPSSE_WRITE_NEG)) != 1)
	{
		return rc;
	}

	if ((rc = writeByte(count_arg)) != 1)
	{
		return rc;
	}
	if ((rc = writeByte(count_arg >> 8)) != 1)
	{
		return rc;
	}
	if ((rc = ftdi_write_data(_ftdic, data, size)) != size)
	{
		setInError(ftdi_get_error_string(_ftdic));
		return rc;
	}

	return rc;
}