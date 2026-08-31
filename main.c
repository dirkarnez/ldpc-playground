#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Real-world Scale Constants (e.g., matching a typical Wi-Fi / 5G block configuration)
#define DATA_BITS 1024      // 1Kb of information bits
#define PARITY_BITS 1024    // 1Kb of protection parity bits (Rate 1/2 code)
#define TOTAL_BITS (DATA_BITS + PARITY_BITS) // 2048-bit Block Size

// Structure representing our massive 2048-bit transmission block
typedef struct {
    int bits[TOTAL_BITS];
} LDPC_Block;

int main() {
    const char *message = "Hello world";
    LDPC_Block block;
    
    // Initialize block with zeros (Padded data)
    memset(&block, 0, sizeof(LDPC_Block));

    // --- STEP 1: Pack "Hello world" into the massive block ---
    int bit_idx = 0;
    for (int i = 0; i < strlen(message); i++) {
        char ch = message[i];
        for (int b = 7; b >= 0; b--) {
            block.bits[bit_idx++] = (ch >> b) & 1;
        }
    }
    // Note: Bits from index 88 to 1023 remain 0 as "padding" 
    // This is exactly how real-world short packets are scaled up to fit modern hardware channels.

    // --- STEP 2: Generate 1,024 Parity Bits via a Sparse Structural Loop ---
    // Instead of random sparse generation, we simulate a Quasi-Cyclic (QC) structure.
    // Each parity check equation is low-density: it looks at a repeating shift window of data bits.
    for (int p = 0; p < PARITY_BITS; p++) {
        int parity_accumulator = 0;
        
        // This structural loop defines the sparse connections (The H-Matrix structure)
        // Each parity bit checks only 4 mathematically staggered data bits (Sparse/Low-Density)
        int check_indices[4];
        check_indices[0] = p % DATA_BITS;
        check_indices[1] = (p + 31) % DATA_BITS;
        check_indices[2] = (p + 127) % DATA_BITS;
        check_indices[3] = (p + 511) % DATA_BITS;

        for (int i = 0; i < 4; i++) {
            parity_accumulator ^= block.bits[check_indices[i]];
        }
        
        // Assign calculated parity bit to the second half of the block
        block.bits[DATA_BITS + p] = parity_accumulator;
    }

    printf("--- REAL-WORLD SCALE LDPC BLOCK ENCODER ---\n");
    printf("Successfully packed \"Hello world\" into a standard large frame.\n");
    printf("Total block size processed: %d bits (%d data payload bits + %d parity check bits).\n\n", 
           TOTAL_BITS, DATA_BITS, PARITY_BITS);

    // --- STEP 3: Channel Noise (Simulate an Air Interference Glitch) ---
    // Let's corrupt bit #11 (which resides inside the letter 'l' in "Hello")
    int corrupted_bit = 11;
    block.bits[corrupted_bit] ^= 1; 
    printf("--- CHANNEL NOISE ---\n");
    printf("Simulated radio static flipped Bit #%d during transmission over the air!\n\n", corrupted_bit);

    // --- STEP 4 & 5: Decoding / Error Hunting ---
    // Track which parity check equations fail
    int failed_checks[PARITY_BITS] = {0};
    int total_failures = 0;

    for (int p = 0; p < PARITY_BITS; p++) {
        int check_accumulator = 0;
        
        int check_indices[4];
        check_indices[0] = p % DATA_BITS;
        check_indices[1] = (p + 31) % DATA_BITS;
        check_indices[2] = (p + 127) % DATA_BITS;
        check_indices[3] = (p + 511) % DATA_BITS;

        for (int i = 0; i < 4; i++) {
            check_accumulator ^= block.bits[check_indices[i]];
        }

        // Compare check node with its original transmitted parity evaluation
        if (check_accumulator != block.bits[DATA_BITS + p]) {
            failed_checks[p] = 1; // Mark this specific equation as broken
            total_failures++;
        }
    }

    printf("--- DECODER (Belief Propagation Tracking) ---\n");
    printf("Scanning matrix equations... Found %d broken parity equations out of %d.\n", 
            total_failures, PARITY_BITS);

    // Hard decision bit flipping resolution:
    // Scan through all 1,024 data bits to see which bit belongs to the most failed check systems
    int culprit_bit = -1;
    int max_votes = 0;

    for (int d = 0; d < DATA_BITS; d++) {
        int vote_count = 0;
        // Check how many of the failed equations this specific data bit participated in
        if (failed_checks[d % PARITY_BITS]) vote_count++;
        if (failed_checks[(d - 31 + DATA_BITS) % PARITY_BITS]) vote_count++;
        if (failed_checks[(d - 127 + DATA_BITS) % PARITY_BITS]) vote_count++;
        if (failed_checks[(d - 511 + DATA_BITS) % PARITY_BITS]) vote_count++;

        if (vote_count > max_votes) {
            max_votes = vote_count;
            culprit_bit = d;
        }
    }

    if (max_votes >= 3) { // High confidence threshold match
        printf("Algorithm Converged: Bit #%d identified as the error source (Failed %d checks).\n", 
                culprit_bit, max_votes);
        block.bits[culprit_bit] ^= 1; // Perform the fix
        printf("Repairing Bit #%d...\n", culprit_bit);
    } else {
        printf("Could not isolate error pattern clearly.\n");
    }

    // --- STEP 6: Read out the text after decoding the massive structure ---
    char output_str[12] = {0};
    for (int i = 0; i < 11; i++) {
        char ch = 0;
        for (int b = 0; b < 8; b++) {
            ch |= (block.bits[i * 8 + b] << (7 - b));
        }
        output_str[i] = ch;
    }

    printf("\n--- SYSTEM RECOVERY COMPLETE ---\n");
    printf("Decoded String out of the 2,048-bit stream: \"%s\"\n", output_str);

    return 0;
}
