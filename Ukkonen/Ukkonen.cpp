// ”кконен, Ќиколай  очетов, 294 группа

#include <vector>
#include <string>
#include <iostream>

const size_t UNDEFINED = -1;
const size_t INFINITY = -2;

struct Edge {
    // [begin..end) - буквы из строки на ребре 
    size_t begin;
    size_t end;
    size_t target; // собственно, куда ведет
    Edge(size_t begin, size_t end = INFINITY, size_t target = UNDEFINED):
        begin(begin),
        end(end),
        target(target) {
    }
};

std::vector < Edge > edgeList;

struct Vertex {
    std::vector < size_t > edges;
    size_t parent;          // откуда пришли
    unsigned char letter;   // и по какой букве
    size_t link;            // суффиксна€ ссылка
    // следущие 3 пол€ не нужны дл€ ”кконена и считаютс€ после построени€ дерева
    size_t maxSubstr;       // днамика - наибоьша€ обща€ побстрока, собираема€ из этой вершины
    bool containsFirst;     // достижим хот€ бы 1 суффикс из 1-й строки
    bool containsSecond;    // достижим хот€ бы 1 суффикс “ќЋ№ ќ из 2-й строки
    Vertex(size_t parent, unsigned char letter):
        edges(1 << (8 * sizeof(char)), UNDEFINED),
        parent(parent),
        letter(letter),
        link(UNDEFINED),
        maxSubstr(UNDEFINED),
        containsFirst(false),
        containsSecond(false) {
    }
};


// позици€ на ребре или в вершине
struct Position {
    size_t vertex;          // откуда пришли 
    size_t shift;           // сколько прошли по ребру
    unsigned char letter;   // с какой буквы начиналось ребро
    // инвариант: 1) shift всегда меньше длины ребра
    //            2) если shift == 0, то letter неопределено и не провер€етс€
    Position(size_t vertex, size_t shift, unsigned char letter = 0):
        vertex(vertex),
        shift(shift),
        letter(letter) {
    }
};

std::vector < Vertex > suffTree;

Position initSuffTree() {
    Vertex root(1, 0);
    Vertex joker(UNDEFINED, 0);
    root.parent = root.link = 1;
    // все ребра по всем буквам ведут из joker в root
    // на самом деле сошлемс€ на 1 ребро, оно все равно не будет мен€тьс€
    edgeList.push_back(Edge(UNDEFINED, 0, 0));
    joker.edges.assign(joker.edges.size(), 0);
    suffTree.push_back(root);
    suffTree.push_back(joker);
    return Position(0, 0);
}

//пилим ребро, на котором сидим. возвращает новую вершину в середине ребра
size_t splitEdge(const Position &position, const std::string &str) {
    Vertex &parent = suffTree[position.vertex];
    size_t currentEdge = parent.edges[position.letter];

    size_t cut = edgeList[currentEdge].begin + position.shift;
    size_t newVertex = suffTree.size();
    suffTree.push_back(Vertex(position.vertex, position.letter));

    suffTree[newVertex].edges[(unsigned char) str[cut]] = edgeList.size();
    edgeList.push_back(edgeList[currentEdge]);
    edgeList.back().begin = cut;
    size_t target = edgeList[currentEdge].target;
    if (target != UNDEFINED) {
        suffTree[target].parent = newVertex;
        suffTree[target].letter = str[cut];
    }

    edgeList[currentEdge].end = cut;
    edgeList[currentEdge].target = newVertex;
    return newVertex;
}

size_t getLink(Vertex &vertex, const std::string &str) {
    if (vertex.link == UNDEFINED) {
        // идем в ролител€, переходим по его ссылке
        Vertex &parent = suffTree[vertex.parent];
        unsigned char letter = vertex.letter;
        size_t begin = edgeList[parent.edges[letter]].begin;
        size_t end = edgeList[parent.edges[letter]].end;
        size_t edgeSize = end - begin;
        size_t parentLink = getLink(parent, str);
        size_t visited = 0;
        size_t nextEdge;
        //спускаемс€ вниз из ссылки родител€ по символам нашего ребра
        while (visited < edgeSize) {
            nextEdge = suffTree[parentLink].edges[(unsigned char) str[visited + begin]];
            visited += edgeList[nextEdge].end - edgeList[nextEdge].begin;
            if (visited > edgeSize) {
                // проскочили, вернемс€ назад
                visited -= edgeList[nextEdge].end - edgeList[nextEdge].begin;
                break;
            }
            parentLink = edgeList[nextEdge].target;
        }
        if (visited == edgeSize) {
            vertex.link = parentLink;
        } else {
            // придетс€ порезать ребро, мы стоим в его центре
            Position position(parentLink, edgeSize - visited, str[edgeList[nextEdge].begin]);
            vertex.link = splitEdge(position, str);
        }
    }
    return vertex.link;
}

