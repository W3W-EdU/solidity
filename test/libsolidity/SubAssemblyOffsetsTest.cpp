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
 * @date 2025
 * Unit tests for subassembly offset support
 */

#include <test/Metadata.h>
#include <test/Common.h>
#include <libsolidity/interface/CompilerStack.h>
#include <libsolidity/interface/Version.h>
#include <libsolutil/SwarmHash.h>
#include <libsolutil/IpfsHash.h>
#include <libsolutil/JSON.h>

#include <boost/test/unit_test.hpp>

namespace solidity::frontend::test
{

BOOST_AUTO_TEST_SUITE(SubAssemblyOffsets)

BOOST_AUTO_TEST_CASE(test_contract) {
	char const* sourceCode = R"(
		pragma solidity >=0.0;
		contract test {
			function g(function(uint) external returns (uint) x) public {}
		}
	)";

	for (auto metadataFormat: std::set<CompilerStack::MetadataFormat>{
		CompilerStack::MetadataFormat::NoMetadata,
		CompilerStack::MetadataFormat::WithReleaseVersionTag,
		CompilerStack::MetadataFormat::WithPrereleaseVersionTag
	})
		for (auto metadataHash: std::set<CompilerStack::MetadataHash>{
			CompilerStack::MetadataHash::IPFS,
			CompilerStack::MetadataHash::Bzzr1,
			CompilerStack::MetadataHash::None
		}) {
			CompilerStack compilerStack;
			compilerStack.setMetadataFormat(metadataFormat);
			compilerStack.setSources({{"", sourceCode}});
			compilerStack.setEVMVersion(solidity::test::CommonOptions::get().evmVersion());
			compilerStack.setOptimiserSettings(solidity::test::CommonOptions::get().optimize);
			compilerStack.setMetadataHash(metadataHash);
			BOOST_REQUIRE_MESSAGE(compilerStack.compile(), "Compiling contract failed");
			evmasm::LinkerObject const& linkerObject = compilerStack.object("test");

			BOOST_REQUIRE(linkerObject.subAssemblyData.size() == 1);
			evmasm::LinkerObject::Structure const& rootObject = linkerObject.subAssemblyData[0];
			BOOST_REQUIRE(linkerObject.bytecode.size() == rootObject.length);
			BOOST_REQUIRE(rootObject.start == 0);
			BOOST_REQUIRE(rootObject.isCreation == true);
			BOOST_REQUIRE(rootObject.subAssemblies.size() == 1);
			evmasm::LinkerObject::Structure const& subAssemblyObject = rootObject.subAssemblies[0];
			BOOST_REQUIRE(subAssemblyObject.isCreation == false);
			BOOST_REQUIRE(subAssemblyObject.start + subAssemblyObject.length == rootObject.length);
			BOOST_REQUIRE(subAssemblyObject.subAssemblies.empty());
		}
}

BOOST_AUTO_TEST_SUITE_END()

}
