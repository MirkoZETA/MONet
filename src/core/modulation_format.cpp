#include "modulation_format.hpp"
#include <stdexcept>

ModulationFormat::ModulationFormat(const std::string &modulationStr)
		: modulationStr(modulationStr)
{
}

ModulationFormat::ModulationFormat(const std::string &modulationStr,
																	 const std::map<fns::Band, int> &requiredSlotsPerBand,
																	 const std::map<fns::Band, double> &reachPerBand)
		: modulationStr(modulationStr),
			requiredSlotsPerBand(requiredSlotsPerBand),
			reachPerBand(reachPerBand)
{
}

std::string ModulationFormat::getModulationStr() const
{
	return this->modulationStr;
}

int ModulationFormat::getRequiredSlots(fns::Band band) const
{
	auto it = this->requiredSlotsPerBand.find(band);
	if (it != this->requiredSlotsPerBand.end())
	{
		return it->second;
	}
	throw std::invalid_argument("Band not found in modulation format");
}

int ModulationFormat::getRequiredSlots() const
{
	return getRequiredSlots(fns::Band::C); // Default to C band
}

double ModulationFormat::getReach(fns::Band band) const
{
	auto it = this->reachPerBand.find(band);
	if (it != this->reachPerBand.end())
	{
		return it->second;
	}
	throw std::invalid_argument("Band not found in modulation format");
}

double ModulationFormat::getReach() const
{
	return getReach(fns::Band::C); // Default to C band
}

void ModulationFormat::setRequiredSlots(fns::Band band, int slots)
{
	if (slots < 0)
	{
		throw std::invalid_argument("Slots cannot be negative");
	}
	this->requiredSlotsPerBand[band] = slots;
}

void ModulationFormat::setReach(fns::Band band, double reach)
{
	if (reach < 0.0)
	{
		throw std::invalid_argument("Reach cannot be negative");
	}
	this->reachPerBand[band] = reach;
}

double ModulationFormat::getRequiredGSNR() const
{
	return this->requiredGSNR;
}

void ModulationFormat::setRequiredGSNR(double gsnr)
{
	this->requiredGSNR = gsnr;
}

double ModulationFormat::getBaudRate() const
{
	return this->baudRate;
}

void ModulationFormat::setBaudRate(double baudRate)
{
	if (baudRate < 0.0)
	{
		throw std::invalid_argument("Baud rate cannot be negative");
	}
	this->baudRate = baudRate;
}
