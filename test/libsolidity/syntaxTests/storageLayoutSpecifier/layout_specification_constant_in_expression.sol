uint constant X = 42;
contract C layout at 0xffff * (50 - X) { }
// ----
// TypeError 6396: (43-60): The storage layout can only be specified by number literals.
