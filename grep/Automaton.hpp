#ifndef _AUTOMATON_
#define _AUTOMATON_

#include <vector>
#include <set>
#include <string>

class Automaton {
    friend class DeterministicAutomaton;
private:
    struct Node {
        static const unsigned char EPSILON = 0;
        std::vector< std::set< Node * > > edges;
        std::vector< unsigned char > existingEdges;
        size_t linksNumber;
        bool terminating;
        bool toDelete;
        Node(): linksNumber(0), edges(1 << (8 * sizeof(char))), toDelete(false) {}
    };
    Node *startState;
    Node *terminatingState;
    std::vector< Node* > deleted;

    void clear() {
        for (size_t i = 0; i < deleted.size(); i++)
            delete deleted[i];
        deleted.clear();
    }

    void markToDeleteHard(Node *node) {
        if (node->toDelete)
            return;
        node->toDelete = true;
        for (size_t i = 0; i < node->existingEdges.size(); i++) {
            std::set< Node * > &edgeSet = node->edges[node->existingEdges[i]];
            std::set< Node * >::iterator it;
            for (it = edgeSet.begin(); it != edgeSet.end(); it++) {
                markToDeleteHard(*it);
            }
        }
    }

    void markToDeleteSoft(Node *node) {
        if (node->toDelete)
            return;
        node->toDelete = true;
        for (size_t i = 0; i < node->existingEdges.size(); i++) {
            std::set< Node * > &edgeSet = node->edges[node->existingEdges[i]];
            std::set< Node * >::iterator it;
            for (it = edgeSet.begin(); it != edgeSet.end(); it++) {
                if (--(*it)->linksNumber == 0 && (*it) != startState)
                    markToDeleteSoft(*it);
            }
        }
    }

    void deleteSubtree(Node *node) {
        markToDeleteHard(node);
        clear();
    }

    void deleteUselessNode(Node *node) {
        markToDeleteSoft(node);
        clear();
    }

    void createEdge(Node *source, Node *destination, unsigned char letter) {
        if (source->edges[letter].empty())
            source->existingEdges.push_back(letter);
        if (source->edges[letter].find(destination) == source->edges[letter].end())
            destination->linksNumber++;
        source->edges[letter].insert(destination);
    }

    void createEmptyAutomaton() {
        startState = new Node();
        terminatingState = startState;
    }

public:
    Automaton() {
        createEmptyAutomaton();    
    }

    Automaton(unsigned char edge) {
        createEmptyAutomaton(); 
        terminatingState = new Node();
        createEdge(startState, terminatingState, edge);
    }

    Automaton(const std::string &str, bool isInverted) {
        createEmptyAutomaton();
        terminatingState = new Node();
        std::vector< bool > edgesMap(1 << 8 * (sizeof(char)), false);
        if (str.size()) {
            edgesMap[(unsigned char) str[0]] = true;
            edgesMap[(unsigned char) str[str.size() - 1]] = true;
            //createEdge(startState, terminatingState, str[0]);
            //createEdge(startState, terminatingState, str[str.size() - 1]);
        }
        for (size_t i = 1; i + 1 < str.size(); i++) {
            if (str[i] == '-') {
                for (char c = str[i - 1]; c != str[i + 1]; c++)
                    edgesMap[(unsigned char) c] = true;
                    //if (c)
                    //    createEdge(startState, terminatingState, c);
            } else {
                //createEdge(startState, terminatingState, str[i]);
                edgesMap[(unsigned char) str[i]] = true;
            }
        }
        for (char c = 1; c; c++)
            if (edgesMap[(unsigned char) c] ^ isInverted)
                createEdge(startState, terminatingState, c);
    }

    Automaton(Automaton &automaton) {
        startState = automaton.startState;
        terminatingState = automaton.terminatingState;
        automaton.createEmptyAutomaton();
    }

    ~Automaton() {
        deleteSubtree(startState);
    }

    void concatenate(Automaton &automaton) {
        createEdge(terminatingState, automaton.startState, Node::EPSILON);
        terminatingState = automaton.terminatingState;
        automaton.createEmptyAutomaton();
    }

    void unite(Automaton &automaton) {
        createEdge(startState, automaton.startState, Node::EPSILON);
        createEdge(automaton.terminatingState, terminatingState, Node::EPSILON);
        automaton.createEmptyAutomaton();
    }

    void makePositiveIterationOfKleene() {
        createEdge(terminatingState, startState, Node::EPSILON);
    }

    void makeIterationOfKleene() {
        makePositiveIterationOfKleene();
        createEdge(startState, terminatingState, Node::EPSILON);
    }

    void addAnyCaracter(bool mayBeEmpty) {
        Node *node = new Node;
        terminatingState->existingEdges.clear();
        for (size_t i = 0; i < terminatingState->edges.size(); i++) {
            if (i != Node::EPSILON || mayBeEmpty) {
                terminatingState->edges[i].insert(node);
                terminatingState->existingEdges.push_back(i);
            }
        }
        terminatingState = node;
    }
};

#endif // _AUTOMATON_