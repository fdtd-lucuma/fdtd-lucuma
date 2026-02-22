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

module lucuma.utils;

import std;
import lucuma.legacy_headers.imgui_graphnode;
import imgui;

namespace lucuma::utils
{

ImguiGraphnodeEdgeWriter::ImguiGraphnodeEdgeWriter()
{}

void ImguiGraphnodeEdgeWriter::start(std::string_view name)
{
	should_write = ImGuiGraphNode::BeginNodeGraph(name.data());
}

void ImguiGraphnodeEdgeWriter::writeEdge(std::string_view l, std::string_view r)
{
	if(!should_write)
		return;

	writeNode(l);
	writeNode(r);

	ImGuiGraphNode::NodeGraphAddEdge("", l.data(), r.data());
}

void ImguiGraphnodeEdgeWriter::end()
{
	if(!should_write)
		return;

	ImGuiGraphNode::EndNodeGraph();
}

const std::string& ImguiGraphnodeEdgeWriter::writeNode(std::string_view node)
{
	std::string str(node);

	auto it = nodes.find(str);

	if(it == nodes.end())
	{
		ImGuiGraphNode::NodeGraphAddNode(node.data());
		return *nodes.emplace(std::move(str)).first;
	}
	else
	{
		return *it;
	}
}

}
