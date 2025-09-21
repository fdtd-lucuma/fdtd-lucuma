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

module lucuma.services.vulkan;

// Explicit template instantiations for faster compilation
namespace lucuma::utils
{
using namespace lucuma::services::vulkan;

//TODO: Find a way to automate this

template All&               Injector::inject<All>();
template Allocator&         Injector::inject<Allocator>();
template Compute&           Injector::inject<Compute>();
template Context&           Injector::inject<Context>();
template Core&              Injector::inject<Core>();
template Debug&             Injector::inject<Debug>();
template DebugRequirements& Injector::inject<DebugRequirements>();
template Device&            Injector::inject<Device>();
template ShaderLoader&      Injector::inject<ShaderLoader>();

}

namespace lucuma::services::vulkan
{
//TODO: Find a way to automate this

template std::vector<vk::DescriptorSet>      unraii(std::span<vk::raii::DescriptorSet>);
template std::vector<vk::DescriptorSetLayout> unraii(std::span<vk::raii::DescriptorSetLayout>);

}
