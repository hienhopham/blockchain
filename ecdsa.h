#ifndef ECDSA_H
#define ECDSA_H

#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>
#include <string>
#include <cstring>
#include <sstream>
#include <random>
#include <ctime>

class PublicKey {
public:
    long p, a, b, n;
    std::pair<long, long> G, Q;
    PublicKey();
    PublicKey(long p, long a, long b, std::pair<long, long> G, long n, std::pair<long, long> Q);
    void printKey();
};

class ECDSA {
public:
    
    static std::vector<bool> toBinary(long n);
    static long mod(long a, long p);
    static long modPow(long base, long exp, long modulo);
    static bool isEllipticCurve(long p, long a, long b);
    static long modularInverse(long a, long n);
    static std::vector<std::pair<long, long>> calculateEp(long p, long a, long b);
    static std::pair<long, long> addingPoints(std::pair<long, long> P, std::pair<long, long> Q, long p, long a);
    static std::pair<long, long> doublingPoint(std::pair<long, long> P, long p, long a);
    static std::pair<long, long> doubleAndAdd(long n, std::pair<long, long> P, long p, long a);
    static long randRange(long left, long right);
    static bool MillerRabinTest(long n);
    static bool isPrime(long n);
    static long generatePrime(long digits);
    static long findPointOrder(long p, long a, long b, std::pair<long, long> G);
    static std::pair<std::pair<long, long>, long> findPrimeOrderPoint(std::vector<std::pair<long, long>> Ep, long p, long a, long b);
    static std::pair<PublicKey, long> generateKey();
    static std::string sha256(std::string input);
    static long digitizeMessage(std::string message, long p);
    static std::pair<long, long> generateSignature(long p, long a, long b, std::pair<long, long> G, long n, long privateKey, long hashMsg);
    static bool verifySignature(PublicKey publicKey, long privateKey, std::pair<long, long> signature, long n, long hashMsg);

};

#endif

