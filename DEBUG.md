# In-game debugger (`mod_debug`)

`mod_debug` is an overlay console for **Bennu processes**, not a C debugger. You can pause the game, step Bennu statements, inspect and change variables, and set breakpoints on process instances or process types.

There is **no step into / step out** of native module calls (`write`, `set_mode`, …). Those run as a single syscall.

## Requirements

The console is installed only when **all** of the following are true:

1. The program imports the module:

   ```
   import "mod_debug"
   ```

   `mod_debug` depends on `libkey` and `librender` (pulled in through video/key modules in a typical game).

2. The DCB is compiled **with debug info** (`bgdc -g`). That embeds source files in the DCB (`NSourceFiles`). Without `-g`, **Alt+C does nothing**.

3. You are running that DCB with `bgdi` (or a static/shared build that includes `mod_debug`).

`EXIT()` lives in `mod_proc`, not in `mod_debug`. Import `mod_proc` if you call `exit()`.

## Compile and run

```shell
bgdc -g -o game.dcb game.prg
bgdi game.dcb
```

After a local CMake static build:

```shell
build-static/core/bgdc/src/bgdc -g -o debug.dcb platforms/web/demo/debug.prg
build-static/core/bgdi/src/bgdi debug.dcb
```

Shared builds need the usual `modules/` plugins next to the interpreter.

## Minimal example

```
import "mod_video"
import "mod_key"
import "mod_text"
import "mod_proc"
import "mod_debug"

PROCESS Main()
PRIVATE
    int n = 0;
BEGIN
    set_mode(320, 240, 16);
    set_fps(30, 0);
    enemy();
    DEBUG;          // optional: break as soon as this line runs
    LOOP
        n++;
        write(0, 160, 16, 1, "n=" + n);
        IF (key(_esc))
            exit();
        END
        FRAME;
    END
END

PROCESS enemy()
PRIVATE
    int hp = 10;
BEGIN
    LOOP
        hp--;
        IF (hp <= 0)
            hp = 10;
        END
        FRAME;
    END
END
```

`DEBUG;` in source emits a `MN_DEBUG` opcode and stops in the console (same idea as a breakpoint in the `.prg`).

The language function `TRACE(n)` is **not** stepping. It only sets the interpreter log level (`DEBUG_LEVEL`). Stepping is the console command `TRACE` / **F8**.

## Open the console

| Key | Action |
|-----|--------|
| **Alt+C** | Open or close the console. Processes pause; input and drawing still run so the overlay can update. |
| **Alt+X** | Force-quit the program (`must_exit`). |

While the console is open, type a command on the `>` line and press **Enter**. **Esc** clears the input. **Up/Down** walk command history.

## Stepping

| Command | Key | Effect |
|---------|-----|--------|
| `GO` | **F5** | Resume until the next breakpoint, `DEBUG;`, or Alt+C. |
| `TRACE` | **F8** | Execute **one Bennu statement** (`MN_SENTENCE` from `-g`) and stop. |
| `NEXTFRAME` | **F10** | Run until the current frame finishes (every process has reached `FRAME`). |
| `NEXTPROC` | **F11** | Run until the **next process** in the scheduler starts executing. |

Typical session:

1. Start the game, press **Alt+C**.
2. `instances` (or **F2**) and note process ids (`Main` is often `65535`).
3. `break enemy` then **F5**, or place `DEBUG;` in the `.prg`.
4. **F8** to walk source lines. The overlay prints the process name, id, and the stored source line.
5. `locals <id>` / expressions such as `123.hp = 1`.

If the current process is waiting on a child, **F8** stays on that process until it runs again. Use **F11** or a breakpoint on the child to follow it. There is no “step out”.

## Breakpoints

Breakpoints are per **instance** or **process type**, not per source line.

| Command | Effect |
|---------|--------|
| `BREAK` | List current breakpoints. |
| `BREAK 123` | Break on instance id `123`. |
| `BREAK Player` | Break on every instance of type `Player`. |
| `BREAKALL` / `DELETEALL` | All instances. |
| `BREAKALLTYPES` / `DELETEALLTYPES` | All process types. |
| `DELETE 123` / `DELETE Player` | Clear one breakpoint. |

Hold **Shift** to open the type/instance list:

| Key | Effect |
|-----|--------|
| **Shift+arrows** | Scroll that list. |
| **Shift+F6** / **F6** (with Shift held) | Toggle types vs instances. |
| **F9** (with Shift) | Toggle breakpoint on the selected row. |
| **Shift+F2** | Brief instance list in the console. |
| **Shift+F3 / F4 / F5** | Locals / privates / publics of the selected instance. |

## Inspection and control

| Command | Effect |
|---------|--------|
| `HELP` | **F1** — command summary. |
| `INSTANCES` | **F2** — process tree. |
| `GLOBALS` | Global variables. |
| `LOCALS proc` | Locals (`proc` = id or type name). |
| `PRIVATES proc` | Privates of a live instance. |
| `PUBLICS proc` | Publics of a live instance. |
| `STRINGS` | String heap. |
| `VARS` | Engine console vars (`SHOW_COLOR`, `FILES`, `DEBUG_LEVEL`). |
| `SHOW expr` | Watch an expression (drawn on screen). |
| `SHOW` / `SHOWDEL id` / `SHOWDELALL` | List or remove watches. |
| `RUN proc [args]` | Spawn a process. |
| `KILL` / `SLEEP` / `WAKEUP` / `FREEZE` `[proc]` | One instance. |
| `KILLALL` / `SLEEPALL` / `WAKEUPALL` / `FREEZEALL` `[proc]` | Matching instances. |
| `QUIT` | Exit the program. |

You can evaluate expressions on the `>` line and assign through `.`:

```
65535.n
MAIN.n = 99
enemy.hp
```

Use a numeric id when several instances share a type.

## Console keys

| Key | Action |
|-----|--------|
| **Page Up / Page Down** | Scroll console history. |
| **Ctrl+arrows** | Scroll console / history. |
| **Alt+arrows** | Resize the console (line/column count). |
| **Shift+arrows** | Scroll the process/type list (see above). |

## If Alt+C does nothing

- The DCB was compiled **without** `-g`.
- `import "mod_debug"` is missing.
- You are not running the DCB you just compiled.

Rebuild **`bgdi`** after changes to `modules/mod_debug/`; rebuilding only the `.prg` is not enough for engine/console fixes.
