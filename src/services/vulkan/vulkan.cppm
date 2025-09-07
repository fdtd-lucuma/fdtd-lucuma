// Una GUI para fdtd
// Copyright © 2025 Otreblan
//
// fdtd-vulkan is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// fdtd-vulkan is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with fdtd-vulkan.  If not, see <http://www.gnu.org/licenses/>.

module;

export module fdtd.services.vulkan;

import std;
import vulkan_hpp;
import fdtd.utils;

export import :all;
export import :context;
export import :core;
export import :debug;
export import :debug_requirements;
export import :device;
export import :pipeline_builder;
export import :allocator;

import :shader_loader;
import :utils;

// Explicit template instantiations for faster compilation
namespace fdtd::utils
{
using namespace fdtd::services::vulkan;

//TODO: Find a way to automate this

extern template Device&            Injector::inject<Device>();
extern template All&               Injector::inject<All>();
extern template Allocator&         Injector::inject<Allocator>();
extern template Context&           Injector::inject<Context>();
extern template Core&              Injector::inject<Core>();
extern template Debug&             Injector::inject<Debug>();
extern template DebugRequirements& Injector::inject<DebugRequirements>();
extern template PipelineBuilder&   Injector::inject<PipelineBuilder>();
extern template ShaderLoader&      Injector::inject<ShaderLoader>();

extern template Device&            Injector::emplace<Device>(Injector&);
extern template All&               Injector::emplace<All>(Injector&);
extern template Allocator&         Injector::emplace<Allocator>(Injector&);
extern template Context&           Injector::emplace<Context>(Injector&);
extern template Core&              Injector::emplace<Core>(Injector&);
extern template Debug&             Injector::emplace<Debug>(Injector&);
extern template DebugRequirements& Injector::emplace<DebugRequirements>(Injector&);
extern template PipelineBuilder&   Injector::emplace<PipelineBuilder>(Injector&);
extern template ShaderLoader&      Injector::emplace<ShaderLoader>(Injector&);

}

namespace fdtd::services::vulkan
{

//TODO: Find a way to automate this

extern template std::vector<vk::DescriptorSetLayout> unraii(std::span<vk::raii::DescriptorSetLayout>);

}

namespace std
{
using namespace fdtd::services::vulkan;

extern template class std::span<Buffer>;
extern template class std::span<vk::raii::DescriptorSetLayout>;

extern template class std::vector<vk::DescriptorSetLayout>;

}
