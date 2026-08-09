# Lean file signatures

This directory holds the public certificate material for the Lean
files in `..`. Private keys and passphrases are supplied externally through
`RAVEL_SIGNING_KEY_DIR` and never appear in this work tree.

## Files

- `MANIFEST.txt` — SHA-256 fingerprints of every `*.lean` file in
  `..`, in alphabetical order, taken after the attribution headers
  were added. The hashes therefore cover the file as signed.
- `MANIFEST.txt.sig` — GPG detached signature of `MANIFEST.txt`.
  Made with the Ravel signing key (fingerprint
  `C254E9BC303372F75B33038C3F3D3FD47F09D7E5`).
- `MANIFEST.txt.sig.pem` — X.509 detached signature of `MANIFEST.txt`.
  Made with the Ravel self-signed certificate.
- `ravel-signing-key.pub.asc` — the GPG public key in ASCII armor.
  Anyone can `gpg --import` this to verify the GPG signatures.
- `ravel-cert.pem` — the X.509 self-signed certificate (CN =
  Ravel RavelMathPub). Public material.
- `ravel-cert.pub` — the X.509 public key extracted from the
  certificate. Used by `openssl pkeyutl -verify`.
- `../*.lean.sig` — per-file GPG detached signatures. One per
  `*.lean` file.

## Layered certificate

This directory produces a layered certificate:

1. **Manifest file** — the canonical list of file hashes.
2. **GPG signature** — proves the manifest was authored by Ravel.
3. **X.509 signature** — independently proves the same manifest was
   authored by Ravel, using a different key algorithm (Ed25519 in
   both cases, but a X.509 key separate from the OpenPGP key).
4. **Per-file GPG signatures** — let a reader verify an individual
   file without trusting the manifest.

A reader can therefore verify:

- that the manifest was signed by the same Ravel on both GPG and
  X.509 layers (defense in depth against one key being compromised);
- that the manifest's hashes match the files actually in the tree
  (against tampering);
- that each individual file was signed by Ravel (against manifest
  substitution).

## Verification

```sh
# GPG layer
gpg --import ravel-signing-key.pub.asc
gpg --verify MANIFEST.txt.sig MANIFEST.txt
for f in ../*.lean; do
    gpg --verify "${f}.sig" "$f"
done

# X.509 layer
openssl x509 -in ravel-cert.pem -noout -text
openssl pkeyutl -verify -pubin -inkey ravel-cert.pub \
    -in MANIFEST.txt \
    -sigfile MANIFEST.txt.sig.pem

# Cross-check: recompute the hashes and diff against the manifest
sha256sum ../*.lean | diff - MANIFEST.txt
```

## Generation

A future session can re-sign with the private material named by
`RAVEL_SIGNING_KEY_DIR`.

## Author and date

Author: Ravel.
Architect: AM (project coordinator and project architect).
Date:   2026-07-29 (creation).
Project: RavelMathPub.
The signing key is authorized in `AUTHORS.md` at the work repository root.
