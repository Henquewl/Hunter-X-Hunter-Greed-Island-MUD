# Gemini Model Efficiency Guidelines (RPM / RPD & Token Optimization)

To maximize throughput and avoid hitting Requests Per Minute (RPM), Requests Per Day (RPD), or Tokens Per Minute (TPM) limits when using Gemini models:

1. **Parallel Tool Batching (Primary RPM Saver):**
   - Whenever you need to perform multiple independent tool operations (e.g. searching with `grep`/`glob`, reading multiple files, running status checks), execute ALL tool calls in parallel in a SINGLE response turn.
   - Do NOT execute independent tools across sequential turns. Each turn is an API request.

2. **Direct Tool Execution vs Subagent Overhead:**
   - For simple single-step queries, checks, or commands (e.g., reading a single file, running `git status`, or a quick `grep`, or running a compile test), execute the tool DIRECTLY in your turn.
   - Do NOT launch a subagent (`task`) for trivial single-turn actions or basic status checks, as spawning subagents introduces extra API requests that unnecessarily consume RPM and RPD.
   - Use subagents only for complex, multi-step tasks or when isolated parallel exploration is genuinely required.

3. **Concise Responses (Token & TPM Saver):**
   - Omit conversational filler, preambles (e.g., "I will now read the file..."), and unsolicited summaries after executing commands.
   - Return straight direct answers or tool calls.

4. **Targeted Reading Strategy:**
   - Never read large files blindly. Use `grep` to find exact line numbers first, then use `read` with `offset` and `limit` to retrieve only the relevant lines.

5. **Avoid Redundant Reads (Input Token & TPM Saver):**
   - NEVER read, grep, or search files or outputs that have already been fully retrieved or matched in the current conversation history.
   - Trust your short-term memory of previous tool outputs to avoid wasting input tokens and triggering unnecessary requests.
