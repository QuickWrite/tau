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
    | Name      | Value           | Description                                                                                           | Required | Default                  |
    |-----------|-----------------|-------------------------------------------------------------------------------------------------------|----------|--------------------------|
    | `symbols` | List of symbols | The different symbols that the Turing Machine should use.                                             | true     | ---                      |
    | `blank`   | Symbol          | The symbol that should be used on the tape by default.                                                | false    | First element in symbols |
    | `start`   | Identifier      | The state in which the Turing Machine should be starting.                                             | true     | ---                      |
    | `end`     | Identifier      | The state in which the Turing Machine should be ending <br>(does not have to be defined in the body). | false    | "HALT"                   |
    | `tape`    | List of symbols | The default initialization of the tape. <br>It can contain any specific sequence of symbols.          | false    | A list of blanks         |

- **Body**: The body contains the different states that the Turing Machine can assume. The end state does not have to be defined.

  To define a state the identifier of the state has to be written and then the logic has to be wrapped inside of curly brackets:
  ```
  NAME {
    # The logic of the state (aka. the rules)
  }
  ```

  Each rule can be defined as 
  ```
  <Curr Symbol> = <New Symbol>, <Direction>, <Next State>
  ```
  so for example this would be valid:
  ```
      1    =   0,      RIGHT,      A
  # Symbol | Symbol | Direction | State
  ```
  And it would mean: If there is a `1` on the tape, then the Turing Machine should write a `0`, go right and switch to the state with the name `A`.

  **Important:** Every single state has to be exhaustive. This means that if there are $n$ symbols defined, all $n$ possibilities have to be defined.

  If all other scenarios are the same, then the `else`-syntax can be used:
  ```
  _ = 1, LEFT, B 
  ```
  And it would mean: In any other scenario the Turing Machine should write a `1`, go left and switch to the state with the name `B`.

  ----

  A state could look like this:
  ```
  A {
    1 = 0, RIGHT, A
    _ = 1, LEFT, B
  }
  ```

> [!NOTE]
> Whitespace is nearly irrelevant. This means that constructs like this would also be possible (but isn't recommended):
> ```
> A{1=0,RIGHT,A _=1,LEFT,B}
> ```

## License
This project is licensed under the [MIT](LICENSE) license.
