#define _USE_MATH_DEFINES

#include <complex>
#include <iostream>
#include <vector>
#include <cstdio>
#include <cmath>

typedef std::complex< double > Base;

size_t reverseBits(size_t value, size_t logSize) {
    size_t result = 0;
    for (size_t i = 0; i < logSize; i++)
        result |= (((1UL << i) & value) >> i) << (logSize - i - 1);
    return result;
}

//меняет значения у пар индексов с обратными битами
void reverseVector(std::vector< Base > &v) {
    size_t logSize = 0;
    while ((1UL << logSize) < v.size())
        logSize++;
    for (size_t i = 0; i < v.size(); i++) {
        size_t j = reverseBits(i, logSize);
        if (i < j)
            std::swap(v[i], v[j]);
    }
}


//предподсчет корней из 1
void calcRoots(std::vector< Base > &roots, bool inverse) {
    double angle = 2 * M_PI / roots.size() * (inverse ? -1 : 1);
    Base mainRoot(cos(angle), sin(angle));
    roots[0] = Base(1, 0);
    for (size_t i = 1; i < roots.size(); i++)
        roots[i] = roots[i - 1] * mainRoot;
}

void fft(std::vector < Base > &v, std::vector < Base > &roots) {
    reverseVector(v);
    for (size_t length = 2; length <= v.size(); length *= 2) {
        for (size_t i = 0; i < v.size() / length; i++) {
            for (size_t j = 0; j < length / 2; j++) {
                Base left = v[i * length + j];
                Base right = v[i * length + length / 2 + j] * roots[v.size() / length * j];
                v[i * length + j] = left + right;
                v[i * length + length / 2 + j] = left - right;
            }
        }
    }
}

void DFT(std::vector< Base > &v) {
    std::vector< Base > roots(v.size());
    calcRoots(roots, false);
    fft(v, roots);
}

void inverseDFT(std::vector< Base > &v) {
    std::vector< Base > roots(v.size());
    calcRoots(roots, true);
    fft(v, roots);
    for (size_t i = 0; i < v.size(); i++)
        v[i] /= v.size();
}

void multiply(std::vector< Base > &a, std::vector< Base > &b, std::vector< Base > &mult) {
    DFT(a);
    DFT(b);
    for (size_t i = 0; i < a.size(); i++)
        mult[i] = a[i] * b[i];
    inverseDFT(mult);
}

size_t solve(const std::string &a, const std::string &b) {
    size_t size = a.size() * 2;
    size_t n = 1;
    //округлим длину вверх до степени 2-и и удвоим
    while (n < 2 * size)
        n *= 2;
    std::vector< Base > first(n), second(n);
    std::vector< Base > firstInv(n), secondInv(n);
    std::vector< Base > mult(n), multInv(n);
    //переведем строки в 0-1 слова
    for (size_t i = 0; i < a.size(); i++) {
        first[i].real(a[i] - 'a');
        first[i + a.size()].real(a[i] - 'a');
        firstInv[i].real(1 - (a[i] - 'a'));
        firstInv[i + a.size()].real(1 - (a[i] - 'a'));
    }
    for (size_t i = 0; i < b.size(); i++) {
        second[b.size() - i - 1].real(b[i] - 'a');
        secondInv[b.size() - i - 1].real(1 - (b[i] - 'a'));
    }
    multiply(first, second, mult);
    multiply(firstInv, secondInv, multInv);
    size_t maxLength = 0;
    for (size_t i = a.size(); i < mult.size(); i++)
        maxLength = std::max(maxLength, (size_t)(mult[i].real() + .5) + (size_t)(multInv[i].real() + .5));
    return maxLength;
}

int main() {
    //freopen("input.in", "r", stdin);
    std::string a, b;
    std::cin >> a >> b;
    std::cout << a.size() - solve(a, b) << std::endl;
}