//добавл€ем к дереву по букве
void add(Position &position, size_t letter, const std::string &str) {
    size_t vertex = position.vertex;
    if (position.shift) {
        Edge &edge = edgeList[suffTree[vertex].edges[position.letter]];
        if (str[edge.begin + position.shift] == str[letter]) {
            // если стоим посреди ребра, и можем его не резать, то спустимс€ вниз и ничего не делаем
            position.shift++;
            if (edge.begin + position.shift == edge.end) {
                position = Position(edge.target, 0);
            }
            return;
        }
        // не можем спуститьс€ по ребру, режем
        vertex = splitEdge(position, str);
    }
    //пока нет ребра, добавим такое и спустимс€ по суффиксной ссылке
    while (suffTree[vertex].edges[(unsigned char)str[letter]] == UNDEFINED) {
        suffTree[vertex].edges[(unsigned char)str[letter]] = edgeList.size();
        edgeList.push_back(Edge(letter));
        vertex = getLink(suffTree[vertex], str);
    }
    //нашли ребро, спускаемс€ по нему и завершаемс€
    position = Position(vertex, 1, str[letter]);
    Edge &newEdge = edgeList[suffTree[vertex].edges[(unsigned char)str[letter]]];
    // гр€зный хак с joker и [UNDEFINED..0) ребром
    if (newEdge.end - newEdge.begin == 1) {
        position = Position(newEdge.target, 0);
    }
}

void printSuffTree(const std::string &str) {
    for (size_t i = 0; i < suffTree.size(); i++) {
        std::cout << i << std::endl << ":";
        // можно и не печатать ребра из joker
        for (size_t j = 0; i != 1 && j < suffTree[i].edges.size(); j++) {
            if (suffTree[i].edges[j] != UNDEFINED) {
                Edge &edge = edgeList[suffTree[i].edges[j]];
                for (size_t k = edge.begin; k < std::min(edge.end, str.size()); k++)
                    std::cout << str[k];
                if (edge.target != UNDEFINED) {
                    std::cout << edge.target;
                }
                std::cout << ' ';
            }
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void buildSuffTree(const std::string &str) {
    Position position = initSuffTree();
    for (size_t i = 0; i < str.size(); i++) {
        add(position, i, str);
        //printSuffTree(str);
    }
}

// просто вычисл€ет флаги containsFirst и containsSecond
void findWordsParts(size_t size, size_t position = 0) {
    std::vector < size_t > &edges = suffTree[position].edges;
    //Vertex &vertex = suffTree[position];
    for (size_t i = 0; i < edges.size(); i++) {
        if (edges[i] != UNDEFINED) {
            Edge &edge = edgeList[edges[i]];
            if (edge.end != INFINITY) {
                findWordsParts(size, edge.target);
                if (suffTree[edge.target].containsFirst) {
                    suffTree[position].containsFirst = true;
                } 
                if (suffTree[edge.target].containsSecond) {
                    suffTree[position].containsSecond = true;
                }
            } else {
                if (edge.begin <= size) {
                    suffTree[position].containsFirst = true;
                } else {
                    suffTree[position].containsSecond = true;
                }
            }
        }
    }
}

// вычисл€ем masSubstr дл€ вершин. 
size_t findMaxSubstr(size_t position = 0) {
    std::vector < size_t > &edges = suffTree[position].edges;
    Vertex &vertex = suffTree[position];
    //≈сли из вершины тостижимы только суффиксы из 1 слова, то UNDEFINED
    if (!vertex.containsFirst || !vertex.containsSecond) {
        return UNDEFINED;
    }
    suffTree[position].maxSubstr = 0;
    for (size_t i = 0; i < edges.size(); i++) {
        if (edges[i] != UNDEFINED) {
            Edge edge = edgeList[edges[i]];
            if (edge.target != UNDEFINED) {
                size_t next = findMaxSubstr(edge.target);
                if (next != UNDEFINED) {
                    suffTree[position].maxSubstr = std::max(suffTree[position].maxSubstr, edge.end - edge.begin + next);
                }
            }
        }
    }
    return suffTree[position].maxSubstr;
}

void printMaxSubstr(const std::string &str, size_t position = 0) {
    std::vector < size_t > &edges = suffTree[position].edges;
    Vertex &vertex = suffTree[position];
    if (vertex.maxSubstr == 0) {
        return;
    }
    for (size_t i = 0; i < edges.size(); i++) {
        if (edges[i] != UNDEFINED) {
            Edge edge = edgeList[edges[i]];
            if (edge.target != UNDEFINED) {
                size_t next = suffTree[edge.target].maxSubstr;
                if (next + edge.end - edge.begin == vertex.maxSubstr) {
                    for (size_t k = edge.begin; k < edge.end; k++) {
                        std::cout << str[k];
                    }
                    return printMaxSubstr(str, edge.target);
                }
            }
        }
    }
}

int main() {
    std::string str1, str2, str;
    std::cin >> str1 >> str2;
    str = str1 + '$' + str2 + '#';
    buildSuffTree(str);
    findWordsParts(str1.size());
    std::cout << findMaxSubstr() << std::endl;
    printMaxSubstr(str);
    std::cout << std::endl;
    return 0;
}
