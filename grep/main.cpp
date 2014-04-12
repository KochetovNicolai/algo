#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <ctime>

#include "Regexp.hpp"

void createFiles(std::string name) {
    std::ofstream in((std::string() + "tests/test" + name + ".in").c_str());
    std::ofstream out((std::string() + "tests/test" + name + ".out").c_str());
}

void genTest(std::string name) {
    std::ifstream in((std::string() + "tests/test" + name + ".in").c_str());
    std::ofstream out((std::string() + "tests/test" + name + ".out").c_str());
    std::string exp;
    std::getline(in, exp);
    Regexp regexp(exp);
    std::string line;
    while (getline(in, line)) {
        std::vector< std::vector< size_t > > positions = regexp.grep(line);
        for (size_t i = 0; i < positions.size(); i++) {
            out << positions[i].size() << '\t';
            for (size_t j = 0; j < positions[i].size(); j++) {
                out << positions[i][j] << ' ';
            }
            out << std::endl;
        }
        out << std::endl;
    }
}

void printPosition(std::vector< std::vector< size_t > > &positions) {
    for (size_t i = 0; i < positions.size(); i++) {
        std::cout << positions[i].size() << '\t';
        for (size_t j = 0; j < positions[i].size(); j++) {
            std::cout << positions[i][j] << ' ';
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}


bool test(std::string name) {
    std::cout <<  "test " + name;
    std::ifstream in((std::string() + "tests/test" + name + ".in").c_str());
    std::ifstream out((std::string() + "tests/test" + name + ".out").c_str());
    clock_t begin = clock();
    std::string exp;
    std::getline(in, exp);
    Regexp regexp(exp);
    std::string line;
    while (getline(in, line)) {
        std::vector< std::vector< size_t > > positions = regexp.grep(line);
        std::vector< std::vector< size_t > > pattern(positions.size());
        for (size_t i = 0; i < positions.size(); i++) {
            size_t size;
            out >> size;
            for (size_t j = 0; j < size; j++) {
                size_t next;
                out >> next;
                pattern[i].push_back(next);
            }
        }
        if (positions != pattern) {
            std::cout << " failed\n";
            std::cout << "expected:\n";
            printPosition(pattern);
            std::cout << "but found:\n";
            printPosition(positions);
            return false;
        }
    }
    clock_t end = clock();
    std::cout << " OK " << (end - begin + .0) / CLOCKS_PER_SEC << " seconds " << std::endl;
    return true;
}

bool checkTests() {
    char testsNumber = 'U';
    //test("E");
    for (char i = 0; i < testsNumber - 'A' + 1; i++)
        if (!test(std::string() + char('A' + i)))
            return false;
    return true;
}

int main(int argc, char **argv) {
    if (argc == 1) {
        std::cout << "testing" << std::endl;
        return !checkTests();
        //genTest("U");
    } else if (argc > 3) {
        std::cerr << "too many arguments" << std::endl;
        std::cerr << "usage: grep regexp [file]" << std::endl;
        return 1;
    } else {
        if (argc == 3)
            freopen(argv[2], "rt", stdin);
        Regexp regexp(argv[1]);
        std::string line;
        size_t lineNumber = 0;
        while (std::getline(std::cin, line)) {
            std::vector< std::vector< size_t > > positions = regexp.grep(line);
            for (size_t i = 0; i < positions.size(); i++)
                for (size_t j = 0; j < positions[i].size(); j++)
                    std::cout << lineNumber << ' ' << i << ' ' << line.substr(i, positions[i][j] - i) << std::endl;
            lineNumber++;
        }
        return 0;
    }
}