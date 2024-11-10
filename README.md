# tau
(τ) An esoteric [Turing machine](https://en.wikipedia.org/wiki/Turing_machine) programming language

> [!IMPORTANT]
> This project is still under development and not really usable. It is mostly a toy project.

## Name
The name is an abbreviation for "**T**uring **Au**tomaton" as it implements a turing machine.

## What does it do?
This project is mainly a Turing machine implemented in C and it will be controllable by using a DSL. This can be used to showcase how a Turing machine works.

### Example
For the famous busy beaver challenge, a three state busy beaver can be implemented like this (based on this [Wikipedia entry](https://en.wikipedia.org/wiki/Turing_machine#Formal_definition)):
```
symbols = 0,1
blank = 0
start = A
end = HALT

------

A {
    0 = 1, RIGHT, B
    1 = 1, LEFT, C
}

B {
    0 = 1, LEFT, A
    1 = 1, RIGHT, B
}

C {
    0 = 1, LEFT, B
    1 = 1, RIGHT, HALT
}
```

## License
This project is licensed under the [MIT](LICENSE) license.