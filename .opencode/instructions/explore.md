You MUST use a subagent (using the `task` tool with `subagent_type: 'explore'`) whenever you are asked to perform trivial tasks.

Trivial tasks include:
- File listing
- Pattern searching (grep/glob)
- Reading small file content
- Checking basic git status
- Running simple, non-interactive test commands

The subagent must produce minimal output, adhering to the standards in `instructions/subagent-output-standards.md`—return only the essential data without conversational padding.
