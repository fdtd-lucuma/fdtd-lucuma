// Una GUI para fdtd
// Copyright © 2025-2026 Otreblan
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

#include "fields.hpp"

export module lucuma.julia;

import std;

import lucuma.utils;
import lucuma.components;
import lucuma.components.dtos;
import lucuma.services.basic;
import lucuma.services.backends;
import lucuma.legacy_headers.entt;

namespace lucuma::julia
{

using lucuma::utils::Backend;
using lucuma::utils::Precision;
using lucuma::utils::SaveAs;

// Mutable builder for components::FdtdDataCreateInfo.
export class SimParams
{
public:
	SimParams();

	void setSize(std::uint64_t x, std::uint64_t y, std::uint64_t z);
	void setGaussPosition(std::uint64_t x, std::uint64_t y, std::uint64_t z);

	void setDeltaT(float v);
	void setDeltaX(float v);
	void setDeltaY(float v);
	void setDeltaZ(float v);

	void setImp0(float v);
	void setCr(float v);

	void setMaxTime(unsigned int v);
	void setGaussSigma(float v);

	void addGaussianSource(std::uint64_t x, std::uint64_t y, std::uint64_t z, double sigma);

	// Optional per-field initialization files (see fields.hpp).
#define X(name) void set_##name(std::string path);
	LUCUMA_JULIA_PATH_FIELDS(X)
#undef X

	const components::FdtdDataCreateInfo& build();

private:
	components::FdtdDataCreateInfo info{};
	std::vector<components::dtos::Source> sources;
};

// Wraps IBackend's init/step/saveFiles behind integer ids, with its own Injector.
export class SimEngine
{
public:
	SimEngine();
	~SimEngine();

	SimEngine(const SimEngine&)            = delete;
	SimEngine& operator=(const SimEngine&) = delete;

	void useBackend(Backend backend, Precision precision);

	void setSaveAs(SaveAs saveAs);
	void setSavePath(std::string path);
	void setDebug(bool enabled);

	std::uint32_t init(SimParams& createInfo);

	bool step(std::uint32_t id);
	void saveFiles(std::uint32_t id);

	void destroy(std::uint32_t id);

private:
	utils::Injector injector;

	services::basic::Settings&        settings;
	services::backends::Instantiator& instantiator;
	entt::registry&                   registry;

	services::backends::IBackend* backend = nullptr;
	std::unordered_set<std::uint32_t> live;

	services::backends::IBackend& requireBackend() const;
};

}
