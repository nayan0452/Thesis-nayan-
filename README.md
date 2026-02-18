# Secure Trie Data Structures — PRAC Framework

Secure MPC implementations of Trie, BitTrie, and IPTrie (Longest Prefix Matching) using the PRAC framework with Duoram ORAM.

## Quick Start

### 1. Build Docker

```bash
cd thesis/docker
./build-docker
```

### 2. Start Docker

```bash
./start-docker
```

### 3. Compile Changes

After modifying source files, recompile inside Docker:

```bash
docker exec -it prac_p0 bash -lc 'cd /root/prac && make -j2'
```

### 4. Run Experiments

**BitTrie** (bit-packed trie for string matching):
```bash
./run-experiment -o bittrie -m 2 -d 23 -i 5 -e 5 -opt 0 -s 1 -debug 0
```

**Trie** (standard byte-level trie):
```bash
./run-experiment -o trie -m 2 -d 23 -i 5 -e 5 -opt 0 -s 1 -debug 0
```



## Command-Line Flags

| Flag | Description | Notes |
|------|-------------|-------|
| `-o` | Online-only mode | All crypto computed on-the-fly |
| `-m` | Alphabet size | e.g. 2 (binary), 4, 8. Not used for IPTrie |
| `-d` | Trie depth | Max 32 for IPTrie (IPv4) |
| `-i` | Number of insertions | |
| `-e` | Number of searches | |
| `-opt` | Optimization flag | 0 = standard, 1 = optimized indexing |
| `-s` | Sanity check flag | 1 = enabled |
| `-debug` | Debug output | 0 = benchmarking (fast), 1 = verbose (slow) |

