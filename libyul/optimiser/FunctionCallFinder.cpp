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

#include <libyul/optimiser/FunctionCallFinder.h>

#include <libyul/optimiser/ASTWalker.h>
#include <libyul/AST.h>
#include <libyul/Dialect.h>

#include <libsolutil/Visitor.h>

#include <variant>

using namespace solidity;
using namespace solidity::yul;

namespace
{
template<typename Base, typename ResultType>
class MaybeConstFunctionCallFinder: Base
{
public:
	using MaybeConstBlock = std::conditional_t<std::is_const_v<ResultType>, Block const, Block>;
	static std::vector<ResultType*> run(MaybeConstBlock& _block, FunctionName const& _functionName, Dialect const& _dialect)
	{
		MaybeConstFunctionCallFinder functionCallFinder(_functionName, _dialect);
		functionCallFinder(_block);
		return functionCallFinder.m_calls;
	}
private:
	explicit MaybeConstFunctionCallFinder(FunctionName const& _functionName, Dialect const& _dialect):
		m_dialect(_dialect), m_functionName(_functionName), m_calls() {}

	using Base::operator();
	void operator()(ResultType& _functionCall) override
	{
		Base::operator()(_functionCall);
		std::visit(util::GenericVisitor{
			[&](Identifier const& _identifier)
			{
				if (std::holds_alternative<Identifier>(m_functionName)
					&& _identifier.name == std::get<Identifier>(m_functionName).name
				)
					m_calls.emplace_back(&_functionCall);
			},
			[&](BuiltinName const& _builtinName)
			{
				if (std::holds_alternative<BuiltinName>(m_functionName) &&
					_builtinName.handle == std::get<BuiltinName>(m_functionName).handle
				)
					m_calls.emplace_back(&_functionCall);
			}
		}, _functionCall.functionName);
	}
	Dialect const& m_dialect;
	FunctionName const& m_functionName;
	std::vector<ResultType*> m_calls;
};
}

std::vector<FunctionCall*> yul::findFunctionCalls(Block& _block, FunctionName const& _functionName, Dialect const& _dialect)
{
	return MaybeConstFunctionCallFinder<ASTModifier, FunctionCall>::run(_block, _functionName, _dialect);
}

std::vector<FunctionCall const*> yul::findFunctionCalls(Block const& _block, FunctionName const& _functionName, Dialect const& _dialect)
{
	return MaybeConstFunctionCallFinder<ASTWalker, FunctionCall const>::run(_block, _functionName, _dialect);
}
