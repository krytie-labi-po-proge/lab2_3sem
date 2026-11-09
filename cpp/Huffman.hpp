#pragma once
#include <iostream>
#include <optional>
#include <string>
#include <fstream>
#include <filesystem>

#include "Array.hpp"
#include "ChainTable.hpp"
#include "Queue.hpp"
#include "Util.hpp"
#include "Value.hpp"

namespace huffman {

struct Node {
    int weight;
    char symbol;
    Node* left;
    Node* right;

    Node(int w, char s, Node* l, Node* r)
        : weight(w), symbol(s), left(l), right(r) {}
};

struct CodeTree {
    Node* root;
    Node** nodes;
    int nodeCount;
    int nodeCapacity;

    CodeTree() : root(nullptr), nodes(nullptr), nodeCount(0), nodeCapacity(0) {}

    ~CodeTree() {
        destroy(root);
        root = nullptr;

        delete[] nodes;
        nodes = nullptr;
        nodeCount = 0;
        nodeCapacity = 0;
    }

    void destroy(Node* node) {
        if (node == nullptr) {
            return;
        }

        destroy(node->left);
        destroy(node->right);
        delete node;
    }

    void resize(int newCapacity) {
        Node** newNodes = new Node*[newCapacity];

        for (int i = 0; i < nodeCount; ++i) {
            newNodes[i] = nodes[i];
        }

        delete[] nodes;
        nodes = newNodes;
        nodeCapacity = newCapacity;
    }

    int add(int w, char s, Node* l, Node* r) {
        if (nodeCount == nodeCapacity) {
            int newCapacity = nodeCapacity == 0 ? 16 : nodeCapacity * 2;
            resize(newCapacity);
        }

        Node* node = new Node(w, s, l, r);
        nodes[nodeCount] = node;
        ++nodeCount;

        return nodeCount - 1;
    }

    Node* nodeOf(int id) const {
        return nodes[id];
    }

    int weightOf(int id) const {
        return nodeOf(id)->weight;
    }

    bool isLeaf(Node* node) const {
        return node != nullptr && node->left == nullptr && node->right == nullptr;
    }

