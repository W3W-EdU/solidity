contract C layout at 42 {
    uint x;
    function f() public returns (uint r) {
        x = 16;
        assembly {
            r := sload(42)
        }
    }
}
// ====
// EVMVersion: >=prague
// compileViaYul: true
// ----
// f() -> 0x10
