// bittrie.cpp
#include <functional>
#include <cmath> // will remove pow usage below; see integer helpers further down
#include "types.hpp"
#include "duoram.hpp"
#include "cell.hpp"
#include "rdpf.hpp"
#include "shapes.hpp"
#include "bittrie.hpp"
#include <string>
#include <cstdlib>

// Forward declaration so BitTrie(...) can call it
static void basic_bit(MPCIO &mpcio,
                      yield_t &yield,
                      int alphasize,
                      int triedepth,
                      size_t n_inserts,
                      size_t n_searches,
                      int is_optimized,
                      unsigned player,
                      MPCTIO &tio);




void BitTrie(unsigned p, MPCIO &mpcio, const PRACOptions &opts, char **args) {
    MPCTIO tio(mpcio, 0, opts.num_cpu_threads);

    int nargs = 0;
    while (args[nargs] != nullptr) ++nargs;

    unsigned player = p;
    int alphasize = 0;
    int triedepth = 0;
    size_t n_inserts = 0;
    size_t n_searches = 0;
    int is_optimized = 0;
    int run_sanity = 0;

    for (int i = 0; i < nargs; i += 2) {
        std::string option = args[i];
        if (option == "-m"   && i + 1 < nargs) alphasize   = std::atoi(args[i + 1]);
        else if (option == "-d"   && i + 1 < nargs) triedepth   = std::atoi(args[i + 1]);
        else if (option == "-i"   && i + 1 < nargs) n_inserts   = std::atoi(args[i + 1]);
        else if (option == "-e"   && i + 1 < nargs) n_searches  = std::atoi(args[i + 1]);
        else if (option == "-opt" && i + 1 < nargs) is_optimized= std::atoi(args[i + 1]);
        else if (option == "-s"   && i + 1 < nargs) run_sanity  = std::atoi(args[i + 1]);
    }

    run_coroutines(tio, [&tio, alphasize, triedepth, n_inserts, n_searches,
                         is_optimized, run_sanity, player, &mpcio](yield_t &yield) {
        basic_bit(mpcio, yield, alphasize, triedepth, n_inserts,
                  n_searches, is_optimized, player, tio);
    });
}


// ---- Init ----
void BitTrieClass::init(MPCTIO &tio, yield_t &yield) {
    auto BitArray = bit_oram.flat(tio, yield);
    BitArray.init([](size_t){ return RegXS{}; }); // all zeros
    num_items = 0;
}

void BitTrieClass::init(MPCTIO &tio, yield_t &yield, size_t n) {
    num_items = n;
    auto BitArray    = bit_oram.flat(tio, yield);
    auto StringArray = second_oram.flat(tio, yield);
    BitArray.init([](size_t){ return RegXS{}; });    // 0
    StringArray.init([](size_t){ return RegXS{}; }); // 0
}

// ---- Per-bit set/get (1 slot == 1 bit) ----
void BitTrieClass::set_bit(MPCTIO &tio, yield_t &yield, RegXS bit_position, unsigned player) {
    auto BitArray = bit_oram.flat(tio, yield);

    // public-1 encoded as XOR share: P0 holds 1, others hold 0
    RegXS one; one.xshare = (player == 0) ? 1 : 0;

    // Just write 1 at secret index (ORAM takes care of obliviousness)
    BitArray[bit_position] = one;
}

RegXS BitTrieClass::get_bit(MPCTIO &tio, yield_t &yield, RegXS bit_position) {
    auto BitArray = bit_oram.flat(tio, yield);
    return BitArray[bit_position]; // returns a RegXS share (0/1)
}

// ---- Trie insert/search ----
void BitTrieClass::insert(MPCTIO &tio, yield_t &yield, RegXS index, RegXS & /*insert_value*/, unsigned player) {
    num_items++;
    // Mark path bit
    set_bit(tio, yield, index, player);
}

void BitTrieClass::search(MPCTIO &tio, yield_t &yield, RegXS index, RegBS &Z, unsigned /*player*/) {
    // Z := Z AND (bit_at_index == 1)
    RegXS bitv = get_bit(tio, yield, index);    // 0/1 in XOR share
    RegBS bval = bitv.bitat(0);                 // safe: value is 0/1, bit 0 equals the value
    RegBS tmp;
    mpc_and(tio, yield, tmp, Z, bval);
    Z = tmp;
}

// ---- Debug print helpers (only P0 sees data) ----
void BitTrieClass::print_bittrie(MPCTIO &tio, yield_t &yield, size_t size) {
    auto BitArray = bit_oram.flat(tio, yield);
    auto R = BitArray.reconstruct();
    if (R.empty()) return; // P1/P2 get empty
    for (size_t i = 0; i < size; ++i) {
        uint64_t v = R[i].share() & 1ULL;
        std::cout << i << "->" << v << "   ";
    }
}

void BitTrieClass::print_bittrie_stringcheck(MPCTIO &tio, yield_t &yield, size_t size) {
    auto StringArray = second_oram.flat(tio, yield);
    auto R = StringArray.reconstruct();
    if (R.empty()) return;
    for (size_t i = 0; i < size; ++i) {
        std::cout << i << "->" << (R[i].share() & 1ULL) << "   ";
    }
}

// ===== helpers: integer math (no std::pow on size_t) =====
static size_t ipow(size_t base, size_t exp) {
    size_t res = 1;
    while (exp) {
        if (exp & 1) res = res * base;
        exp >>= 1;
        if (exp) base = base * base;
    }
    return res;
}
static size_t geom_sum_levels(size_t n, size_t m) {
    // 1 + n + n^2 + ... + n^m
    if (m == 0) return 1;
    size_t sum = 1, term = 1;
    for (size_t k = 1; k <= m; ++k) {
        term *= n;
        sum  += term;
    }
    return sum;
}

