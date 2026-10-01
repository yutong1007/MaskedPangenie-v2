# MaskedPanGenie-v2

MaskedPanGenie-v2 is an optimized version of [MaskedPanGenie](https://github.com/hhaentze/MaskedPangenie), which extends PanGenie with **spaced seeds** for alignment-free genotyping.

Spaced seeds specify *care* and *don't-care* positions when extracting k-mers. Compared with contiguous k-mers, they can be more tolerant of sequencing errors. MaskedPanGenie accepts a spaced-seed pattern through the `-m` option and uses it to identify informative spaced k-mers in a pangenome graph and match them against sequencing reads.

MaskedPanGenie-v2 integrates **MaskedJellyfish-based spaced k-mer counting** into the workflow to reduce preprocessing overhead. In our experiments, this implementation achieved an approximately **2–3× speedup** over the original MaskedPanGenie; performance depends on the dataset and computational environment.

## Installation

Clone the repository:

```bash
git clone https://github.com/yutong1007/MaskedPangenie-v2.git
cd MaskedPangenie-v2
```

Create and activate the Conda environment:

```bash
conda env create -f environment.yml
conda activate pangenie
```

Replace Jellyfish's `mer_overlap_sequence_parser.hpp` with the modified header provided in this repository. Set `JELLYFISH_INCLUDE_DIR` to the directory containing the installed Jellyfish header (the exact location depends on your installation):

```bash
cp mer_overlap_sequence_parser.hpp "$JELLYFISH_INCLUDE_DIR/mer_overlap_sequence_parser.hpp"
```

Build the project:

```bash
mkdir -p build
cd build
cmake ..
make -j
cd ..
```

## Usage

Run genotyping with a spaced-seed file:

```bash
./build/src/PanGenie \
    -i <reads.fa/fq> \
    -r <reference.fa> \
    -v <variants.vcf> \
    -m <spacedSeed.txt> \
    -t <genotyping-threads> \
    -j <counting-threads>
```

The output is a VCF containing predicted genotypes, genotype likelihoods, and genotype quality scores. Use `-o <prefix>` to specify an output prefix. For additional options, run `./build/src/PanGenie --help`.

By default, the genotyping output is `result_genotyping.vcf`. Specify `-o <prefix>` to change the output prefix. MaskedPanGenie-v2 additionally supports the spaced-seed option `-m` shown above.

Common options:

| Option | Description |
| --- | --- |
| `-i` | Input sequencing reads in FASTA/FASTQ format. |
| `-r` | Reference genome in FASTA format. |
| `-v` | Pangenome variants in VCF format. |
| `-m` | Spaced-seed pattern file (MaskedPanGenie extension). |
| `-j` | Number of threads for k-mer counting (default: 1). |
| `-t` | Number of threads for genotyping (default: 1). |
| `-k` | K-mer weight/size, as supported by the executable (default: 31). |
| `-e` | Jellyfish hash size. |
| `-o` | Output file prefix (default: `result`). |
| `-p` | Run experimental phasing using the Viterbi algorithm. |
| `-s` | Output sample name (default: `sample`). |

Use `./build/src/PanGenie --help` for the full option list supported by the installed version.

## Runtime and Memory Usage

Runtime and memory usage depend on the number of variants, panel haplotypes, sequencing coverage, and available hardware. **The following are historical measurements from the original PanGenie**, not performance measurements of MaskedPanGenie-v2:

- On the dataset described in the [PanGenie paper](https://doi.org/10.1038/s41588-022-01043-w), PanGenie took **1 hour 25 minutes** with **22 cores** and used **68 GB RAM**.
- On a dataset with approximately **16 million variants**, **64 haplotypes**, and **30× coverage**, the original implementation took **1 hour 46 minutes** with **24 cores** and used **120 GB RAM**.

## Notes

The original PanGenie documentation also reports processing a panel of **44 samples (88 haplotypes)** and **20,661,169 variants** in **3 hours 15 minutes** with **24 cores**, using **153 GB RAM**. These figures are provided as historical context only.

## Limitations

The underlying PanGenie model becomes more computationally demanding as the number of haplotype paths increases. The original implementation documents a technical limit of **254 input haplotypes (127 diploid samples)**; consult the relevant implementation and upstream documentation before assuming this limit applies unchanged to other versions.

## Demo

A small example is provided in [`demo/`](demo/), including a pangenome VCF (`test-variants.vcf`), short reads (`test-reads.fa`), and a reference sequence (`test-reference.fa`). After building the project, run the following command from the repository root:

```bash
./build/src/PanGenie \
    -i demo/test-reads.fa \
    -r demo/test-reference.fa \
    -v demo/test-variants.vcf \
    -o test \
    -e 100000
```

The genotyping results are written to `test_genotyping.vcf`. The `-e` parameter sets a small Jellyfish hash size suitable for the demo; larger datasets may use the default or a larger value. To use spaced seeds with this version, also provide `-m <spacedSeed.txt>`.

## Acknowledgements

MaskedPanGenie-v2 builds upon the following projects:

- [PanGenie](https://github.com/eblerjana/pangenie), developed by Jana Ebler and collaborators.
- [MaskedPanGenie](https://github.com/hhaentze/MaskedPangenie), developed by Hartmut Häntze and collaborators.
- [PalindromeSpEED](https://github.com/garyasd/PalindromeSpEED), a related tool for designing sensitive palindromic spaced seeds.

This implementation also builds on earlier spaced k-mer counting optimizations developed within our research group. We acknowledge the authors and contributors of the upstream projects and retain their applicable copyright and license notices. The codebase also incorporates work from the group's earlier [Fast_MaskedPanGenie implementation](https://github.com/garyasd/Fast_MaskedPanGenie).

## Citation

If you use this software, please cite the underlying PanGenie and MaskedPanGenie publications:

1. Ebler, J., Ebert, P., Clarke, W. E., et al. **Pangenome-based genome inference allows efficient and accurate genotyping across a wide spectrum of variant classes.** *Nature Genetics* **54**, 518–525 (2022). https://doi.org/10.1038/s41588-022-01043-w
2. Häntze, H., & Horton, P. **Effects of spaced k-mers on alignment-free genotyping.** *Bioinformatics* **39**(Supplement_1), i213–i221 (2023). https://doi.org/10.1093/bioinformatics/btad202

## License

This repository retains the original PanGenie MIT license; see [LICENSE.md](LICENSE.md). Third-party source files retain their applicable copyright and license notices.