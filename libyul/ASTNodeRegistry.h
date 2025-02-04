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

#pragma once

#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace solidity::yul
{

/// Instances of the `ASTNodeRegistry` are immutable containers describing a labelling of nodes inside the AST.
/// Each element of the AST that possesses a label has a `ASTNodeRegistry::NodeId`, with which the label can
/// be queried in O(1).
/// Preferred way of creating instances is via `ASTNodeRegistryBuilder` when parsing/importing and
/// via `NodeIdDispenser` during/after optimization.
class ASTNodeRegistry
{
public:
	/// unsafe to use from a different registry instance, it is up to the user to safeguard against this
	using NodeId = size_t;

	static constexpr auto ghostPlaceholder = "GHOST[@]";

	ASTNodeRegistry();
	ASTNodeRegistry(std::vector<std::string> _labels, std::vector<size_t> _idToLabelMapping);

	std::string_view operator[](NodeId _id) const;

	static bool constexpr empty(NodeId const _id) { return _id == emptyId(); }
	static NodeId constexpr emptyId() { return 0; }
	static NodeId constexpr ghostId() { return 1; }

	std::vector<std::string> const& labels() const { return m_labels; }
	NodeId maximumId() const { return m_idToLabelMapping.size() - 1; }

	size_t idToLabelIndex(NodeId _id) const;
	/// this is a potentially expensive operation
	std::optional<NodeId> findIdForLabel(std::string_view _label) const;
private:
	std::string_view lookupGhost(NodeId _id) const;

	/// unique labels in the container, beginning with empty ("") and ghost (ghostPlaceholder).
	std::vector<std::string> m_labels;
	/// Each element in the vector is one NodeId. The value of the vector points to the corresponding label. E.g.,
	/// m_labels[m_idToLabelMapping[3]] is the label for NodeId 3. Therefore, there can be duplicates and the lengths
	/// of `m_labels` and `m_idToLabelMapping` do not need to correspond.
	std::vector<size_t> m_idToLabelMapping;
	mutable std::map<NodeId, std::string> m_ghostLabelCache;
};

/// Produces instances of `ASTNodeRegistry`. Preferably used during parsing/importing.
class ASTNodeRegistryBuilder
{
public:
	ASTNodeRegistryBuilder();
	explicit ASTNodeRegistryBuilder(ASTNodeRegistry const& _existingRegistry);
	ASTNodeRegistry::NodeId define(std::string_view _label);
	ASTNodeRegistry build() const;
private:
	class LabelToIdMapping
	{
	public:
		LabelToIdMapping();
		std::tuple<ASTNodeRegistry::NodeId, bool> tryInsert(std::string_view _label, ASTNodeRegistry::NodeId _id);

		auto const& data() const { return m_mapping; }
	private:
		std::map<std::string, size_t, std::less<>> m_mapping;
	};

	LabelToIdMapping m_mapping;
	size_t m_nextId = 0;
};

}
