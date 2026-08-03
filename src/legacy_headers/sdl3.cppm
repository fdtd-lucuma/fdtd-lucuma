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

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

export module lucuma.legacy_headers.sdl3;

import std;

export using ::SDL_Init;
export using ::SDL_Quit;
export using ::SDL_SetAppMetadata;

export using ::SDL_Event;
export using ::SDL_PollEvent;
export using ::SDL_PeepEvents;
export using ::SDL_WaitEvent;

export using ::SDL_CommonEvent;
export using ::SDL_DisplayEvent;
export using ::SDL_WindowEvent;
export using ::SDL_KeyboardDeviceEvent;
export using ::SDL_KeyboardEvent;
export using ::SDL_TextEditingEvent;
export using ::SDL_TextEditingCandidatesEvent;
export using ::SDL_TextInputEvent;
export using ::SDL_MouseDeviceEvent;
export using ::SDL_MouseMotionEvent;
export using ::SDL_MouseButtonEvent;
export using ::SDL_MouseWheelEvent;
export using ::SDL_JoyDeviceEvent;
export using ::SDL_JoyAxisEvent;
export using ::SDL_JoyBallEvent;
export using ::SDL_JoyHatEvent;
export using ::SDL_JoyButtonEvent;
export using ::SDL_JoyBatteryEvent;
export using ::SDL_GamepadDeviceEvent;
export using ::SDL_GamepadAxisEvent;
export using ::SDL_GamepadButtonEvent;
export using ::SDL_GamepadTouchpadEvent;
export using ::SDL_GamepadSensorEvent;
export using ::SDL_AudioDeviceEvent;
export using ::SDL_CameraDeviceEvent;
export using ::SDL_SensorEvent;
export using ::SDL_QuitEvent;
export using ::SDL_UserEvent;
export using ::SDL_TouchFingerEvent;
export using ::SDL_PinchFingerEvent;
export using ::SDL_PenProximityEvent;
export using ::SDL_PenTouchEvent;
export using ::SDL_PenMotionEvent;
export using ::SDL_PenButtonEvent;
export using ::SDL_PenAxisEvent;
export using ::SDL_RenderEvent;
export using ::SDL_DropEvent;
export using ::SDL_ClipboardEvent;

export using ::SDL_Window;
export using ::SDL_CreateWindow;
export using ::SDL_DestroyWindow;
export using ::SDL_GetWindowSize;
export using ::SDL_GetWindowSizeInPixels;

export using ::SDL_Vulkan_CreateSurface;
export using ::SDL_Vulkan_DestroySurface;
export using ::SDL_Vulkan_GetInstanceExtensions;
export using ::SDL_Vulkan_GetPresentationSupport;
export using ::SDL_Vulkan_GetVkGetInstanceProcAddr;
export using ::SDL_Vulkan_LoadLibrary;
export using ::SDL_Vulkan_UnloadLibrary;

export namespace sdl3
{

template <auto f>
struct deleter {
	template<class T>
	void operator()(T* p) const noexcept
	{
		f(p);
	}
};

using unique_window = std::unique_ptr<SDL_Window, deleter<SDL_DestroyWindow>>;

};
