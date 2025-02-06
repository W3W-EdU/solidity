contract C layout at 7 {
    int transient x;
    int y;
    uint transient w;
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
// f() -> 0, 0
// g() -> 7, 0
// h() -> 8, 0