    char symbolOf(Node* node) const {
        return node->symbol;
    }
};

void countFrequencies(const std::string& text, ChainTable& counts, Array& symbols) {
    for (char c : text) {
        Value key(c);
        std::optional<Value> current = counts.get(key);
        if (current) {
            counts.put(key, Value(current->asInt() + 1));
        } else {
            counts.put(key, Value(1));
            symbols.pushBack(key);
        }
    }
}

bool precedes(const Value& a, const Value& b, ChainTable& counts) {
    int fa = counts.get(a)->asInt();
    int fb = counts.get(b)->asInt();
    if (fa != fb) return fa < fb;
    return static_cast<unsigned char>(a.asChar()) < static_cast<unsigned char>(b.asChar());
}

void sortSymbols(Array& symbols, ChainTable& counts) {
    for (int i = 1; i < symbols.length(); ++i) {
        Value current = symbols.get(i);
        int j = i - 1;
        while (j >= 0 && precedes(current, symbols.get(j), counts)) {
            symbols.set(j + 1, symbols.get(j));
            --j;
        }
        symbols.set(j + 1, current);
    }
}

int takeLightest(const CodeTree& tree, Queue& leaves, Queue& merged) {
    if (merged.length() == 0) return leaves.pop().asInt();
    if (leaves.length() == 0) return merged.pop().asInt();
    if (tree.weightOf(leaves.front().asInt()) <= tree.weightOf(merged.front().asInt())) return leaves.pop().asInt();
    return merged.pop().asInt();
}

int buildTree(const Array& symbols, ChainTable& counts, CodeTree& tree) {
    Queue leaves, merged;
    for (int i = 0; i < symbols.length(); ++i) {
        Value symbol = symbols.get(i);
        int id = tree.add(counts.get(symbol)->asInt(), symbol.asChar(), nullptr, nullptr);
        leaves.push(Value(id));
    }
    while (leaves.length() + merged.length() > 1) {
        int first = takeLightest(tree, leaves, merged);
        int second = takeLightest(tree, leaves, merged);
        int parent = tree.add(tree.weightOf(first) + tree.weightOf(second), '\0', tree.nodeOf(first), tree.nodeOf(second));
        merged.push(Value(parent));
    }
    if (leaves.length() > 0) {
        return leaves.pop().asInt();
    }
    return merged.pop().asInt();
}

void assignCodes(const CodeTree& tree, Node* node, std::string prefix, ChainTable& codes) {
    if (node == nullptr) {
        return;
    }

    if (tree.isLeaf(node)) {
        codes.put(Value(tree.symbolOf(node)), Value(prefix.empty() ? std::string("0") : prefix));
        return;
    }

    assignCodes(tree, node->left, prefix + "0", codes);
    assignCodes(tree, node->right, prefix + "1", codes);
}

void saveTree(const Array& symbols, ChainTable& counts, const char* filename) {
    std::ofstream out(filename);

    if (!out.is_open()) {
        std::cout << "ERROR: не удалось открыть файл для записи\n";
        return;
    }

    for (int i = 0; i < symbols.length(); ++i) {
        Value symbol = symbols.get(i);
        int code = static_cast<unsigned char>(symbol.asChar());
        int frequency = counts.get(symbol)->asInt();

        out << code << " " << frequency << "\n";
    }

    out.close();
}

int loadTree(const char* filename, ChainTable& counts, Array& symbols, CodeTree& tree) {
    std::ifstream in(filename);

    if (!in.is_open()) {
        return -1;
    }

    int code;
    int frequency;

    while (in >> code >> frequency) {
        if (code < 0 || code > 255 || frequency <= 0) {
            return -1;
        }

        Value key(static_cast<char>(code));

        if (counts.get(key)) {
            return -1;
        }

        counts.put(key, Value(frequency));
        symbols.pushBack(key);
    }

    if (!in.eof() || symbols.length() == 0) {
        return -1;
    }

    in.close();

    sortSymbols(symbols, counts);

    int root = buildTree(symbols, counts, tree);
    tree.root = tree.nodeOf(root);

    return root;
}

std::string encode(const std::string& text, ChainTable& codes) {
    std::string bits;
    for (char c : text) bits += codes.get(Value(c))->toString();
    return bits;
}

bool decode(const std::string& bits, const CodeTree& tree, int root, std::string& out) {
    out.clear();

    if (root < 0 || root >= tree.nodeCount) {
        return false;
    }

    if (tree.isLeaf(tree.nodeOf(root))) {
        for (char b : bits) {
            if (b != '0') return false;
            out += tree.symbolOf(tree.nodeOf(root));
        }
        return true;
    }

    Node* current = tree.nodeOf(root);

    for (char b : bits) {
        if (b == '0') current = current->left;
        else if (b == '1') current = current->right;
        else return false;

        if (current == nullptr) return false;

        if (tree.isLeaf(current)) {
            out += tree.symbolOf(current);
            current = tree.nodeOf(root);
        }
    }

    return current == tree.nodeOf(root);
}

std::string label(char c) {
    if (c == '\n') return "\\n";
    if (c == '\t') return "\\t";
    if (c == ' ') return "' '";
    return std::string(1, c);
}

int process_stdin(int argc, char* argv[]) {
    std::string modeString;
    std::string text;
    std::string filename;

    bool hasMode = false;
    bool hasQuery = false;
    bool hasFilename = false;

    for (int i = 1; i < argc; ++i) {
        std::string argument(argv[i]);

        if (argument == "--mode") {
            if (i + 1 >= argc) {
                std::cout << "ERROR: после --mode не указано значение\n";
                return 0;
            }

            modeString = argv[i + 1];
            hasMode = true;
            ++i;
        } else if (argument == "--query") {
            if (i + 1 >= argc) {
                std::cout << "ERROR: после --query не указано значение\n";
                return 0;
            }

            text = argv[i + 1];
            hasQuery = true;
            ++i;
        } else if (argument == "--filename") {
            if (i + 1 >= argc) {
                std::cout << "ERROR: после --filename не указано значение\n";
                return 0;
            }

            filename = argv[i + 1];
            hasFilename = true;
            ++i;
        }
    }

    if (!hasMode || !hasQuery || !hasFilename) {
        std::cout << "ERROR: необходимо указать --mode, --query и --filename\n";
        return 0;
    }

    if (modeString != "encode" && modeString != "decode") {
        std::cout << "ERROR: режим должен быть encode или decode\n";
        return 0;
    }

    std::cout << "Режим (encode/decode): " << modeString << '\n';
    std::cout << "Строка: " << text << '\n';
    std::cout << "Файл таблицы: " << filename << '\n';

    bool mode = modeString == "encode";

    while (!text.empty() && (text.back() == '\n' || text.back() == '\r')) {
        text.pop_back();
    }

    if (text.empty()) {
        std::cout << "ERROR: пустая строка\n";
        return 0;
    }

    ChainTable counts(16, true);
    ChainTable codes(16, true);
    Array symbols;
    CodeTree tree;

    int root = -1;

    if (mode) {
        countFrequencies(text, counts, symbols);
        sortSymbols(symbols, counts);

        root = buildTree(symbols, counts, tree);
        tree.root = tree.nodeOf(root);

        assignCodes(tree, tree.nodeOf(root), "", codes);
        saveTree(symbols, counts, filename.c_str());
    } else {
        if (!std::filesystem::exists(filename)) {
            std::cout << "Файл таблицы не существует\n";
            return 0;
        }

        root = loadTree(filename.c_str(), counts, symbols, tree);

        if (root == -1) {
            std::cout << "ERROR: не удалось загрузить таблицу\n";
            return 0;
        }
    }

    if (mode) {
        std::string bits = encode(text, codes);

        std::cout << "Исходная строка: " << text << '\n';
        std::cout << "Таблица (символ, частота, код):\n";

        for (int i = 0; i < symbols.length(); ++i) {
            Value s = symbols.get(i);
            std::cout << "  " << label(s.asChar()) << "  " << counts.get(s)->asInt() << "  " << codes.get(s)->toString() << '\n';
        }

        std::cout << "Код: " << bits << '\n';
        std::cout << "Длина кода: " << bits.size() << " бит, исходная строка: " << text.size() * 8 << " бит\n";
    } else {
        std::string restored;

        if (!decode(text, tree, root, restored)) {
            std::cout << "ERROR: код не удалось расшифровать\n";
            return 0;
        }

        std::cout << "Восстановленная строка: " << restored << '\n';
    }

    return 0;
}

}