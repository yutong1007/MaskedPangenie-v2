# Jellyfish license and modification notes

This directory contains the GPLv3 license supplied with Jellyfish 2.2.10 and
a summary of the parser modifications included in MaskedPanGenie-v2. The
original PanGenie MIT license is retained unchanged in
[LICENSE.md](../../LICENSE.md).

## Jellyfish version and license

- Upstream project: [Jellyfish](https://github.com/gmarcais/Jellyfish).
- Dependency version: **Jellyfish 2.2.10**, specified as
  `jellyfish=2.2.10=h6bb024c_1` in [environment.yml](../../environment.yml).
- Upstream reference: tag `v2.2.10`, commit
  `da914202df293a5ab449c1907f69995879876d5c`.
- [Jellyfish-GPL-3.0.txt](Jellyfish-GPL-3.0.txt) is an unmodified copy of the
  [LICENSE supplied with Jellyfish v2.2.10](https://github.com/gmarcais/Jellyfish/blob/da914202df293a5ab449c1907f69995879876d5c/LICENSE).
  That release's parser header specifies GPL version 3 or any later version
  (`GPL-3.0-or-later`).

The build locates Jellyfish through the `jellyfish-2.0` pkg-config module;
that module name is not the dependency version. These notes describe the
2.2.10 dependency declared by the project environment. Existing license
headers in the source files are preserved; this documentation update does
not change their licensing.

## Modified parser

File: [src/mer_overlap_sequence_parser.hpp](../../src/mer_overlap_sequence_parser.hpp).

The following summary describes the current implementation, compared with
the [upstream v2.2.10 parser](https://github.com/gmarcais/Jellyfish/blob/da914202df293a5ab449c1907f69995879876d5c/include/jellyfish/mer_overlap_sequence_parser.hpp).
This comparison is a reference for reviewing the code, rather than a claim that
v2.2.10 was the exact historical base or that all differences were introduced
by the current maintainer.

| Area | Modification present in this repository |
| --- | --- |
| Parser interface | Adds the `masked` flag, `seed` argument, and optional `raw_masked` flag to the constructor. |
| Seed information | Stores the seed pattern, its span (`spaced_len`), and the number of retained positions (`spaced_weight`). |
| FASTA dispatch | Routes masked FASTA counting to `read_fasta_masked_raw()` when raw mode is enabled. The earlier `read_fasta_masked()` branch remains available for callers using the default interface. |
| Legacy extraction | The earlier branch selects bases at positions marked `1`, skips candidates with an uppercase `N` at a retained position, and appends an `N` separator after each extracted k-mer. |
| FSH iterator input | Raw mode sends sequence buffers directly to `SpacedMerIterator`, avoiding intermediate spaced k-mer strings and separators. Buffer metadata distinguishes raw masked FASTA from the other formats. |
| Input buffering | Raw mode preserves the existing 199-base chunk and physical-line boundaries for seeds fitting those chunks. Overlap storage is sized to the seed span and belongs to each stream. Seeds longer than a legacy chunk receive larger raw buffers. The old fixed-size temporary buffers remain confined to the legacy branch. |

The masked extraction branch is currently implemented for FASTA. FASTQ input
still follows `read_fastq()` and does not pass through `read_fasta_masked()`.

Other differences from the v2.2.10 reference include CR/LF handling and passing
a line-ending marker to FASTQ quality skipping. Their historical origin has
not been established, so they are recorded as observed differences rather
than attributed to this project's spaced-seed work.

## Integration with MaskedPanGenie-v2

- The modified parser is bundled in `src/mer_overlap_sequence_parser.hpp`.
  Both `src/jellyfishcounter.hpp` and `src/jellyfishreader.hpp` include this
  project-local header directly. Installation no longer requires copying it
  into Jellyfish's include directory; the external Jellyfish library is still
  required. The relocation preserves the parser implementation and its
  existing license notice.
- [src/jellyfishcounter.hpp](../../src/jellyfishcounter.hpp) creates an immutable
  FSH plan when a nonempty seed is supplied, enables raw masked-FASTA parsing,
  and selects [SpacedMerIterator](../../src/spacedmeriterator.hpp). Jellyfish's
  `COUNT`, `PRIME`, and `UPDATE` hash-table operations are retained. Without a
  seed, it uses the standard contiguous parser and iterator.
- [src/jellyfishcounter.cpp](../../src/jellyfishcounter.cpp) forwards the seed
  when counting read and graph sequences. Canonical k-mer counting remains
  enabled in these constructors.
- [src/pggtyper.cpp](../../src/pggtyper.cpp) reads the seed file from `-m`, checks
  the pattern, and supplies it to the counting workflow.

## FSH counting implementation

[src/fsh.hpp](../../src/fsh.hpp) and [src/fsh.cpp](../../src/fsh.cpp) independently
implement the single-seed method from
[Girotto, Comin and Pizzi (2018)](https://doi.org/10.1186/s13015-018-0125-4).
No source from the authors' FSH software is bundled. The plan precomputes
rank-based shifts, reusable-position masks, remaining positions to encode,
and the best prior offset available during warmup. Per-worker state reuses
packed keys and tracks ambiguous bases through the same operations.

The packed representation follows Jellyfish's base order and retains all
`2 * weight` bits, including keys using multiple 64-bit words. Canonicalization
still uses Jellyfish's reverse complement and comparison. Masked FASTA read
and graph counting use this path; FASTQ keeps its existing contiguous counting
behavior. Informative allele k-mer extraction and genotyping are unchanged.

## Modification history

- **2026-10-21:** Added the independently implemented FSH core, raw masked-FASTA
  parser mode, and packed-key counting iterator. Updated the source lists for
  the library and existing test executable. No additional test cases were
  added. Jellyfish remains at version 2.2.10; the parser's existing license
  header and the root `LICENSE.md` are unchanged.
- Earlier parser changes and relocation are summarized above. Exact source
  commits and contributor attribution can be added when historical records
  are confirmed.
