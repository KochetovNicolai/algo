#ifndef _DETERMINISTIC_AUTOMATON_
#define _DETERMINISTIC_AUTOMATON_

#include <vector>
#include <set>
#include <map>

#include "Automaton.hpp"

class DeterministicAutomaton {
private:
    struct Node {
        static const unsigned char EPSILON = 0;
        std::vector< Node * > edges;
        std::vector< unsigned char > existingEdges;
        bool terminating;
        bool toDelete;
        Node(): edges(1 << (8 * sizeof(char)), NULL), terminating(false), toDelete(false) {}
    };
    Node *startState;
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
            markToDeleteHard(node->edges[node->existingEdges[i]]);
        }
    }

    void deleteSubtree(Node *node) {
        markToDeleteHard(node);
        clear();
    }

    bool addEpsilonReachableEdges(Automaton::Node *node, Automaton::Node *startNode, 
                std::set< Automaton::Node * > &epsilonReachable, Automaton &automaton) {
        if (epsilonReachable.find(node) != epsilonReachable.end())
            return false;
        epsilonReachable.insert(node);
        bool terminating = node == automaton.terminatingState;
        std::set< Automaton::Node * >::iterator it;
        std::set< Automaton::Node * > &epsilonEdges = node->edges[Automaton::Node::EPSILON];
        for (it = epsilonEdges.begin(); it != epsilonEdges.end(); it++) {
            terminating |= addEpsilonReachableEdges(*it, startNode, epsilonReachable, automaton);
        }
        for (size_t i = 0; i < node->existingEdges.size(); i++) {
            if (node->existingEdges[i] != Automaton::Node::EPSILON) {
                std::set< Automaton::Node * > &edges = node->edges[node->existingEdges[i]];
                for (it = edges.begin(); it != edges.end(); it++)
                    automaton.createEdge(startNode, *it, node->existingEdges[i]);
            }
        }
        return terminating;
    }

    void pumpEdges(Automaton &automaton, Automaton::Node *node) {
        if (node->toDelete)
            return;
        node->toDelete = true;
        std::set< Automaton::Node * > epsilonReachable;
        node->terminating = addEpsilonReachableEdges(node, node, epsilonReachable, automaton);
        for (size_t i = 0; i < node->existingEdges.size(); i++) {
            std::set< Automaton::Node * > &edgeSet = node->edges[node->existingEdges[i]];
            std::set< Automaton::Node * >::iterator it;
            for (it = edgeSet.begin(); it != edgeSet.end(); it++)
                pumpEdges(automaton, *it);
        }
    }

    void removeEpsilons(Automaton &automaton, Automaton::Node *node) {
        if (!node->toDelete)
            return;
        node->toDelete = false;
        for (size_t i = 0; i < node->existingEdges.size(); i++) {
            std::set< Automaton::Node * > &edgeSet = node->edges[node->existingEdges[i]];
            std::set< Automaton::Node * >::iterator it;
            for (it = edgeSet.begin(); it != edgeSet.end(); it++) {
                if (node->existingEdges[i] == Automaton::Node::EPSILON) {
                    if (--(*it)->linksNumber == 0 && *it != automaton.startState)
                        automaton.deleteUselessNode(*it);
                } else {
                    removeEpsilons(automaton, *it);
                }
            }
            if (node->existingEdges[i] == Automaton::Node::EPSILON) {
                std::swap(node->existingEdges[i], node->existingEdges.back());
                node->existingEdges.pop_back();
            }
        }
        node->edges[Automaton::Node::EPSILON].clear();
    }

    Node *createNode(std::set< Automaton::Node * > &nodes) {
        std::set< Automaton::Node * >::iterator it;
        std::set< unsigned char > edges;
        Node *node = new Node();
        for (it = nodes.begin(); it != nodes.end(); it++) {
            node->terminating |= (*it)->terminating;
            for (size_t i = 0; i < (*it)->existingEdges.size(); i++)
                edges.insert((*it)->existingEdges[i]);
        }
        node->existingEdges.assign(edges.begin(), edges.end());
        return node;
    }

    std::set< Automaton::Node * > goByEdge(std::set< Automaton::Node * > &nodes, unsigned char edge) {
        std::set< Automaton::Node * > nextSet;
        std::set< Automaton::Node * >::iterator it;
        for (it = nodes.begin(); it != nodes.end(); it++)
            nextSet.insert((*it)->edges[edge].begin(), (*it)->edges[edge].end());
        return nextSet;
    }

    Node *buildDeterministicAutomaton(std::set< Automaton::Node * > node, std::map< std::set< Automaton::Node * >, Node * > &nodes) {
        std::map< std::set< Automaton::Node * >, Node * >::iterator it = nodes.find(node);
        if (it == nodes.end()) {
            Node *ptr = createNode(node);
            nodes[node] = ptr;
            for (size_t i = 0; i < ptr->existingEdges.size(); i++) {
                ptr->edges[ptr->existingEdges[i]] = buildDeterministicAutomaton(goByEdge(node, ptr->existingEdges[i]), nodes);
            }
            return ptr;
        }
        return it->second;
    }

public:

    DeterministicAutomaton(Automaton &automaton) {
        pumpEdges(automaton, automaton.startState);
        removeEpsilons(automaton, automaton.startState);
        std::map< std::set< Automaton::Node * >, Node * > nodes;
        std::set< Automaton::Node * > startNode;
        startNode.insert(automaton.startState);
        startState = buildDeterministicAutomaton(startNode, nodes);
        automaton.deleteSubtree(automaton.startState);
        automaton.createEmptyAutomaton();
    }

    std::vector< std::vector< size_t > > grep(const std::string &str) {
        std::vector< std::vector< size_t > > entries(str.size());
        std::vector< Node * > positions(str.size(), startState);
        for (size_t i = 0; i < str.size(); i++) {
            if (startState->terminating)
                entries[i].push_back(i);
            for (size_t j = 0; j <= i; j++) {
                if (positions[j] != NULL)
                    positions[j] = positions[j]->edges[(unsigned char)str[i]];
                if (positions[j] != NULL && positions[j]->terminating)
                    entries[j].push_back(i + 1);
            }
        }
        return entries;
    }
};

#endif // _DETERMINISTIC_AUTOMATON_