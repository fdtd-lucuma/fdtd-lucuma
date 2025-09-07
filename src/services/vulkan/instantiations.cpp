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

module fdtd.services.vulkan;

// Explicit template instantiations for faster compilation
namespace fdtd::utils
{
using namespace fdtd::services::vulkan;

//TODO: Find a way to automate this

template Device&            Injector::inject<Device>();
template All&               Injector::inject<All>();
template Allocator&         Injector::inject<Allocator>();
template Context&           Injector::inject<Context>();
template Core&              Injector::inject<Core>();
template Debug&             Injector::inject<Debug>();
template DebugRequirements& Injector::inject<DebugRequirements>();
template PipelineBuilder&   Injector::inject<PipelineBuilder>();
template ShaderLoader&      Injector::inject<ShaderLoader>();

template Device&            Injector::emplace<Device>(Injector&);
template All&               Injector::emplace<All>(Injector&);
template Allocator&         Injector::emplace<Allocator>(Injector&);
template Context&           Injector::emplace<Context>(Injector&);
template Core&              Injector::emplace<Core>(Injector&);
template Debug&             Injector::emplace<Debug>(Injector&);
template DebugRequirements& Injector::emplace<DebugRequirements>(Injector&);
template PipelineBuilder&   Injector::emplace<PipelineBuilder>(Injector&);
template ShaderLoader&      Injector::emplace<ShaderLoader>(Injector&);

}

namespace fdtd::services::vulkan
{
//TODO: Find a way to automate this

template std::vector<vk::DescriptorSetLayout> unraii(std::span<vk::raii::DescriptorSetLayout>);

}

namespace std
{
using namespace fdtd::services::vulkan;

template class std::span<Buffer>;
template class std::span<vk::raii::DescriptorSetLayout>;

template class std::vector<vk::DescriptorSetLayout>;

}
