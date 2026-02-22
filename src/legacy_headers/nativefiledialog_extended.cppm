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

#include <nfd.hpp>
#include <nfd_glfw3.h>

export module lucuma.legacy_headers.nativefiledialog_extended;

export namespace NFD
{

using NFD::Init;
using NFD::Quit;

using NFD::UniquePathN;
using NFD::UniquePathSetPathN;
using NFD::UniquePathSet;

using NFD::OpenDialog;
using NFD::OpenDialogMultiple;
using NFD::SaveDialog;
using NFD::PickFolder;
using NFD::PickFolderMultiple;
using NFD::GetError;
using NFD::ClearError;

};

export using enum ::nfdresult_t;
export using ::nfdnfilteritem_t;
export using ::NFD_SetDisplayPropertiesFromGLFW;
export using ::NFD_GetNativeWindowFromGLFWWindow;
