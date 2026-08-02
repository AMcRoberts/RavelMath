# Local full-text archive

`references.bib` is the canonical bibliography.  Material relied upon in
research should, when lawfully obtainable, also be preserved locally in the
adjacent `FullText/` directory so the working record does not depend on the
continued availability of a website.

`FullText/` is deliberately excluded from the public Git repository.  Some
papers permit reading and local preservation without permitting republication
in a public source tree.  The local archive is instead included in verified
full-project checkpoints under the continuity backup policy.

`FULLTEXT_MANIFEST.tsv` records the exact local filename, SHA-256 digest,
bibliography key, acquisition origin, and status.  `archived` means the file
was present and its digest was checked; `missing` means the bibliography entry
still needs a lawful local copy; `metadata-only` means no distributable or
locally archivable full text has yet been identified.  A citation is not to be
called archived merely because a URL resolves.

When a source materially informs a proof, diagnostic, or literature claim:

1. add or correct its entry in `references.bib`;
2. preserve a lawful copy in `FullText/` before relying on future network
   access;
3. compute its SHA-256 digest and update `FULLTEXT_MANIFEST.tsv`;
4. record the exact section, theorem, or pages used in the relevant research
   note; and
5. include the local archive in the next full checkpoint.

The archive is evidence, not authority: definitions and calculations should
still be independently checked, and the public work tree should remain usable
without publishing third-party full text.
