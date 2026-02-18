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


## Appendix: Raw Data Tables

### A1. Initialization Phase — All Parties, All Depths

| Impl | Depth | ORAM Size | Party | Messages | Bytes | Lamport | AES Ops | Wall Clock (ms) | Memory (KiB) | Status |
|------|-------|-----------|-------|----------|-------|---------|---------|-----------------|--------------|--------|
| BitTrie | 23 | 262,144 words | P0 | 0 | 0 | 0 | 0 | 15 | 20,556 | S |
| BitTrie | 23 | 262,144 words | P1 | 0 | 0 | 0 | 0 | 23 | 20,820 | S |
| BitTrie | 23 | 262,144 words | P2 | 0 | 0 | 0 | 0 | 13 | 16,476 | S |
| BitTrie | 29 | 16,777,216 words | P0 | 0 | 0 | 0 | 0 | 934 | 794,228 | S |
| BitTrie | 29 | 16,777,216 words | P1 | 0 | 0 | 0 | 0 | 930 | 794,372 | S |
| BitTrie | 29 | 16,777,216 words | P2 | 0 | 0 | 0 | 0 | 519 | 532,292 | S |
| BitTrie | 30 | 33,554,432 words | P0 | 0 | 0 | 0 | 0 | 1,464 | 1,580,872 | F |
| BitTrie | 30 | 33,554,432 words | P1 | 0 | 0 | 0 | 0 | 1,472 | 1,581,136 | F |
| BitTrie | 30 | 33,554,432 words | P2 | 0 | 0 | 0 | 0 | 853 | 1,056,752 | F |
| Trie | 23 | 16,777,215 nodes | P0 | 0 | 0 | 0 | 0 | 1,490 | 794,600 | S |
| Trie | 23 | 16,777,215 nodes | P1 | 0 | 0 | 0 | 0 | 1,487 | 794,868 | S |
| Trie | 23 | 16,777,215 nodes | P2 | 0 | 0 | 0 | 0 | 388 | 532,536 | S |
| Trie | 24 | 33,554,431 nodes | P0 | 0 | 0 | 0 | 0 | 2,795 | 1,581,020 | F |
| Trie | 24 | 33,554,431 nodes | P1 | 0 | 0 | 0 | 0 | 2,803 | 1,580,956 | F |
| Trie | 24 | 33,554,431 nodes | P2 | 0 | 0 | 0 | 0 | 1,681 | 1,056,884 | F |

### A2. Insert Phase — All Parties (Successful Runs Only)

| Impl | Depth | Party | Messages | Bytes | Lamport | AES Ops | Wall Clock (ms) | Memory (KiB) |
|------|-------|-------|----------|-------|---------|---------|-----------------|--------------|
| BitTrie | 23 | P0 | 1,700 | 91,280 | 1,660 | 62,914,320 | 3,886 | 48,932 |
| BitTrie | 23 | P1 | 1,700 | 91,280 | 1,660 | 62,914,320 | 3,882 | 48,860 |
| BitTrie | 23 | P2 | 200 | 165,920 | 1,660 | 62,915,520 | 3,894 | 23,668 |
| BitTrie | 29 | P0 | 2,180 | 120,320 | 2,140 | 4,026,531,600 | 36,049 | 2,068,264 |
| BitTrie | 29 | P1 | 2,180 | 120,320 | 2,140 | 4,026,531,600 | 36,057 | 2,067,892 |
| BitTrie | 29 | P2 | 200 | 221,120 | 2,140 | 4,026,532,800 | 36,583 | 539,844 |
| Trie | 23 | P0 | 1,860 | 105,080 | 1,825 | 3,523,215,150 | 33,253 | 2,068,072 |
| Trie | 23 | P1 | 1,860 | 105,080 | 1,825 | 3,523,215,150 | 33,110 | 2,068,360 |
| Trie | 23 | P2 | 140 | 192,640 | 1,825 | 3,690,988,290 | 34,186 | 540,204 |

### A3. Search Phase — All Parties (Successful Runs Only)

| Impl | Depth | Party | Messages | Bytes | Lamport | AES Ops | Wall Clock (ms) | Memory (KiB) |
|------|-------|-------|----------|-------|---------|---------|-----------------|--------------|
| BitTrie | 23 | P0 | 860 | 45,360 | 841 | 31,457,160 | 2,273 | 48,932 |
| BitTrie | 23 | P1 | 860 | 45,360 | 841 | 31,457,160 | 2,273 | 48,860 |
| BitTrie | 23 | P2 | 122 | 83,488 | 837 | 20,971,840 | 2,265 | 23,668 |
| BitTrie | 29 | P0 | 1,100 | 59,880 | 1,081 | 2,013,265,800 | 13,958 | 2,068,264 |
| BitTrie | 29 | P1 | 1,100 | 59,880 | 1,081 | 2,013,265,800 | 13,950 | 2,067,892 |
| BitTrie | 29 | P2 | 122 | 111,088 | 1,077 | 1,342,177,600 | 13,840 | 539,844 |
| Trie | 23 | P0 | 1,060 | 59,540 | 1,041 | 2,013,265,800 | 16,005 | 2,068,092 |
| Trie | 23 | P1 | 1,060 | 59,540 | 1,041 | 2,013,265,800 | 16,141 | 2,068,364 |
| Trie | 23 | P2 | 82 | 110,128 | 1,039 | 1,342,177,560 | 16,075 | 540,328 |
