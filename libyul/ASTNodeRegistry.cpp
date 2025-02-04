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

ASTNodeRegistry::ASTNodeRegistry(): m_labels{""}, m_idToLabelMapping{0} {}

ASTNodeRegistry::ASTNodeRegistry(std::vector<std::string> _labels, std::vector<size_t> _idToLabelMapping)
{
	yulAssert(!_labels.empty());
	yulAssert(_labels[0].empty());
	yulAssert(!_idToLabelMapping.empty());
	yulAssert(_idToLabelMapping[0] == 0);
	// using vector<uint8_t> over vector<bool>, as the latter is optimized for space-efficiency
	std::vector<uint8_t> labelVisited (_labels.size(), false);
	size_t numLabels = 0;
	for (auto const& id: _idToLabelMapping)
	{
		if (id == ghostId())
			continue;
		yulAssert(id < _labels.size());
		// it is possible to have multiple references to empty / ghost
		yulAssert(
			id == 0 || !labelVisited[id],
			fmt::format("NodeId {} (label \"{}\") is not unique.", id, _labels[id])
		);
		labelVisited[id] = true;
		if (id >= 1)
			++numLabels;
	}
	yulAssert(numLabels + 1 == _labels.size(), "Unused labels present.");
	m_labels = std::move(_labels);
	m_idToLabelMapping = std::move(_idToLabelMapping);
}

ASTNodeRegistry::NodeId ASTNodeRegistry::maximumId() const
{
	yulAssert(!m_idToLabelMapping.empty());
	return m_idToLabelMapping.size() - 1;
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
	for (NodeId id = 1; id <= maximumId(); ++id)
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

ASTNodeRegistryBuilder::DefinedLabels::DefinedLabels():
	m_mapping{{"", 0}}
{}

std::tuple<ASTNodeRegistry::NodeId, bool> ASTNodeRegistryBuilder::DefinedLabels::tryInsert(
	std::string_view const _label,
	ASTNodeRegistry::NodeId const _id
)
{
	auto const [it, emplaced] = m_mapping.try_emplace(std::string{_label}, _id);
	return std::make_tuple(it->second, emplaced);
}

ASTNodeRegistryBuilder::ASTNodeRegistryBuilder():
	m_nextId(1)
{}

ASTNodeRegistryBuilder::ASTNodeRegistryBuilder(ASTNodeRegistry const& _existingRegistry)
{
	auto const maxId = _existingRegistry.maximumId();
	yulAssert(_existingRegistry[0].empty());
	for (size_t i = 1; i <= maxId; ++i)
	{
		auto const existingLabel = _existingRegistry[i];
		if (!existingLabel.empty())
		{
			if (_existingRegistry.idToLabelIndex(i) == ASTNodeRegistry::ghostId())
				m_ghosts.push_back(i);
			else
			{
				auto const [_, inserted] = m_definedLabels.tryInsert(_existingRegistry[i], i);
				yulAssert(inserted);
			}
		}
	}
	m_nextId = _existingRegistry.maximumId() + 1;
}

ASTNodeRegistry::NodeId ASTNodeRegistryBuilder::define(std::string_view const _label)
{
	auto const [id, inserted] = m_definedLabels.tryInsert(_label, m_nextId);
	if (inserted)
		m_nextId++;
	return id;
}

ASTNodeRegistry::NodeId ASTNodeRegistryBuilder::addGhost()
{
	m_ghosts.push_back(m_nextId);
	return m_nextId++;
}

ASTNodeRegistry ASTNodeRegistryBuilder::build() const
{
	auto const& labelToIdMapping = m_definedLabels.labelToIdMapping();
	yulAssert(labelToIdMapping.contains(""));
	yulAssert(labelToIdMapping.at("") == 0);

	std::vector<std::string> labels{""};
	labels.reserve(labelToIdMapping.size());
	auto const maxLabelId = ranges::max(labelToIdMapping | ranges::views::values);
	auto const maxGhostId = m_ghosts.empty() ? 0 : m_ghosts.back();
	std::vector<size_t> idToLabelMapping( std::max(maxLabelId, maxGhostId) + 1, 0);
	yulAssert(!idToLabelMapping.empty(), "Mapping must at least contain empty label");
	for (auto const& [label, id]: labelToIdMapping)
	{
		// skip empty and ghost
		if (id < 1)
			continue;

		labels.emplace_back(label);
		idToLabelMapping[id] = labels.size() - 1;
	}
	for (auto const ghostId: m_ghosts)
		idToLabelMapping[ghostId] = ASTNodeRegistry::ghostId();
	return ASTNodeRegistry{std::move(labels), std::move(idToLabelMapping)};
}
