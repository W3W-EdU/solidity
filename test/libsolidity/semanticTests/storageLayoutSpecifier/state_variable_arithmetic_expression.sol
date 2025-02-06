contract C layout at 7 {
    uint x;
    uint y;
    uint z;

    function f(uint a) public returns (uint) {
        x = x + a;
        y = y + x;
        z = y - 2;

        return z;
    }
}
// ====
// EVMVersion: >=prague
// compileViaYul: true
// ----
// f(uint256): 2 -> 0
// f(uint256): 3 -> 5
// f(uint256): 5 -> 15