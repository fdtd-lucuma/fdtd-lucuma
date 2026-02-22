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

module lucuma.services.window;

import lucuma.legacy_headers.nativefiledialog_extended;

namespace lucuma::services::window
{

using namespace lucuma::services;

Filedialog::Filedialog([[maybe_unused]] Injector& injector):
	glfw(injector.inject<Glfw>())
{
	init();
}

void Filedialog::init()
{
	if(NFD::Init() != NFD_OKAY)
		throw new std::runtime_error(NFD::GetError());

	if(!NFD_SetDisplayPropertiesFromGLFW())
		throw new std::runtime_error(NFD::GetError());

	// TODO: Parent window in wayland and X11
}

Filedialog::~Filedialog()
{
	NFD::Quit();
}

}
