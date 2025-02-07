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

#include <libyul/ASTNodeRegistry.h>

#include <functional>
#include <set>

namespace solidity::yul
{

struct Block;
class Dialect;

class NodeIdDispenser
{
public:
	using NodeId = ASTNodeRegistry::NodeId;
	explicit NodeIdDispenser(
		ASTNodeRegistry const& _labels,
		std::set<std::string> const& _reserved = {}
	);

	ASTNodeRegistry const& labels() const { return m_registry; }

	NodeId newId(NodeId parent = 0);
	NodeId newGhost();
	ASTNodeRegistry generateNewLabels(Block const& _root, Dialect const& _dialect) const;
private:
	NodeId resolveBaseId(NodeId _id) const;

	ASTNodeRegistry const& m_registry;
	std::set<std::string, std::less<>> m_reservedLabels;
	size_t m_offset;
	std::vector<NodeId> m_mapping;
};

}
