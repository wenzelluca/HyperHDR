#include "LedDeviceAPA102_ftdi.h"

#define LED_HEADER 0b11100000
#define LED_BRIGHTNESS_FULL 31

LedDeviceAPA102_ftdi::LedDeviceAPA102_ftdi(const QJsonObject &deviceConfig) : ProviderFtdi(deviceConfig)
{
}

LedDevice *LedDeviceAPA102_ftdi::construct(const QJsonObject &deviceConfig)
{
	return new LedDeviceAPA102_ftdi(deviceConfig);
}

bool LedDeviceAPA102_ftdi::init(const QJsonObject &deviceConfig)
{
	bool isInitOK = false;

	// Initialise sub-class
	if (ProviderFtdi::init(deviceConfig))
	{
		CreateHeader();
		isInitOK = true;
	}
	return isInitOK;
}

void LedDeviceAPA102_ftdi::CreateHeader()
{
	const unsigned int startFrameSize = 4;
	const unsigned int endFrameSize = qMax<unsigned int>(((_ledCount + 15) / 16), 4);
	const unsigned int APAbufferSize = (_ledCount * 4) + startFrameSize + endFrameSize;

	_ledBuffer.resize(0, 0xFF);
	_ledBuffer.resize(APAbufferSize, 0xFF);
	_ledBuffer[0] = 0x00;
	_ledBuffer[1] = 0x00;
	_ledBuffer[2] = 0x00;
	_ledBuffer[3] = 0x00;

	Debug(_log, "APA102 buffer created. Led's number: %d", _ledCount);
}

int LedDeviceAPA102_ftdi::write(const std::vector<ColorRgb> &ledValues)
{
	if (_ledCount != ledValues.size())
	{
		Warning(_log, "APA102 led's number has changed (old: %d, new: %d). Rebuilding buffer.", _ledCount, ledValues.size());
		_ledCount = ledValues.size();

		CreateHeader();
	}

	for (signed iLed = 0; iLed < static_cast<int>(_ledCount); ++iLed)
	{
		const ColorRgb &rgb = ledValues[iLed];
		_ledBuffer[4 + iLed * 4 + 0] = LED_HEADER | LED_BRIGHTNESS_FULL;
		_ledBuffer[4 + iLed * 4 + 1] = rgb.red;
		_ledBuffer[4 + iLed * 4 + 2] = rgb.green;
		_ledBuffer[4 + iLed * 4 + 3] = rgb.blue;
	}

	return writeBytes(_ledBuffer.size(), _ledBuffer.data());
}
