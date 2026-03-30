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

export module lucuma.utils.imgui:progress;

import imgui;
import std;

import :text;

namespace lucuma::utils::imgui
{

export template <typename T>
requires std::is_integral_v<T>
void ProgressBar(T current_value, T max_value)
{
	float progress = (float)current_value/max_value;

	ImGui::ProgressBar(progress, ImVec2(-std::numeric_limits<float>::min(),0), StackStr<32>("{}/{}", current_value, max_value));
}

}

