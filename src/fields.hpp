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

#pragma once

// Optional per-field init files of FdtdDataCreateInfo, shared by the lucuma.julia module and the bindings.
#define LUCUMA_JULIA_PATH_FIELDS(X) \
	X(Hx0)   X(Hy0)   X(Hz0)      \
	X(Chxh0) X(Chyh0) X(Chzh0)    \
	X(Chxe0) X(Chye0) X(Chze0)    \
	X(CMhx0) X(CMhy0) X(CMhz0)    \
	X(mux0)  X(muy0)  X(muz0)     \
	X(muxR0) X(muyR0) X(muzR0)    \
	X(Ex0)   X(Ey0)   X(Ez0)      \
	X(Cexe0) X(Ceye0) X(Ceze0)    \
	X(Cexh0) X(Ceyh0) X(Cezh0)    \
	X(CEEx0) X(CEEy0) X(CEEz0)    \
	X(epsx0) X(epsy0) X(epsz0)    \
	X(epsxR0) X(epsyR0) X(epszR0) \
	X(eyx00) X(ezx00) X(eyx10) X(ezx10) \
	X(exy00) X(ezy00) X(exy10) X(ezy10) \
	X(exz00) X(eyz00) X(exz10) X(eyz10)
