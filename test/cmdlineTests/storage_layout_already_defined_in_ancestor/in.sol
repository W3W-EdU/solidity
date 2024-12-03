//SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.0;
contract A layout at 0x1234 {
}

contract B is A {
}

contract C layout at 0x1234 is B {
}