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

#include <jlcxx/jlcxx.hpp>

#include "fields.hpp"

import lucuma.julia;
import lucuma.utils;

namespace
{

using lucuma::julia::SimParams;
using lucuma::julia::SimEngine;
using lucuma::utils::Backend;
using lucuma::utils::Precision;
using lucuma::utils::SaveAs;

}

JLCXX_MODULE define_julia_module(jlcxx::Module& mod)
{
	// --- enums -------------------------------------------------------------
	mod.add_bits<Backend>("Backend", jlcxx::julia_type("CppEnum"));
	mod.set_const("Sequential", Backend::sequential);
	mod.set_const("Taskflow",   Backend::taskflow);
	mod.set_const("Vulkan",     Backend::vulkan);

	mod.add_bits<Precision>("Precision", jlcxx::julia_type("CppEnum"));
	mod.set_const("F16", Precision::f16);
	mod.set_const("F32", Precision::f32);
	mod.set_const("F64", Precision::f64);

	mod.add_bits<SaveAs>("SaveAs", jlcxx::julia_type("CppEnum"));
	mod.set_const("NoSave",    SaveAs::none);
	mod.set_const("PlainText", SaveAs::plain_text);

	// --- SimParams ------------------------------------------------------
	auto createInfo = mod.add_type<SimParams>("SimParams");
	createInfo.constructor<>();

	createInfo.method("set_size",            &SimParams::setSize);
	createInfo.method("set_gauss_position",  &SimParams::setGaussPosition);
	createInfo.method("set_delta_t",         &SimParams::setDeltaT);
	createInfo.method("set_delta_x",         &SimParams::setDeltaX);
	createInfo.method("set_delta_y",         &SimParams::setDeltaY);
	createInfo.method("set_delta_z",         &SimParams::setDeltaZ);
	createInfo.method("set_imp0",            &SimParams::setImp0);
	createInfo.method("set_cr",              &SimParams::setCr);
	createInfo.method("set_max_time",        &SimParams::setMaxTime);
	createInfo.method("set_gauss_sigma",     &SimParams::setGaussSigma);
	createInfo.method("add_gaussian_source", &SimParams::addGaussianSource);

#define X(name) createInfo.method("set_" #name, &SimParams::set_##name);
	LUCUMA_JULIA_PATH_FIELDS(X)
#undef X

	// --- SimEngine --------------------------------------------------------
	auto engine = mod.add_type<SimEngine>("SimEngine");
	engine.constructor<>();

	engine.method("use_backend",   &SimEngine::useBackend);
	engine.method("set_save_as",   &SimEngine::setSaveAs);
	engine.method("set_save_path", &SimEngine::setSavePath);
	engine.method("set_debug",     &SimEngine::setDebug);
	engine.method("init",          &SimEngine::init);
	engine.method("step",          &SimEngine::step);
	engine.method("save_files",    &SimEngine::saveFiles);
	engine.method("destroy",       &SimEngine::destroy);
}
