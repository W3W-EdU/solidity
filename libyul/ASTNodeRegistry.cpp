/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/
// SPDX-License-Identifier: GPL-3.0

#include <libyul/ASTNodeRegistry.h>

#include <libyul/Exceptions.h>

#include <fmt/format.h>

#include <range/v3/algorithm/max.hpp>
#include <range/v3/view/map.hpp>

using namespace solidity::yul;

ASTNodeRegistry::ASTNodeRegistry(): m_labels{"", ghostPlaceholder}, m_idToLabelMapping{0, 1} {}

ASTNodeRegistry::ASTNodeRegistry(std::vector<std::string> _labels, std::vector<size_t> _idToLabelMapping)
{
	yulAssert(_labels.size() >= 2);
	yulAssert(_labels[0].empty());
	yulAssert(_labels[1] == ghostPlaceholder);
	yulAssert(_idToLabelMapping.size() >= 2);
	yulAssert(_idToLabelMapping[0] == 0);
	yulAssert(_idToLabelMapping[1] == 1);
	// using vector<uint8_t> over vector<bool>, as the latter is optimized for space-efficiency
	std::vector<uint8_t> labelVisited (_labels.size(), false);
	size_t numLabels = 0;
	for (auto const& id: _idToLabelMapping)
	{
		yulAssert(id < _labels.size());
		// it is possible to have multiple references to empty / ghost
		yulAssert(
			id < 2 || !labelVisited[id],
			fmt::format("NodeId {} (label \"{}\") is not unique.", id, _labels[id])
		);
		labelVisited[id] = true;
		if (id >= 2)
			++numLabels;
	}
	yulAssert(numLabels + 2 == _labels.size(), "Unused labels present.");
	m_labels = std::move(_labels);
	m_idToLabelMapping = std::move(_idToLabelMapping);
}

size_t ASTNodeRegistry::idToLabelIndex(NodeId const _id) const
{
	yulAssert(_id < m_idToLabelMapping.size());
	return m_idToLabelMapping[_id];
}

std::string_view ASTNodeRegistry::operator[](NodeId const _id) const
{
	auto const labelIndex = idToLabelIndex(_id);
	if (labelIndex == ghostId())
		return lookupGhost(_id);
	return m_labels[labelIndex];
}

std::optional<ASTNodeRegistry::NodeId> ASTNodeRegistry::findIdForLabel(std::string_view const _label) const {
	if (_label.empty())
		return emptyId();
	if (_label == ghostPlaceholder)
		return NodeId{1};
	for (NodeId id = 2; id <= maximumId(); ++id)
		if ((*this)[id] == _label)
			return id;
	return std::nullopt;
}

std::string_view ASTNodeRegistry::lookupGhost(NodeId const _id) const
{
	yulAssert(idToLabelIndex(_id) == ghostId());
	auto const [it, _] = m_ghostLabelCache.try_emplace(_id, fmt::format("GHOST[{}]", _id));
	return it->second;
}

ASTNodeRegistryBuilder::LabelToIdMapping::LabelToIdMapping():
	m_mapping{{"", 0}, {ASTNodeRegistry::ghostPlaceholder, 1}}
{}

std::tuple<ASTNodeRegistry::NodeId, bool> ASTNodeRegistryBuilder::LabelToIdMapping::tryInsert(
	std::string_view const _label,
	ASTNodeRegistry::NodeId const _id
)
{
	yulAssert(_label != ASTNodeRegistry::ghostPlaceholder);
	auto const [it, emplaced] = m_mapping.try_emplace(std::string{_label}, _id);
	return std::make_tuple(it->second, emplaced);
}

ASTNodeRegistryBuilder::ASTNodeRegistryBuilder():
	m_nextId(2)
{}

ASTNodeRegistryBuilder::ASTNodeRegistryBuilder(ASTNodeRegistry const& _existingRegistry)
{
	for (size_t i = 2; i <= _existingRegistry.maximumId(); ++i)
	{
		auto const existingLabel = _existingRegistry[i];
		if (!existingLabel.empty())
		{
			auto const [_, inserted] = m_mapping.tryInsert(_existingRegistry[i], i);
			yulAssert(inserted);
		}
	}
	m_nextId = _existingRegistry.maximumId() + 1;
}

ASTNodeRegistry::NodeId ASTNodeRegistryBuilder::define(std::string_view const _label)
{
	auto const [id, inserted] = m_mapping.tryInsert(_label, m_nextId);
	if (inserted)
		m_nextId++;
	return id;
}

ASTNodeRegistry ASTNodeRegistryBuilder::build() const
{
	auto const& mapping = m_mapping.data();
	yulAssert(mapping.contains(""));
	yulAssert(mapping.at("") == 0);
	yulAssert(mapping.contains(ASTNodeRegistry::ghostPlaceholder));
	yulAssert(mapping.at(ASTNodeRegistry::ghostPlaceholder) == 1);

	std::vector<std::string> labels{"", ASTNodeRegistry::ghostPlaceholder};
	labels.reserve(mapping.size());
	std::vector<size_t> idToLabelMapping(ranges::max(mapping | ranges::views::values) + 1, 0);
	yulAssert(idToLabelMapping.size() >= 2, "Mapping must at least contain empty and ghost entry");
	idToLabelMapping[1] = 1;
	for (auto const& [label, id]: mapping)
	{
		// skip empty and ghost
		if (id < 2)
			continue;

		labels.emplace_back(label);
		idToLabelMapping[id] = labels.size() - 1;
	}
	return ASTNodeRegistry{std::move(labels), std::move(idToLabelMapping)};
}
