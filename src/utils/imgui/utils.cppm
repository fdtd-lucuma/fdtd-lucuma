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

export module lucuma.utils.imgui:utils;

import imgui;
import std;

export namespace ImGui
{

bool InputText(const char* label, std::filesystem::path* path, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr);

template <typename T>
constexpr ImGuiDataType getDataType()
{
	if constexpr(std::is_same_v<T, char>)
		return ImGuiDataType_S8;
	if constexpr(std::is_same_v<T, unsigned char>)
		return ImGuiDataType_U8;
	if constexpr(std::is_same_v<T, short>)
		return ImGuiDataType_S16;
	if constexpr(std::is_same_v<T, unsigned short>)
		return ImGuiDataType_U16;
	if constexpr(std::is_same_v<T, int>)
		return ImGuiDataType_S32;
	if constexpr(std::is_same_v<T, unsigned int>)
		return ImGuiDataType_U32;
	if constexpr(std::is_same_v<T, long long>)
		return ImGuiDataType_S64;
	if constexpr(std::is_same_v<T, unsigned long long>)
		return ImGuiDataType_U64;
	if constexpr(std::is_same_v<T, float>)
		return ImGuiDataType_Float;
	if constexpr(std::is_same_v<T, double>)
		return ImGuiDataType_Double;

	return ImGuiDataType_COUNT;
}


// TODO: Steps
template <typename T>
inline bool InputArithmetic(const char* label, T* p_data, const void* p_step = nullptr, const void* p_step_fast = nullptr, const char* format = nullptr, ImGuiInputTextFlags flags = 0)
{

	if constexpr(std::is_same_v<T, _Float16>)
	{
		float data            = p_data == nullptr ? float{} : (float)*p_data;
		const float step      = p_step == nullptr ? float{} : (float)*(_Float16*)p_step;
		const float step_fast = p_step_fast == nullptr ? float{} : (float)*(_Float16*)p_step_fast;

		bool result = InputArithmetic(label, p_data == nullptr ? nullptr : &data, p_step == nullptr ? nullptr : &step, p_step_fast == nullptr ? nullptr : &step_fast, format, flags);

		if(result)
			*p_data = data;

		return result;
	}
	else
	{
		return InputScalar(label, getDataType<T>(), p_data, p_step, p_step_fast, format, flags);
	}

}

}
