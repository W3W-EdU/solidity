contract C layout at 42 {
    uint128 x;
    int128 y;
    uint z;
    bool b;
    address a;
    function f() public pure returns (uint, uint, uint, uint, uint) {
        uint sx;
        uint sy;
        uint sz;
        uint sb;
        uint sa;
        assembly {
            sx := x.slot
            sy := y.slot
            sz := z.slot
            sb := b.slot
            sa := a.slot
        }
        return (sx, sy, sz, sb, sa);
    }
}
// ====
// EVMVersion: >=prague
// compileViaYul: false
// ----
// f() -> 0x2a, 0x2a, 0x2b, 0x2c, 0x2c
