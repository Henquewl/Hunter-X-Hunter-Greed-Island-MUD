# Efficiency guidelines (RPM / RPD / TPM)

Each turn is a separate API request against the Gemini free tier. To stay under Requests Per
Minute (RPM), Requests Per Day (RPD), and Tokens Per Minute (TPM) limits:

1. **Batch independent tool calls in one turn.** Whenever you need multiple independent
   operations (grep/glob searches, reading several files, status checks), issue ALL of them in a
   single response. Never spread independent tool calls across sequential turns — each turn
   costs one request.
2. **Use `task`/subagents only for genuinely multi-step or open-ended work** (broad codebase
   exploration, research spanning multiple subdirectories). For a single-step action — one grep,
   one file read, `git status`, a compile check — run the tool directly; spawning a subagent for
   a trivial action wastes a request on overhead alone.
3. **Read narrowly.** Don't read large files blindly — `grep` for the exact lines first, then
   `read` with `offset`/`limit` to pull only what's relevant.
4. **Don't re-fetch what's already in context.** If a file or command output was already
   retrieved this conversation, use it from memory instead of reading/grepping again.
5. **Answer directly.** Skip conversational filler and preambles ("I will now read the
   file..."). Return the tool call or the answer.

## Subagent output

When you run as a subagent, the caller wants raw results, not a conversation:

- No padding ("Here are the files you requested", "Let me know if you need anything else").
- Return only the raw requested data — file paths, file content, regex matches — in a
  machine-readable form (lists, key-value pairs) where that fits.
- Trim: return only the requested lines/relevant context, not whole files, unless asked for the
  whole file.
