// Local includes
#include <utils/ColorRgbw.h>

const ColorRgbw ColorRgbw::BLACK  = {   0,   0,   0,   0 };
const ColorRgbw ColorRgbw::RED    = { 255,   0,   0,   0 };
const ColorRgbw ColorRgbw::GREEN  = {   0, 255,   0,   0 };
const ColorRgbw ColorRgbw::BLUE   = {   0,   0, 255,   0 };
const ColorRgbw ColorRgbw::YELLOW = { 255, 255,   0,   0 };
const ColorRgbw ColorRgbw::WHITE  = {   0,   0,   0, 255 };

namespace RGBW {

	WhiteAlgorithm stringToWhiteAlgorithm(const QString& str)
	{
		if (str == "subtract_minimum")
		{
			return WhiteAlgorithm::SUBTRACT_MINIMUM;
		}
		if (str == "sub_min_warm_adjust")
		{
			return WhiteAlgorithm::SUB_MIN_WARM_ADJUST;
		}
		if (str == "sub_min_cool_adjust")
		{
			return WhiteAlgorithm::SUB_MIN_COOL_ADJUST;
		}
		if (str == "sub_min_custom_adjust")
		{
			return WhiteAlgorithm::SUB_MIN_CUSTOM_ADJUST;
		}

		if (str == "wled_auto")
		{
			return WhiteAlgorithm::WLED_AUTO;
		}

		if (str == "wled_auto_max")
		{
			return WhiteAlgorithm::WLED_AUTO_MAX;
		}
		
		if (str == "wled_auto_accurate")
		{
			return WhiteAlgorithm::WLED_AUTO_ACCURATE;
		}
		
		if (str.isEmpty() || str == "white_off")
		{
			return WhiteAlgorithm::WHITE_OFF;
		}
		return WhiteAlgorithm::INVALID;
	}
	void Rgb_to_RgbwAdjust(ColorRgb input, ColorRgbw* output, CalibrationConfig config)
	{
		output->white = static_cast<uint8_t>(qMin(input.red * config.F1, qMin(input.green * config.F2, input.blue * config.F3)));
		output->red = input.red - static_cast<uint8_t>(output->white / config.F1);
		output->green = input.green - static_cast<uint8_t>(output->white / config.F2);
		output->blue = input.blue - static_cast<uint8_t>(output->white / config.F3);
	}

	void Rgb_to_Rgbw(ColorRgb input, ColorRgbw* output, WhiteAlgorithm algorithm)
	{
		switch (algorithm)
		{
		case WhiteAlgorithm::SUBTRACT_MINIMUM:
		{
			output->white = static_cast<uint8_t>(qMin(qMin(input.red, input.green), input.blue));
			output->red = input.red - output->white;
			output->green = input.green - output->white;
			output->blue = input.blue - output->white;
			break;
		}

		case WhiteAlgorithm::SUB_MIN_WARM_ADJUST:
		{
			Rgb_to_RgbwAdjust(input, output, WARM_WHITE);
			break;
		}

		case WhiteAlgorithm::SUB_MIN_COOL_ADJUST:
		{
			Rgb_to_RgbwAdjust(input, output, COLD_WHITE);			
			break;
		}

		case WhiteAlgorithm::WHITE_OFF:
		{
			output->red = input.red;
			output->green = input.green;
			output->blue = input.blue;
			output->white = 0;
			break;
		}

		case WhiteAlgorithm::WLED_AUTO_MAX: 
		{
			output->red = input.red;
			output->green = input.green;
			output->blue = input.blue;
			output->white = input.red > input.green ? (input.red > input.blue ? input.red : input.blue) : (input.green > input.blue ? input.green : input.blue);
			break;
		}
		
		case WhiteAlgorithm::WLED_AUTO_ACCURATE:
		{
			output->white = input.red < input.green ? (input.red < input.blue ? input.red : input.blue) : (input.green < input.blue ? input.green : input.blue);
			output->red = input.red - output->white;
			output->green = input.green - output->white;
			output->blue = input.blue - output->white;
			break;
		}

		case WhiteAlgorithm::WLED_AUTO:
		{

			output->red = input.red;
			output->green = input.green;
			output->blue = input.blue;
			output->white = input.red < input.green ? (input.red < input.blue ? input.red : input.blue) : (input.green < input.blue ? input.green : input.blue);
			break;
		}
		default:
			break;
		}
	}
};
