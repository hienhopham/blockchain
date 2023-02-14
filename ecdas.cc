#include "sha256.h"
#include "ecdsa.h"

PublicKey::PublicKey(long _p, long _a, long _b, std::pair<long, long> _G, long _n, std::pair<long, long> _Q) {
    p = _p;
    a = _a;
    b = _b;
    G = _G;
    n = _n;
    Q = _Q;
}

PublicKey::PublicKey() {
    std::pair<long, long> _0 = {0, 0};
    PublicKey(0, 0, 0, _0, 0, _0);
}

void
PublicKey::printKey() {
    std::cout << "public key = " << p << ", " << a << ", " << b << ", (" << G.first << ", " << G.second << "), " << n << ", (" << Q.first << ", " << Q.second << ")" << std::endl;
}


std::vector<bool> 
ECDSA::toBinary(long n) {
    std::vector<bool> bits;
    while (n > 0) {
        bits.push_back(n & 1);
        n >>= 1;
    }

    return bits;
}

long 
ECDSA::mod(long a, long p) {
    long result = a % p;

    return (result < 0) ? result + p : result;
}

long 
ECDSA::modPow(long base, long exp, long modulo) {
    if (modulo == 1) {
        return 0;
    }

    double fbase = (double)base;
    double fexp = (double)exp;
    double fmodulo = (double)modulo;

    double result = 1.0;
    while (fexp > 0) {
        if (fmod(fexp, 2) == 1) {
            result = fmod(result * fbase, fmodulo);
        }
        fexp = floor(fexp / 2);
        fbase = fmod(fbase * fbase, fmodulo);
    }

    return (long)result;
}

bool 
ECDSA::isEllipticCurve(long p, long a, long b) {
    if (mod((long)(4 * pow(a, 3) + 27 * pow(b, 2)), p) == 0) {
        return false;
    }

    return true;
}

long 
ECDSA::modularInverse(long a, long n) {
    float s = 0;
    float t = 1;
    float r = n;
    float old_s = 1;
    float old_t = 0;
    float old_r = a;

    float quotient;
    float temp;

    while (r != 0) {
        quotient = floor(old_r / r);

        temp = old_r;
        old_r = r;
        r = temp - quotient * r;

        temp = old_s;
        old_s = s;
        s = temp - quotient * s;

        temp = old_t;
        old_t = t;
        t = temp - quotient * t;
    }

    if (old_s < 0) {
        old_s = n + old_s;
    }

    return (long)old_s;
}

std::vector<std::pair<long, long>> 
ECDSA::calculateEp(long p, long a, long b) {
    std::vector<long> Qp;
    for (long x = 1; x <= (long)(p - 1)/2; ++x) {
        Qp.push_back(mod(x * x, p));
    }

    auto f = [](long x, long p, long a, long b) {
        return mod((long)(pow(x, 3) + a * x + b), p);
    };

    std::vector<std::pair<long, long>> Ep;
    for (long x = 0; x < p; ++x) {
        long fx = f(x, p, a, b);
        auto it = std::find(Qp.begin(), Qp.end(), fx);

        if (it != Qp.end()) {
            int index = it - Qp.begin() + 1;
            int y1 = index;
            int y2 = p - index;
            Ep.emplace_back(x, y1);
            Ep.emplace_back(x, y2);
        }
        else if (fx == 0) {
            Ep.emplace_back(x, 0);
        }
    }

    return Ep;
}

std::pair<long, long> 
ECDSA::addingPoints(std::pair<long, long> P, std::pair<long, long> Q, long p, long a) {
    static std::pair<long, long> Point_0 = {0, 0};

    if (P == Point_0 && Q == Point_0) {
        return Point_0;
    }
    if (P == Point_0 || Q == Point_0) {
        if (P == Point_0) {
            return Q;
        }
        if (Q == Point_0) {
            return P;
        }
    }

    long xP = P.first;
    long yP = P.second;
    long xQ = Q.first;
    long yQ = Q.second;

    if (xP == xQ && yQ == (p - yP)) {
        return Point_0;
    }

    long numerator = yP - yQ;
    long denominator = xP - xQ;
    long m = mod(numerator * modularInverse(denominator, p), p);
    long xR = mod(m * m - xP - xQ, p);
    long yR = mod(m * (xP - xR) - yP, p);

    std::pair<long, long> R = {xR, yR};

    return R;
}

