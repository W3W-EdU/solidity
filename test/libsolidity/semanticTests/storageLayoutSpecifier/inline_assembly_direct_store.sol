contract C layout at 42 {
    uint x;
    function f() public returns (uint) {
        assembly {
            sstore(42, 16)
        }
        return x;
    }
}
// ====
// EVMVersion: >=prague
// compileViaYul: true
// ----
// f() -> 0x10
