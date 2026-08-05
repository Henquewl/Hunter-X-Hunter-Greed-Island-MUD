For single-step actions (e.g., searching with grep, listing files with glob, reading a specific file section, checking git status), perform the action DIRECTLY using the corresponding tool to minimize API request overhead (RPM/RPD).

Use an exploration subagent (`task` with `subagent_type: 'explore'`) when handling multi-step code exploration, open-ended research, or searching across multiple subdirectories.

When an exploration subagent is launched, it must produce minimal raw output adhering to `instructions/subagent-output-standards.md` without conversational padding.
