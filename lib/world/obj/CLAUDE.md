# Editing `.obj` files from the CLI

The `Read` tool **fails** on `.obj` files that contain ANSI color codes (`@m`, `@g`, `@n`, etc.) — it misidentifies them as binary. Workflow:
- **Read content**: use `Grep` (works correctly).
- **Write/replace**: use WSL Python in **binary mode** to avoid encoding issues with ANSI bytes. Open with `open(path, 'rb')` / `open(path, 'wb')`. Never open in text mode (`'latin-1'` or `'utf-8'`) for writing, because the `open(..., 'w')` call truncates the file before Python raises a `UnicodeEncodeError`, leaving the file empty. Use `git checkout <file>` to recover.
- Keep all new string literals pure ASCII — em dashes (`—`), curly quotes, etc. will fail the latin-1 write path.
