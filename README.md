# updater

# Wyland Update Script Language (WUSL)

WUSL is a **tiny DSL** designed to automate file and string operations in a custom virtual machine environment.  
It focuses on **readability**, **low-level control**, and a hint of COBOL style with modern conveniences.

> Use it to create folders, download files, compare strings, and write simple logic using `IF`/`ELSE`.

---

## Features

- `MKDIR`, `MKFILE`, `DOWNLOAD` for filesystem operations  
- `SET`, `PRINT`, `READ` for working with string variables  
- `COMPARE` sets an internal `FLAGS` variable to `"true"` or `"false"`  
- Simple control flow with `IF`, `ELSE`, `ENDIF`  
- **Semicolon (`;`) is used as a statement separator**  
- **All variables are strings**  
- **No spaces allowed in file paths**

---

## Basic Syntax

```txt
SET NAME = "World";
PRINT "Hello, ";
PRINT NAME;
```

---

## Filesystem Commands

```txt
MKDIR tmp/data;
MKFILE tmp/data/notes.txt;
DOWNLOAD https://example.com/data.zip TO tmp/data/archive.zip;
```

---

## User Input

```txt
PRINT "Enter your name:";
READ USERNAME;
PRINT "Welcome, ";
PRINT USERNAME;
```

---

## String Comparison and Control Flow

```txt
READ INPUT;
COMPARE INPUT == "open_sesame";
IF FLAGS == "true";
  PRINT "Access granted.";
ELSE;
  PRINT "Access denied.";
ENDIF;
```

---

## Variables

Variables are created using `SET` and referenced by name:

```txt
SET SECRET = "rosebud";
COMPARE SECRET == "rosebud";
```

The `FLAGS` variable is automatically set by `COMPARE` to `"true"` or `"false"`.

---

## Example Program

```txt
SET WELCOME = "Hi there!";
PRINT WELCOME;
READ CODE;
COMPARE CODE == "1234";
IF FLAGS == "true";
  PRINT "Correct code. Launching rockets.";
ELSE;
  PRINT "Incorrect code. Launching ducks.";
ENDIF;
```

---

## Notes

- All keywords are **UPPERCASE**  
- Values in `SET`, `COMPARE`, and `PRINT` can be **quoted strings** or **variables**  
- `COMPARE` does **not** print anything — it only sets `FLAGS`  
- Whitespace (spaces, tabs, line breaks) is ignored where reasonable

---

## Usage

```bash
./update_script my_script.txt
```

Ensure your script uses semicolons to separate commands.  
If something breaks, the issue is likely in the script (or possibly in the tool).

---

## Hidden Message

If you’ve read this far: there are absolutely no bugs. Ever. Seriously. Not a lie.
```

---
