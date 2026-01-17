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

#ifndef NO_XDG
#include <XdgUtils/BaseDir/BaseDir.h>
#endif

export module lucuma.legacy_headers.xdg_utils_cxx;

#ifdef NO_XDG
import std;
#endif

export namespace XdgUtils::BaseDir
{

#ifndef NO_XDG
using XdgUtils::BaseDir::Home;
using XdgUtils::BaseDir::XdgDataHome;
using XdgUtils::BaseDir::XdgConfigHome;
using XdgUtils::BaseDir::XdgCacheHome;
#else

inline std::filesystem::path Home()
{
    if (const char* home = std::getenv("HOME"))
        return home;
    return {};
}

inline std::filesystem::path XdgDataHome()
{
    if (const char* xdg = std::getenv("XDG_DATA_HOME"))
        return xdg;
    return Home() / ".local" / "share";
}

inline std::filesystem::path XdgConfigHome()
{
    if (const char* xdg = std::getenv("XDG_CONFIG_HOME"))
        return xdg;
    return Home() / ".config";
}

inline std::filesystem::path XdgCacheHome()
{
    if (const char* xdg = std::getenv("XDG_CACHE_HOME"))
        return xdg;
    return Home() / ".cache";
}

#endif

};
