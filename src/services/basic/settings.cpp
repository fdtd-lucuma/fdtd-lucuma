// Una GUI para fdtd
// Copyright © 2025 Otreblan
//
// fdtd-lucuma is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// fdtd-lucuma is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with fdtd-lucuma.  If not, see <http://www.gnu.org/licenses/>.

module;

module lucuma.services.basic;

import std;
import glm;

namespace lucuma::services::basic
{

Settings::Settings([[maybe_unused]]Injector& injector):
	argumentParser(injector.inject<ArgumentParser>())
{ }

bool Settings::isHeadless() const
{
	return argumentParser.isHeadless();
}

bool Settings::debug() const
{
	return argumentParser.debug();
}

std::size_t Settings::sizeX() const
{
	return argumentParser.sizeX().value_or(defaultSizeX);
}

std::size_t Settings::sizeY() const
{
	return argumentParser.sizeY().value_or(defaultSizeY);
}

std::size_t Settings::sizeZ() const
{
	return argumentParser.sizeZ().value_or(defaultSizeZ);
}

std::size_t Settings::gaussX() const
{
	return argumentParser.gaussX().value_or(defaultGaussX);
}

std::size_t Settings::gaussY() const
{
	return argumentParser.gaussY().value_or(defaultGaussY);
}

std::size_t Settings::gaussZ() const
{
	return argumentParser.gaussZ().value_or(defaultGaussZ);
}

float Settings::deltaT() const
{
	return argumentParser.deltaT().value_or(defaultDeltaT);
}

float Settings::deltaX() const
{
	return argumentParser.deltaX().value_or(defaultDeltaX);
}

float Settings::deltaY() const
{
	return argumentParser.deltaY().value_or(defaultDeltaY);
}

float Settings::deltaZ() const
{
	return argumentParser.deltaZ().value_or(defaultDeltaZ);
}

float Settings::imp0() const
{
	return argumentParser.imp0().value_or(defaultImp0);
}

float Settings::Cr() const
{
	return argumentParser.Cr().value_or(1.f/std::sqrt(3.f));
}

float Settings::gaussSigma() const
{
	return argumentParser.gaussSigma().value_or(defaultGaussSigma);
}

const std::filesystem::path Settings::empty = {};

const std::filesystem::path& Settings::Hx0() const
{
	const auto& ref = argumentParser.Hx0();
	return ref.has_value() ? ref.value() : empty;
}

const std::filesystem::path& Settings::Hy0() const
{
	const auto& ref = argumentParser.Hy0();
	return ref.has_value() ? ref.value() : empty;
}

const std::filesystem::path& Settings::Hz0() const
{
	const auto& ref = argumentParser.Hz0();
	return ref.has_value() ? ref.value() : empty;
}

const std::filesystem::path& Settings::Chxh0() const
{
	const auto& ref = argumentParser.Chxh0();
	return ref.has_value() ? ref.value() : empty;
}

const std::filesystem::path& Settings::Chyh0() const
{
	const auto& ref = argumentParser.Chyh0();
	return ref.has_value() ? ref.value() : empty;
}

const std::filesystem::path& Settings::Chzh0() const
{
	const auto& ref = argumentParser.Chzh0();
	return ref.has_value() ? ref.value() : empty;
}

const std::filesystem::path& Settings::Chxe0() const
{
	const auto& ref = argumentParser.Chxe0();
	return ref.has_value() ? ref.value() : empty;
}

const std::filesystem::path& Settings::Chye0() const
{
	const auto& ref = argumentParser.Chye0();
	return ref.has_value() ? ref.value() : empty;
}

const std::filesystem::path& Settings::Chze0() const
{
	const auto& ref = argumentParser.Chze0();
	return ref.has_value() ? ref.value() : empty;
}

const std::filesystem::path& Settings::CMhx0() const
{
	const auto& ref = argumentParser.CMhx0();
	return ref.has_value() ? ref.value() : empty;
}

const std::filesystem::path& Settings::CMhy0() const
{
	const auto& ref = argumentParser.CMhy0();
	return ref.has_value() ? ref.value() : empty;
}

const std::filesystem::path& Settings::CMhz0() const
{
	const auto& ref = argumentParser.CMhz0();
	return ref.has_value() ? ref.value() : empty;
}

const std::filesystem::path& Settings::mux0() const
{
	const auto& ref = argumentParser.mux0();
	return ref.has_value() ? ref.value() : empty;
}

const std::filesystem::path& Settings::muy0() const
{
	const auto& ref = argumentParser.muy0();
	return ref.has_value() ? ref.value() : empty;
}

const std::filesystem::path& Settings::muz0() const
{
	const auto& ref = argumentParser.muz0();
	return ref.has_value() ? ref.value() : empty;
}

const std::filesystem::path& Settings::muxR0() const
{
	const auto& ref = argumentParser.muxR0();
	return ref.has_value() ? ref.value() : empty;
}

const std::filesystem::path& Settings::muyR0() const
{
	const auto& ref = argumentParser.muyR0();
	return ref.has_value() ? ref.value() : empty;
}

const std::filesystem::path& Settings::muzR0() const
{
	const auto& ref = argumentParser.muzR0();
	return ref.has_value() ? ref.value() : empty;
}

const std::filesystem::path& Settings::Ex0() const
{
	const auto& ref = argumentParser.Ex0();
	return ref.has_value() ? ref.value() : empty;
}

const std::filesystem::path& Settings::Ey0() const
{
	const auto& ref = argumentParser.Ey0();
	return ref.has_value() ? ref.value() : empty;
}

const std::filesystem::path& Settings::Ez0() const
{
	const auto& ref = argumentParser.Ez0();
	return ref.has_value() ? ref.value() : empty;
}

const std::filesystem::path& Settings::Cexe0() const
{
	const auto& ref = argumentParser.Cexe0();
	return ref.has_value() ? ref.value() : empty;
}

const std::filesystem::path& Settings::Ceye0() const
{
	const auto& ref = argumentParser.Ceye0();
	return ref.has_value() ? ref.value() : empty;
}

const std::filesystem::path& Settings::Ceze0() const
{
	const auto& ref = argumentParser.Ceze0();
	return ref.has_value() ? ref.value() : empty;
}

const std::filesystem::path& Settings::Cexh0() const
{
	const auto& ref = argumentParser.Cexh0();
	return ref.has_value() ? ref.value() : empty;
}

const std::filesystem::path& Settings::Ceyh0() const
{
	const auto& ref = argumentParser.Ceyh0();
	return ref.has_value() ? ref.value() : empty;
}

const std::filesystem::path& Settings::Cezh0() const
{
	const auto& ref = argumentParser.Cezh0();
	return ref.has_value() ? ref.value() : empty;
}

const std::filesystem::path& Settings::CEEx0() const
{
	const auto& ref = argumentParser.CEEx0();
	return ref.has_value() ? ref.value() : empty;
}

const std::filesystem::path& Settings::CEEy0() const
{
	const auto& ref = argumentParser.CEEy0();
	return ref.has_value() ? ref.value() : empty;
}

const std::filesystem::path& Settings::CEEz0() const
{
	const auto& ref = argumentParser.CEEz0();
	return ref.has_value() ? ref.value() : empty;
}

const std::filesystem::path& Settings::epsx0() const
{
	const auto& ref = argumentParser.epsx0();
	return ref.has_value() ? ref.value() : empty;
}

const std::filesystem::path& Settings::epsy0() const
{
	const auto& ref = argumentParser.epsy0();
	return ref.has_value() ? ref.value() : empty;
}

const std::filesystem::path& Settings::epsz0() const
{
	const auto& ref = argumentParser.epsz0();
	return ref.has_value() ? ref.value() : empty;
}

const std::filesystem::path& Settings::epsxR0() const
{
	const auto& ref = argumentParser.epsxR0();
	return ref.has_value() ? ref.value() : empty;
}

const std::filesystem::path& Settings::epsyR0() const
{
	const auto& ref = argumentParser.epsyR0();
	return ref.has_value() ? ref.value() : empty;
}

const std::filesystem::path& Settings::epszR0() const
{
	const auto& ref = argumentParser.epszR0();
	return ref.has_value() ? ref.value() : empty;
}

const std::filesystem::path& Settings::eyx00() const
{
	const auto& ref = argumentParser.eyx00();
	return ref.has_value() ? ref.value() : empty;
}

const std::filesystem::path& Settings::ezx00() const
{
	const auto& ref = argumentParser.ezx00();
	return ref.has_value() ? ref.value() : empty;
}

const std::filesystem::path& Settings::eyx10() const
{
	const auto& ref = argumentParser.eyx10();
	return ref.has_value() ? ref.value() : empty;
}

const std::filesystem::path& Settings::ezx10() const
{
	const auto& ref = argumentParser.ezx10();
	return ref.has_value() ? ref.value() : empty;
}

const std::filesystem::path& Settings::exy00() const
{
	const auto& ref = argumentParser.exy00();
	return ref.has_value() ? ref.value() : empty;
}

const std::filesystem::path& Settings::ezy00() const
{
	const auto& ref = argumentParser.ezy00();
	return ref.has_value() ? ref.value() : empty;
}

const std::filesystem::path& Settings::exy10() const
{
	const auto& ref = argumentParser.exy10();
	return ref.has_value() ? ref.value() : empty;
}

const std::filesystem::path& Settings::ezy10() const
{
	const auto& ref = argumentParser.ezy10();
	return ref.has_value() ? ref.value() : empty;
}

const std::filesystem::path& Settings::exz00() const
{
	const auto& ref = argumentParser.exz00();
	return ref.has_value() ? ref.value() : empty;
}

const std::filesystem::path& Settings::eyz00() const
{
	const auto& ref = argumentParser.eyz00();
	return ref.has_value() ? ref.value() : empty;
}

const std::filesystem::path& Settings::exz10() const
{
	const auto& ref = argumentParser.exz10();
	return ref.has_value() ? ref.value() : empty;
}

const std::filesystem::path& Settings::eyz10() const
{
	const auto& ref = argumentParser.eyz10();
	return ref.has_value() ? ref.value() : empty;
}

unsigned int Settings::time() const
{
	return argumentParser.time().value_or(defaultTime);
}

svec3 Settings::size() const
{
	return {sizeX(), sizeY(), sizeZ()};
}

Backend Settings::backend() const
{
	return argumentParser.backend().value_or(defaultBackend);
}

Precision Settings::precision() const
{
	return argumentParser.precision().value_or(defaultPrecision);
}

SaveAs Settings::saveAs() const
{
	return argumentParser.saveAs().value_or(defaultSaveAs);
}


}
