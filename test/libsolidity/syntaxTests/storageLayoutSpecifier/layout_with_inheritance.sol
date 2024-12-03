contract A { }
contract B { }
contract Z { }
contract C is A, B layout at 0x1234 { }
contract D layout at 0xABCD is Z { }
// ----
