---
applyTo: '**'
---

# Meeting Note Generation Instructions

These instructions outline how to generate meeting notes for the Capstone Project. These notes are critical for project management and assessment.

## Core Requirements
1.  **Regularity**: Notes must be generated for every regular meeting with the supervisor.
2.  **Individual Work**: Even if working in a group, the notes must reflect the *individual's* perspective, tasks, and understanding.
3.  **Supervisor Feedback**: Explicitly record feedback received on project proposals, interim reports, or code.
4.  **Format**: The output must be clean Markdown, suitable for export to PDF.
5.  **Submission Targets**:
    *   **Interim Phase (Sept - Dec)**: Minimum 4 sets.
    *   **Final Phase (Jan - Apr)**: Minimum 4 sets.

## Content Structure
When asked to write a meeting note, ensure the following sections are included:

### 1. Header Information
*   **Meeting Number**: (e.g., Meeting #1)
*   **Date & Time**:
*   **Location**: (Physical room or Online platform)
*   **Attendees**: List all present, marking the Supervisor clearly.

### 2. Agenda / Objective
*   Briefly state the purpose of the meeting (e.g., "Review Project Proposal", "Discuss Algorithm Implementation").

### 3. Progress Update
*   Summarize what has been achieved since the last meeting.
*   *Tip: Be specific about completed tasks.*

### 4. Discussion & Supervisor Feedback (Crucial)
*   Record the main points discussed.
*   **MUST INCLUDE**: A dedicated subsection or clear bullet points for **Supervisor's Feedback**.
    *   *Example: "Supervisor suggested refining the scope of the database schema..."*
*   If applicable, note differences in opinion and how they were resolved.

### 5. Problems Encountered
*   List technical hurdles, resource constraints, or theoretical gaps encountered.
*   Describe how these were discussed or solved during the meeting.

### 6. Action Items (The "To-Do" List)
*   Clear, actionable tasks to be completed before the next meeting.
*   Assign owners if it's a group project (though focus remains on the user).
*   Set deadlines if discussed.

### 7. Next Meeting
*   Tentative date/time.
*   Planned agenda items.

## Standard Template
Use the following Markdown structure for consistency:

```markdown
# Meeting Note [Number]

**Date:** YYYY-MM-DD
**Time:** HH:MM - HH:MM
**Location:** [Location]
**Attendees:** [Student Name], [Supervisor Name]

## 1. Objectives
*   [Objective 1]
*   [Objective 2]

## 2. Progress Report
*   [Completed Task A]
*   [Completed Task B]

## 3. Discussion & Feedback
### Key Discussion Points
*   [Point 1]
*   [Point 2]

### Supervisor Feedback
> [Quote or summary of specific feedback regarding the proposal/project]

## 4. Issues & Challenges
*   [Problem description] - [Status: Resolved/Pending]

## 5. Action Items
- [ ] [Action 1] (Due: [Date])
- [ ] [Action 2] (Due: [Date])

## 6. Next Meeting Plan
*   **Target Date:** [Date]
*   **Focus:** [Topic]
```

## Best Practices for the AI
*   **Tone**: Professional, objective, and concise.
*   **Clarity**: Use bullet points for readability.
*   **Verification**: Ask the user if they want to add specific feedback details if they haven't provided them in the prompt.
*   **PDF Readiness**: Ensure the Markdown renders cleanly (avoid complex HTML).
