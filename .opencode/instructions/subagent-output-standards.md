# Subagent Output Standards

To minimize token usage, subagent output MUST adhere to the following standards:

1. **No Conversational Padding:** Do NOT include phrases like "Here are the files you requested", "I have found the following...", or "Let me know if you need anything else."
2. **Raw Data Only:** Return only the raw, requested data (e.g., file paths, raw file content, regex matches).
3. **Structured Format:** Use machine-readable formats where possible (lists, key-value pairs).
4. **Trimming:** If returning file content, provide only the requested lines or relevant context. Truncate verbosely if necessary.
