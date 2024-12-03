contract A {}
contract B is A layout at 64 {}
contract C is A layout at 42 {}
// ----
// TypeError 2031: (46-77): Conflicting storage layout specifications:Storage layout for base contract 'A' was also specified by another contract which derives from it.
