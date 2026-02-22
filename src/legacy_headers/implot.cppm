
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

#include <implot.h>

export module lucuma.legacy_headers.implot;

import imgui;

export namespace ImPlot
{

using ImPlot::CreateContext;
using ImPlot::DestroyContext;
using ImPlot::ShowDemoWindow;
using ImPlot::BeginPlot;
using ImPlot::PlotHeatmap;
using ImPlot::EndPlot;
using ImPlot::SetupAxes;
using ImPlot::SetupAxesLimits;
using ImPlot::PushColormap;
using ImPlot::PopColormap;

};

export using ::ImPlotPoint;

export using ::ImPlotAxisFlags;
export using ::ImPlotFlags;
export using ::ImPlotColormap;
export using ::ImPlotHeatmapFlags;

export using ::ImPlotAxisFlags_;
export using ::ImPlotFlags_;
export using ::ImPlotColormap_;
export using ::ImPlotHeatmapFlags_;