std::pair<long, long> 
ECDSA::doublingPoint(std::pair<long, long> P, long p, long a) {
    static std::pair<long, long> Point_0 = {0, 0};

    if (P == Point_0) {
        return Point_0;
    }

    long xP = P.first;
    long yP = P.second;

    long m = mod((3 * xP * xP + a) * modularInverse(2 * yP, p), p);
    long xR = mod(m * m - 2 * xP, p);
    long yR = mod(m * (xP - xR) - yP, p);

    std::pair<long, long> R = {xR, yR};

    return R;
}

std::pair<long, long> 
ECDSA::doubleAndAdd(long n, std::pair<long, long> P, long p, long a) {
    static std::pair<long, long> Point_0 = {0, 0};

    n = mod(n, p);

    if (n == 1) {
        return P;
    }
    else if (n == 0 || P == Point_0) {
        return Point_0;
    }

    std::pair<long, long> result = Point_0;
    std::pair<long, long> addend = P;
    std::vector<bool> bits = toBinary(n);

    for (int i = 0; i < bits.size(); ++i) {
        if (bits[i]) {
            result = addingPoints(result, addend, p, a);
        }
        addend = doublingPoint(addend, p, a);
    }

    return result;
}

long 
ECDSA::randRange(long left, long right) {
    static std::mt19937 gen(std::time(nullptr));
    std::uniform_int_distribution<> distr(left, right);

    return distr(gen);
}

bool 
ECDSA::MillerRabinTest(long n) {
    const int N_TRIALS = 5;

    long d = n - 1;
    long r = 0;

    while (mod(d, 2) == 0) {
        d /= 2;
        r += 1;
    }

    long a;
    long x;
    long i;

    for (int i = 0; i < N_TRIALS; ++i) {
        a = randRange(2, n - 2);
        x = modPow(a, d, n);

        if (x != 1) {
            i = 0;

            while (x != (n - 1)) {
                if (i == (r - 1)) {
                    return false;
                }
                else {
                    i += 1;
                    x = mod(x * x, n);
                }
            }
        }
    }

    return true;
}

bool 
ECDSA::isPrime(long n) {
    if (n < 2) {
        return false;
    }

    static std::vector<int> lowPrimes = {
        2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149,
        151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 311,
        313, 317, 331, 337, 347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409, 419, 421, 431, 433, 439, 443, 449, 457, 461, 463, 467, 479, 487,
        491, 499, 503, 509, 521, 523, 541, 547, 557, 563, 569, 571, 577, 587, 593, 599, 601, 607, 613, 617, 619, 631, 641, 643, 647, 653, 659, 661, 673,
        677, 683, 691, 701, 709, 719, 727, 733, 739, 743, 751, 757, 761, 769, 773, 787, 797, 809, 811, 821, 823, 827, 829, 839, 853, 857, 859, 863, 877,
        881, 883, 887, 907, 911, 919, 929, 937, 941, 947, 953, 967, 971, 977, 983, 991, 997};

    auto it = std::find(lowPrimes.begin(), lowPrimes.end(), n);

    if (it != lowPrimes.end()) {
        return true;
    }

    for (int i = 0; i < lowPrimes.size(); ++i) {
        if (n % lowPrimes[i] == 0) {
            return false;
        }
    }

    return MillerRabinTest(n);
}

long 
ECDSA::generatePrime(long digits) {
    long result = randRange((long)pow(10, digits - 1), (long)pow(10, digits) - 1);

    if (mod(result, 2) == 0) {
        result += 1;
    }

    while (!isPrime(result)) {
        result += 2;
    }

    return result;
}

long 
ECDSA::findPointOrder(long p, long a, long b, std::pair<long, long> G) {
    static std::pair<long, long> Point_0 = {0, 0};
    std::pair<long, long> kG = doublingPoint(G, p, a);
    long k = 2;

    while (kG != Point_0) {
        k += 1;
        kG = addingPoints(kG, G, p , a);
    }

    return k;
}

