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

export module lucuma.components:gaussian_source;

import lucuma.utils.imgui;
import imgui;
import lucuma.legacy_headers.entt;
import std;

namespace lucuma::components
{

export template<typename T>
struct GaussianSource
{
	T sigma;
};

export template<typename T>
bool editor(components::GaussianSource<T>& source, entt::handle)
{
	bool updated = false;

	updated |= ImGui::InputArithmetic("Sigma", &source.sigma);

	return updated;
}

}
