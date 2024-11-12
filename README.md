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

The program is separated into two different parts: The **head** and the **body**. They are being separated by a `---` (this can have a variable length as long as it contains at least one `-`).

- **Head**: The head specifies different aspects of the Turing Machine:
    | Name      | Value           | Description                                               | Required | Default                  |
    |-----------|-----------------|-----------------------------------------------------------|----------|--------------------------|
    | `symbols` | List of symbols | The different symbols that the Turing Machine should use. | true     | ---                      |
    | `blank`   | Symbol          | The symbol that should be used on the tape by default     | false    | First element in symbols |
    | `start`   | Identifier      | The state                                                 | true     | ---                      |
    | `end`     | Identifier      |                                                           | false    | "HALT"                   |
    | `tape`    | List of symbols |                                                           | false    | A list of blanks         |

- **Body**: The body contains the different states that the Turing Machine can assume. The end state does not have to be defined.

## License
This project is licensed under the [MIT](LICENSE) license.