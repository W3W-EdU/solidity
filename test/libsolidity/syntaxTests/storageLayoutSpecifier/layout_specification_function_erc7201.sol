function erc7201(bytes memory id) pure returns (uint256) {
    return uint256(
        keccak256(bytes.concat(bytes32(uint256(keccak256(id)) - 1))) &
        ~bytes32(uint256(0xff))
    );
}
contract C layout at erc7201("C") { }
// ----
// TypeError 1139: (212-224): The storage layout must be specified by an expression that can be evaluated at compilation time.
// TypeError 6396: (212-224): The storage layout can only be specified by number literals.
