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
| Parser interface | Adds the `masked` flag and `seed` argument to the constructor. |
| Seed information | Stores the seed pattern, its span (`spaced_len`), and the number of retained positions (`spaced_weight`). |
| FASTA dispatch | Routes FASTA input to `read_fasta_masked()` when masked processing is enabled. |
| Spaced k-mer extraction | Slides a seed-length window over the input and selects the bases at positions marked `1`. An uppercase `N` at a retained position causes that candidate to be skipped. |
| Iterator input | Appends an `N` separator after each extracted spaced k-mer, so the existing Jellyfish iterator can process the extracted strings separately. |
| Input buffering | Adds `read_sequence_masked()`, a 200-byte temporary sequence buffer, and a 100-byte `fsmp` buffer for carrying trailing bases between full input chunks. |

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
- [src/jellyfishcounter.hpp](../../src/jellyfishcounter.hpp) passes the masked
  processing flag and seed to the parser, while retaining Jellyfish's iterator
  and `COUNT`, `PRIME`, and `UPDATE` hash-table operations.
- [src/jellyfishcounter.cpp](../../src/jellyfishcounter.cpp) forwards the seed
  when counting read and graph sequences. Canonical k-mer counting remains
  enabled in these constructors.
- [src/pggtyper.cpp](../../src/pggtyper.cpp) reads the seed file from `-m`, checks
  the pattern, and supplies it to the counting workflow.

The parser currently reconstructs each spaced k-mer by scanning the seed;
Fast Spaced-seed Hashing (FSH) has not yet been integrated.

## Modification history

The summary above records existing code; this documentation change does not
modify the parser or counting implementation. Exact source commits, change
dates, and contributor attribution can be added when the historical records
are confirmed.