// ===== your driver (keep your arrays & flags) =====
#define BITTRIE_VERBOSE
static int preIndex_bit[10];

static int letterToIndex_bit(char x, int pos, int alphasize, int is_optimized) {
    int ch = (is_optimized == 1) ? (x - 'a') : (x - 'a' + 1);
    if (pos == 0) {
        preIndex_bit[pos] = ch;
        return ch;
    } else {
        preIndex_bit[pos] = preIndex_bit[pos-1] * alphasize + ch;
        return preIndex_bit[pos];
    }
}

static size_t sumOfPowers_bit(size_t n, size_t m) {
    // You were using doubles; switch to exact integer version
    // If your trie uses levels 0..m, the total nodes is 1 + n + ... + n^m
    return geom_sum_levels(n, m);
}

static size_t Power_bit(size_t x, size_t y) {
    // You used pow(x, y+1); keep intent but exact
    return ipow(x, y+1);
}

void basic_bit(MPCIO &mpcio, yield_t &yield, int alphasize, int triedepth,
               size_t n_inserts, size_t n_searches, int is_optimized,
               unsigned player, MPCTIO &tio)
{
    // total slots = total trie nodes (one bit per node)
    size_t size = sumOfPowers_bit(alphasize, triedepth) + 1;
    std::cout << "BitTrie size: " << size << " bits\n";

    BitTrieClass tree(tio.player(), size);
    tree.init(tio, yield, size);
#ifdef BITTRIE_VERBOSE
    tree.print_bittrie(tio, yield, size);
    std::cout << "\n===== BitTrie Init Stats =====\n";
#endif
    tio.sync_lamport();
    mpcio.dump_stats(std::cout);
    mpcio.reset_stats();
    tio.reset_lamport();

    std::string insertArray[] = {"dad","aab","aca","daa","dca"};
    std::string searchArray[] = {"ddd","aab","aca","daa","dca"};

    // ---------- INSERT ----------
    for (size_t i = 0; i < n_inserts; i++) {
        RegXS share;  // secret index share
        for (size_t j = 0; j < insertArray[i].length(); j++) {
            // build public path index (deterministic) -> secret-share it
            int inserted_index = letterToIndex_bit(insertArray[i][j], (int)j, alphasize, is_optimized);

            RegXS idx_pub;           // public constant (represented as XOR share) //unused now
            idx_pub.xshare = inserted_index;//unused now

            // create *secret* index by masking with a fixed pad known to both parties
            // (still fine for tests; for real security, use fresh randomness)
            // NEW: public index encoded as XOR share
            share.xshare = (player == 0) ? inserted_index : 0;


            // Set the bit at this node
            RegXS dummy; dummy.xshare = 1; // unused now (kept to match signature)
            tree.insert(tio, yield, share, dummy, player);

            if (is_optimized == 1) {
                tree.print_bittrie(tio, yield, size);
                std::cout << "\n";
            }
        }

        // mark end-of-string in second_oram at the *last* node index
        auto End_String = tree.second_oram.flat(tio, yield);
        RegXS one; one.xshare = (player == 0) ? 1 : 0;
        End_String[share] = one;

        std::cout << "\ninserted value is " << insertArray[i] << std::endl;
#ifdef BITTRIE_VERBOSE
        tree.print_bittrie(tio, yield, size);
        std::cout << "\nString presence array \n";
        tree.print_bittrie_stringcheck(tio, yield, size);
        std::cout << "\n";
#endif
    }

    std::cout << "\n===== BitTrie Insert Stats =====\n";
    tio.sync_lamport();
    mpcio.dump_stats(std::cout);
    mpcio.reset_stats();
    tio.reset_lamport();

#ifdef BITTRIE_VERBOSE
    tree.print_bittrie(tio, yield, size);
    std::cout << "\n";
#endif

    // ---------- SEARCH ----------
    for (size_t i = 0; i < n_searches; i++) {
        // Z := public-true encoded as XOR boolean share (P0=1, others=0)
        RegBS Z; Z.bshare = (player == 0);

        RegXS share;
        for (size_t j = 0; j < searchArray[i].length(); j++) {
            int inserted_index = letterToIndex_bit(searchArray[i][j], (int)j, alphasize, is_optimized);

            RegXS idx_pub; idx_pub.xshare = inserted_index;

            // rebuild secret index with same pad convention (matches insert)
            // NEW: same public index sharing as insert
            share.xshare = (player == 0) ? inserted_index : 0;


            // Accumulate path constraint: Z &= bit[share]
            tree.search(tio, yield, share, Z, player);
        }

        // Also require end-of-string marker at final node
        auto End_String = tree.second_oram.flat(tio, yield);
        RegXS check = End_String[share];   // 0/1 RegXS
        RegBS end = check.bitat(0);        // convert to RegBS
        RegBS value;
        mpc_and(tio, yield, value, Z, end);
        Z = value;

        bool z_final = mpc_reconstruct(tio, yield, Z);  // <- reconstruct the boolean share
        if (player == 0) {
            if (z_final)
                std::cout << "\nThe value " << searchArray[i] << " is present" << std::endl;
            else
            std::cout << "\nThe value " << searchArray[i] << " is not present" << std::endl;
}

    }

    std::cout << "\n===== BitTrie Search Stats =====\n";
    tio.sync_lamport();
    mpcio.dump_stats(std::cout);
}
