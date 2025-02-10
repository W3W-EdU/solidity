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
/**
 * AST walker that finds all calls to a function of a given name.
 */

#pragma once

#include <libyul/ASTForward.h>

#include <libyul/AST.h>
#include <string_view>
#include <vector>

namespace solidity::yul
{

class Dialect;

/**
 * Finds all calls to a function of a given name using an ASTModifier.
 *
 * Prerequisite: Disambiguator
 */
std::vector<FunctionCall*> findFunctionCalls(Block& _block, FunctionName const& _functionName, Dialect const& _dialect);
inline std::vector<FunctionCall*> findFunctionCalls(Block& _block, BuiltinHandle const& _builtin, Dialect const& _dialect)
{
	return findFunctionCalls(_block, BuiltinName{nullptr, _builtin}, _dialect);
}

/**
 * Finds all calls to a function of a given name using an ASTWalker.
 *
 * Prerequisite: Disambiguator
 */
std::vector<FunctionCall const*> findFunctionCalls(Block const& _block, FunctionName const& _functionName, Dialect const& _dialect);
inline std::vector<FunctionCall const*> findFunctionCalls(Block const& _block, BuiltinHandle const& _builtin, Dialect const& _dialect)
{
	return findFunctionCalls(_block, BuiltinName{nullptr, _builtin}, _dialect);
}

}
