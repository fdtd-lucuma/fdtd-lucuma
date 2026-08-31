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

#include "fields.hpp"

module lucuma.julia;

import std;

import lucuma.utils;
import lucuma.components;
import lucuma.components.dtos;
import lucuma.services.basic;
import lucuma.services.backends;
import lucuma.legacy_headers.entt;

namespace lucuma::julia {

SimParams::SimParams() = default;

void SimParams::setSize(std::uint64_t x, std::uint64_t y, std::uint64_t z) {
  info.size = utils::svec3(x, y, z);
}

void SimParams::setGaussPosition(std::uint64_t x, std::uint64_t y,
                                  std::uint64_t z) {
  info.gaussPosition = utils::svec3(x, y, z);
}

void SimParams::setDeltaT(float v) { info.deltaT = v; }
void SimParams::setDeltaX(float v) { info.deltaX = v; }
void SimParams::setDeltaY(float v) { info.deltaY = v; }
void SimParams::setDeltaZ(float v) { info.deltaZ = v; }

void SimParams::setImp0(float v) { info.imp0 = v; }
void SimParams::setCr(float v) { info.Cr = v; }

void SimParams::setMaxTime(unsigned int v) { info.maxTime = v; }
void SimParams::setGaussSigma(float v) { info.gaussSigma = v; }

void SimParams::addGaussianSource(std::uint64_t x, std::uint64_t y,
                                   std::uint64_t z, double sigma) {
  sources.push_back(components::dtos::Source{
      .type = utils::SourceType::GAUSSIAN,
      .source =
          components::dtos::GaussianSource{
              .position = utils::svec3(x, y, z),
              .sigma = sigma,
          },
  });
}

#define X(name)                                                                \
  void SimParams::set_##name(std::string path) {                              \
    info.name = std::filesystem::path(std::move(path));                        \
  }
LUCUMA_JULIA_PATH_FIELDS(X)
#undef X

const components::FdtdDataCreateInfo &SimParams::build() {
  info.sources = sources;
  return info;
}

namespace {

utils::Injector &primeInjector(utils::Injector &injector) {
  static char arg0[] = "fdtd-lucuma-julia";
  char *argv[] = {arg0, nullptr};

  injector.emplace<services::basic::ArgumentParser>(1, argv);
  return injector;
}

} // namespace

SimEngine::SimEngine()
    : injector(),
      settings(primeInjector(injector).inject<services::basic::Settings>()),
      instantiator(injector.inject<services::backends::Instantiator>()),
      registry(injector.inject<entt::registry>()) {}

SimEngine::~SimEngine() {
  for (auto id : live) {
    auto entity = static_cast<entt::entity>(id);

    if (registry.valid(entity))
      registry.destroy(entity);
  }

  live.clear();
}

services::backends::IBackend &SimEngine::requireBackend() const {
  if (backend == nullptr)
    throw std::runtime_error(
        "SimEngine: no backend selected, call useBackend() first");

  return *backend;
}

void SimEngine::useBackend(Backend selected, Precision precision) {
  backend = &instantiator.get(selected, precision);
}

void SimEngine::setSaveAs(SaveAs saveAs) { settings.setSaveAsOverride(saveAs); }

void SimEngine::setSavePath(std::string path) {
  std::filesystem::path dir(std::move(path));

  if (!dir.empty())
    std::filesystem::create_directories(dir);

  settings.setSavePath(std::move(dir));
}

void SimEngine::setDebug(bool enabled) { settings.setDebugOverride(enabled); }

std::uint32_t SimEngine::init(SimParams &createInfo) {
  auto &selected = requireBackend();

  auto entity = registry.create();
  selected.init(createInfo.build(), entity);

  auto id = static_cast<std::uint32_t>(entt::to_integral(entity));
  live.insert(id);

  return id;
}

bool SimEngine::step(std::uint32_t id) {
  return requireBackend().step(static_cast<entt::entity>(id));
}

void SimEngine::saveFiles(std::uint32_t id) {
  requireBackend().saveFiles(static_cast<entt::entity>(id));
}

void SimEngine::destroy(std::uint32_t id) {
  auto entity = static_cast<entt::entity>(id);

  if (registry.valid(entity))
    registry.destroy(entity);

  live.erase(id);
}

} // namespace lucuma::julia
