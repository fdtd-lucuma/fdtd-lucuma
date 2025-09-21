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

export module fdtd.services.basic;

import fdtd.utils;

export import :argument_parser;
export import :file_reader;
export import :settings;
export import :path;

namespace fdtd::utils
{
using namespace fdtd::services::basic;

extern template ArgumentParser& Injector::inject<ArgumentParser>();
extern template FileReader&     Injector::inject<FileReader>();
extern template Settings&       Injector::inject<Settings>();

}
