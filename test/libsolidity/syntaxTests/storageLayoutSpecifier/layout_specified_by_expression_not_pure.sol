function f(uint x) returns (uint) { return x + 1; }
contract A layout at f(2) {}
// ----
// TypeError 1139: (73-77): The storage layout must be specified by an expression that can be evaluated at compilation time.
// TypeError 6396: (73-77): The storage layout can only be specified by number literals.
