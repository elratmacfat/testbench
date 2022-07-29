### testbench

- C++ test library
- One header file only
- Private project (made public) with the intention of learning/improving.
    - Not meant to be used in a production environment.

#### Example output of a failed test

```
Example Testbench
-----------------
[OK]       "First testcase" (checks: 1)
[FAILED]   "Use of testing function" (checks: 2)
           #2: Expression evaluated to 'false'.
[FAILED]   "Comparison" (checks: 6)
           #1: Expected [2], but found [1].
[FAILED]   "Floating point comparison" (checks: 2)
           #1: Expected [1.0009], but found [1.0008].

FAILED
------
3/4 testcases
3/11 checks
```

#### Example output of passed test 

```
Testbench Selftest
------------------
[OK]       "'testcase' reporting back to 'testbench' on destruction." (checks: 3)
[OK]       "operator overloads throwing exceptions" (checks: 3)
[OK]       "equal, integral types" (checks: 3)
[OK]       "equal, 3 param, integral types" (checks: 4)
[OK]       "equal, 3 param, float/double" (checks: 1)
[OK]       "less_than" (checks: 1)
[OK]       "greater_than" (checks: 1)
[OK]       "in_range" (checks: 5)
[OK]       "not_in_range" (checks: 5)
[OK]       "does_not_throw" (checks: 2)
[OK]       "throws_any()" (checks: 1)
[OK]       "throws_stdexcept()" (checks: 1)
[OK]       "throws" (checks: 1)

PASSED
------
(total: 13 testcases, 31 checks)
```
