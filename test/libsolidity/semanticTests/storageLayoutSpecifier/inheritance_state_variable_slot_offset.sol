contract A {
    uint x;
    int w;
}

contract C is A layout at 7 {
    int32 y;
    uint256 z;

    function f() public returns(uint s, uint o) {
        assembly {
            s := x.slot
            o := x.offset
        }
    }
    function g() public returns(uint s, uint o) {
        assembly {
            s := y.slot
            o := y.offset
        }
    }
    function h() public returns(uint s, uint o) {
        assembly {
            s := z.slot
            o := z.offset
        }
    }
}
// ====
// EVMVersion: >=prague
// compileViaYul: true
// ----
// f() -> 7, 0
// g() -> 9, 0
// h() -> 10, 0
