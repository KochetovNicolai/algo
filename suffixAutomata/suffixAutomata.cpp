#include <iostream>
#include <vector>
#include <string>

const size_t UNDEFINED = -1;

struct Vertex {
    std::vector < size_t > edges;
    size_t length;     // ����� ������������� ���� �� �������
    size_t link;       // ���������� ������
    size_t pathNumber; // �������� - ����� ��������� ����� �� �������
    Vertex(size_t length = 0, size_t link = UNDEFINED):
        edges(1 << (8 * sizeof(char)), UNDEFINED),
        length(length),
        link(link),
        pathNumber(UNDEFINED) {
    }
};

std::vector < Vertex > suffAutomata;

size_t init() {
    // ������
    suffAutomata.push_back(Vertex());
    return 0;
}

size_t addNextCharacter(size_t lastVertex, unsigned char character) {
    size_t newVertex = suffAutomata.size();
    suffAutomata.push_back(Vertex(suffAutomata[lastVertex].length + 1));
    // ���� �� ���������� ������� � ����������� ����� � ����� �������, ���� �����
    while (lastVertex != UNDEFINED && suffAutomata[lastVertex].edges[character] == UNDEFINED) {
        suffAutomata[lastVertex].edges[character] = newVertex;
        lastVertex = suffAutomata[lastVertex].link;
    }
    if (lastVertex == UNDEFINED) {
        // ��������� ����� �����
        suffAutomata[newVertex].link = 0;
        return newVertex;
    }
    // ����� lastVertex ��� �������� ����� �� ������� charcter � nextVertex
    size_t nextVertex = suffAutomata[lastVertex].edges[character];
    if (suffAutomata[nextVertex].length == suffAutomata[lastVertex].length + 1) {
        // � nextVertex ���� ������ ����� �������, ����� ����� ��������������� ������ �� ������ 
        suffAutomata[newVertex].link = nextVertex;
        return newVertex;
    }
    // ���� ������, ��������������� � nextVertex ������� �����. ������� ����� ����� ���������������
    size_t clone = suffAutomata.size();
    suffAutomata.push_back(suffAutomata[nextVertex]);
    suffAutomata[clone].length = suffAutomata[lastVertex].length + 1;
    suffAutomata[newVertex].link = clone;
    suffAutomata[nextVertex].link = clone;
    while (lastVertex != UNDEFINED && suffAutomata[lastVertex].edges[character] == nextVertex) {
        suffAutomata[lastVertex].edges[character] = clone;
        lastVertex = suffAutomata[lastVertex].link;
    }
    return newVertex;
}

size_t buildSuffAutomata(const std::string &str) {
    // �������, � ������� ������������� ������
    size_t lastVertex = init();
    for (size_t i = 0; i < str.size(); i++) {
        lastVertex = addNextCharacter(lastVertex, str[i]);
    }
    return lastVertex;
}

size_t pathNumber(size_t vertex) {
    if (vertex == UNDEFINED) {
        return 0;
    } else {
        if (suffAutomata[vertex].pathNumber == UNDEFINED) {
            suffAutomata[vertex].pathNumber = 1;
            for (size_t i = 0; i < suffAutomata[vertex].edges.size(); i++) {
                suffAutomata[vertex].pathNumber += pathNumber(suffAutomata[vertex].edges[i]);
            }
        }
        return suffAutomata[vertex].pathNumber;
    }
}

void printSuffAutomata() {
    for (size_t i = 0; i < suffAutomata.size(); i++) {
        std::cout << i << std::endl << ":";
        for (size_t j = 0; j < suffAutomata[i].edges.size(); j++) {
            if (suffAutomata[i].edges[j] != UNDEFINED) {
                std::cout << (unsigned char) j << suffAutomata[i].edges[j] << ' ';
            }
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void main() {
    std::string str;
    std::cin >> str;
    buildSuffAutomata(str);
    printSuffAutomata();
    //������ ���� ������������
    std::cout << pathNumber(0) - 1 << std::endl;
}