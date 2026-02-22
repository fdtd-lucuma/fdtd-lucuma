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

export module lucuma.services.basic:settings;

import lucuma.utils;
import std;
import glm;

namespace lucuma::services::basic
{

using namespace lucuma::utils;

class ArgumentParser;

export class Settings
{
public:
	Settings(Injector& injector);

	bool isHeadless() const;
	bool debug() const;
	bool tracy() const;

	static constexpr std::size_t defaultSizeX = 128;
	static constexpr std::size_t defaultSizeY = 128;
	static constexpr std::size_t defaultSizeZ = 128;

	static constexpr unsigned int defaultTime = 300;

	static constexpr std::size_t defaultGaussX = 64;
	static constexpr std::size_t defaultGaussY = 64;
	static constexpr std::size_t defaultGaussZ = 64;

	static constexpr float defaultDeltaT = 1;

	static constexpr float defaultDeltaX = 1;
	static constexpr float defaultDeltaY = 1;
	static constexpr float defaultDeltaZ = 1;

	static constexpr float defaultImp0 = 377;
	//static constexpr float defaultCr = 1.f/std::sqrt(3.f);
	static constexpr float defaultGaussSigma = 10;

	static const std::filesystem::path empty;

	static constexpr Backend   defaultBackend   = Backend::sequential;
	static constexpr Precision defaultPrecision = Precision::f32;
	static constexpr SaveAs    defaultSaveAs    = SaveAs::none;

	std::size_t sizeX() const;
	std::size_t sizeY() const;
	std::size_t sizeZ() const;

	std::size_t gaussX() const;
	std::size_t gaussY() const;
	std::size_t gaussZ() const;

	float deltaT() const;

	float deltaX() const;
	float deltaY() const;
	float deltaZ() const;

	float imp0() const;
	float Cr() const;
	float gaussSigma() const;

	const std::filesystem::path& Hx0() const;
	const std::filesystem::path& Hy0() const;
	const std::filesystem::path& Hz0() const;

	const std::filesystem::path& Chxh0() const;
	const std::filesystem::path& Chyh0() const;
	const std::filesystem::path& Chzh0() const;

	const std::filesystem::path& Chxe0() const;
	const std::filesystem::path& Chye0() const;
	const std::filesystem::path& Chze0() const;

	const std::filesystem::path& CMhx0() const;
	const std::filesystem::path& CMhy0() const;
	const std::filesystem::path& CMhz0() const;

	const std::filesystem::path& mux0() const;
	const std::filesystem::path& muy0() const;
	const std::filesystem::path& muz0() const;

	const std::filesystem::path& muxR0() const;
	const std::filesystem::path& muyR0() const;
	const std::filesystem::path& muzR0() const;

	const std::filesystem::path& Ex0() const;
	const std::filesystem::path& Ey0() const;
	const std::filesystem::path& Ez0() const;

	const std::filesystem::path& Cexe0() const;
	const std::filesystem::path& Ceye0() const;
	const std::filesystem::path& Ceze0() const;

	const std::filesystem::path& Cexh0() const;
	const std::filesystem::path& Ceyh0() const;
	const std::filesystem::path& Cezh0() const;

	const std::filesystem::path& CEEx0() const;
	const std::filesystem::path& CEEy0() const;
	const std::filesystem::path& CEEz0() const;

	const std::filesystem::path& epsx0() const;
	const std::filesystem::path& epsy0() const;
	const std::filesystem::path& epsz0() const;

	const std::filesystem::path& epsxR0() const;
	const std::filesystem::path& epsyR0() const;
	const std::filesystem::path& epszR0() const;

	const std::filesystem::path& eyx00() const;
	const std::filesystem::path& ezx00() const;
	const std::filesystem::path& eyx10() const;
	const std::filesystem::path& ezx10() const;

	const std::filesystem::path& exy00() const;
	const std::filesystem::path& ezy00() const;
	const std::filesystem::path& exy10() const;
	const std::filesystem::path& ezy10() const;

	const std::filesystem::path& exz00() const;
	const std::filesystem::path& eyz00() const;
	const std::filesystem::path& exz10() const;
	const std::filesystem::path& eyz10() const;

	unsigned int time() const;

	svec3 size() const;

	Backend   backend()   const;
	Precision precision() const;
	SaveAs    saveAs()    const;

private:
	ArgumentParser& argumentParser;

};

}
