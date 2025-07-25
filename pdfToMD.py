import pymupdf4llm

md_text = pymupdf4llm.to_markdown(".\Why Is Dual-Pivot Quicksort Fast.pdf")

# now work with the markdown text, e.g. store as a UTF8-encoded file
import pathlib
pathlib.Path(".\Why Is Dual-Pivot Quicksort Fast.md").write_bytes(md_text.encode())