std::pair<std::pair<long, long>, long> 
ECDSA::findPrimeOrderPoint(std::vector<std::pair<long, long>> Ep, long p, long a, long b) {
    static std::pair<long, long> Point_0 = {0, 0};

    long Ep_size = Ep.size();

    if (isPrime(Ep_size + 1)) {
        std::pair<long, long> randPoint = Ep[randRange(0, Ep_size - 1)];
        std::pair<std::pair<long, long>, long> result = {randPoint, Ep_size + 1};

        return result;
    }

    long n;

    for (long i = 0; i < Ep_size; ++i) {
        if (Ep[i].second == 0) {
            continue;
        }

        n = findPointOrder(p, a, b, Ep[i]);

        if (isPrime(n)) {
            std::pair<std::pair<long, long>, long> result = {Ep[i], n};

            return result;
        }
    }

    std::pair<std::pair<long, long>, long> result = {Point_0, 0};

    return result;
}

std::pair<PublicKey, long> 
ECDSA::generateKey() {
    long p, a, b;
    p = a = b = 1;
    while (!isEllipticCurve(p, a, b)) {
        p = generatePrime(4);
        a = generatePrime(3);
        b = generatePrime(3);
    }

    std::pair<std::pair<long, long>, long> primeOrderPoint = findPrimeOrderPoint(calculateEp(p, a, b), p, a, b);
    std::pair<long, long> G = primeOrderPoint.first;
    long n = primeOrderPoint.second;

    long privateKey = randRange(1, n - 1);

    std::pair<long, long> Q = doubleAndAdd(privateKey, G, p, a);

    PublicKey publicKey(p, a, b, G, n, Q);

    std::pair<PublicKey, long> keyPair = {publicKey, privateKey};

    return keyPair;
}

std::string 
ECDSA::sha256(std::string input) {
    SHA256 ctx = SHA256();

    unsigned char digest[ctx.DIGEST_SIZE];
    memset(digest, 0, ctx.DIGEST_SIZE);

    ctx.init();
    ctx.update( (unsigned char*)input.c_str(), input.length());
    ctx.final(digest);

    char buf[2*SHA256::DIGEST_SIZE+1];
    buf[2*SHA256::DIGEST_SIZE] = 0;

    for (int i = 0; i < SHA256::DIGEST_SIZE; i++)
        sprintf(buf+i*2, "%02x", digest[i]);
    return std::string(buf);

}

long 
ECDSA::digitizeMessage(std::string message, long p) {
    message = sha256(message).substr(0, 8);
    std::stringstream ss;
    long result;
    ss << std::hex << message;
    ss >> result;

    return mod(result, p);
}

std::pair<long, long> 
ECDSA::generateSignature(long p, long a, long b, std::pair<long, long> G, long n, long privateKey, long hashMsg) {
    static std::pair<long, long> Point_0 = {0, 0};
    long k = randRange(1, n - 1);
    std::pair<long, long> kG = doubleAndAdd(k, G, p, a);
    long r = mod(kG.first, n);

    if (r == 0) {
        generateSignature(p, a, b, G, n, privateKey, hashMsg);
        return Point_0;
    }

    long s = mod(modularInverse(k, n) * (hashMsg + privateKey * r), n);
    std::pair<long, long> result = {r, s};

    return result;
}

bool 
ECDSA::verifySignature(PublicKey publicKey, long privateKey, std::pair<long, long> signature, long n, long hashMsg) {
    long p = publicKey.p;
    long a = publicKey.a;
    std::pair<long, long> G = publicKey.G;
    std::pair<long, long> Q = publicKey.Q;
    long r = signature.first;
    long s = signature.second;

    if (r < 1 || r >= n) {
        return false;
    }

    if (s < 1 || s >= n) {
        return false;
    }

    long w = modularInverse(s, n);
    long u1 = mod(hashMsg * w, n);
    long u2 = mod(r * w, n);
    std::pair<long, long> A1 = doubleAndAdd(u1, G, p, a);
    std::pair<long, long> A2 = doubleAndAdd(u2, Q, p, a);
    std::pair<long, long> A = addingPoints(A1, A2, p, a);
    long v = mod(A.first, n);

    if (v != r) {
        return false;
    }

    return true;
}