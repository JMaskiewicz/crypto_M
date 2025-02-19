//
// Created by jmask on 18/02/2025.
//
#include <iostream>
#include <vector>
#include <unordered_map>
#include <tuple>
#include <ctime>
#include <openssl/sha.h>
#include <sodium.h>
#include <cmath>
#include <sstream>

// Pixel struct to store blockchain data
struct Pixel {
    int x, y;  // Coordinates
    int r, g, b;  // 4x4x4 RGB color values
    std::string owner;  // Owner of the pixel
    time_t timestamp;  // Mining timestamp
    std::string signature; // Digital signature for security
    int nonce;  // PoW nonce

    Pixel(int x, int y, int r, int g, int b, std::string owner, std::string signature, int nonce)
        : x(x), y(y), r(r), g(g), b(b), owner(owner), signature(signature), nonce(nonce) {
        timestamp = time(nullptr);
    }
};

// Block struct to store blockchain blocks
struct Block {
    int index;
    std::vector<Pixel> pixels;
    time_t timestamp;
    std::string prev_hash;
    std::string hash;

    Block(int index, std::vector<Pixel> pixels, std::string prev_hash)
        : index(index), pixels(pixels), prev_hash(prev_hash) {
        timestamp = time(nullptr);
        hash = computeHash();
    }

    std::string computeHash() {
        std::stringstream ss;
        ss << index << prev_hash << timestamp;
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256((unsigned char*)ss.str().c_str(), ss.str().size(), hash);
        std::string hexHash;
        for (unsigned char i : hash) {
            hexHash += sprintf("%02x", i);
        }
        return hexHash;
    }
};

// Blockchain class to manage mined pixels
class Blockchain {
private:
    std::vector<Block> chain;
    std::unordered_map<std::tuple<int, int>, Pixel, std::hash<int>> pixels;
    int difficulty = 2;  // Mining difficulty (number of leading zeros required in hash)

public:
    Blockchain() {
        // Initialize cryptographic library
        if (sodium_init() < 0) {
            throw std::runtime_error("Failed to initialize cryptographic library");
        }
        // Create the genesis block with the first pixel at (1,1)
        Pixel genesisPixel(1, 1, 0, 0, 0, "Genesis", "", 0);
        Block genesisBlock(0, {genesisPixel}, "0");
        chain.push_back(genesisBlock);
        pixels[{1, 1}] = genesisPixel;
    }

    // Compute Manhattan distance
    int manhattanDistance(int x1, int y1, int x2, int y2) {
        return std::abs(x1 - x2) + std::abs(y1 - y2);
    }

    // Perform Proof of Work
    int proofOfWork(int x, int y, int r, int g, int b, std::string owner) {
        int nonce = 0;
        std::string hash;
        do {
            std::stringstream ss;
            ss << x << y << r << g << b << owner << nonce;
            unsigned char hashBytes[SHA256_DIGEST_LENGTH];
            SHA256((unsigned char*)ss.str().c_str(), ss.str().size(), hashBytes);
            hash.clear();
            for (unsigned char i : hashBytes) {
                hash += sprintf("%02x", i);
            }
            nonce++;
        } while (hash.substr(0, difficulty) != std::string(difficulty, '0'));
        return nonce - 1;
    }

    // Check if a pixel is already mined
    bool isPixelMined(int x, int y) {
        return pixels.find({x, y}) != pixels.end();
    }

    // Mine a new pixel with PoW and Manhattan distance validation
    bool minePixel(int x, int y, int r, int g, int b, std::string owner) {
        if (isPixelMined(x, y)) {
            std::cout << "Mining failed: Pixel at (" << x << ", " << y << ") is already mined." << std::endl;
            return false;
        }

        int nonce = proofOfWork(x, y, r, g, b, owner);
        std::string dataToSign = owner + std::to_string(x) + std::to_string(y) + std::to_string(r) + std::to_string(g) + std::to_string(b) + std::to_string(nonce);
        std::string signature = dataToSign;  // Placeholder for a real digital signature
        Pixel newPixel(x, y, r, g, b, owner, signature, nonce);
        pixels[{x, y}] = newPixel;

        // Create a new block containing the mined pixel
        Block newBlock(chain.size(), {newPixel}, chain.back().hash);
        chain.push_back(newBlock);

        std::cout << "Mined pixel at (" << x << ", " << y << ") with color "
                  << "(" << r << ", " << g << ", " << b << ") and PoW nonce " << nonce << std::endl;
        return true;
    }

    // Display all mined pixels and blockchain data
    void displayBlockchain() {
        for (const auto &block : chain) {
            std::cout << "Block " << block.index << " (Hash: " << block.hash << ")" << std::endl;
            for (const auto &pixel : block.pixels) {
                std::cout << "  Pixel at (" << pixel.x << ", " << pixel.y << ") - Color: "
                          << "(" << pixel.r << ", " << pixel.g << ", " << pixel.b << ")"
                          << " - Owner: " << pixel.owner << " - PoW Nonce: " << pixel.nonce << "\n";
            }
        }
    }
};

int main() {
    Blockchain blockchain;
    blockchain.minePixel(1, 2, 2, 3, 1, "User1");  // Adjacent to (1,1)
    blockchain.minePixel(2, 2, 1, 2, 3, "User2");  // Validated using Manhattan distance
    blockchain.minePixel(2, 2, 3, 1, 2, "User3");  // Should fail (already mined)
    blockchain.displayBlockchain();
    return 0;
}
