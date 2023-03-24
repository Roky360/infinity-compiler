# Infinity Compiler

Compiler from the "Infinity" imaginary language to Linux 32-bit Assembly, written in C.

Language syntax (BNF) can be found in the `syntax.bnf` file, and as text form in `syntax.txt`.

## Language Contents
* **Data types:**
    * `int`
    * `char`
    * `bool`
    * `string`
    * `void` (as a return type)
* **Intuitive arithmetic expressions**. Including all the basic operators, power (^) and factorial (!); boolean operators written like in Python.
* **if** statements (if, if-else)
* **while** statements
* **loop** statement (similar to `for` loop)
* **functions** and function calls
* **swap** statement (swaps the values of two variables of the same type)
* **start** statement - allows to define the entry point of the application
* **Built-in functions:**
  * `print`
  * `println`
  * `exit`
