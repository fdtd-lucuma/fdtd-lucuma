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

export template<typename>
struct is_gaussian_source : std::false_type {};

export template<typename T>
struct is_gaussian_source<lucuma::components::GaussianSource<T>> : std::true_type {};

export template<typename T>
concept GaussianSourceType = is_gaussian_source<T>::value;

}

namespace lucuma::gui
{

export template<components::GaussianSourceType T>
void componentEditor(void* v, entt::handle)
{
	ImGui::InputArithmetic("Sigma", ((T*)v)->sigma);
}

}
