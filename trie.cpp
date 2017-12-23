#include <iostream>
#include <cassert>
#include <memory>
#include <functional>
#include <map>
#include <vector>
#include <algorithm>
#include <sstream>
#include <queue>
#include <string>
#include <regex>

using namespace std;

template<typename T>
class Trie {
   friend class TrieTest;

private:
   class Node {
   private:
      Node *_parent;
      map<T, Node *> _children;
      T _value;
      bool _end;

   public:
      Node(): _parent(nullptr), _value(T()), _end(false) {}

      Node(Node *parent, T value, bool end): 
         _parent(parent), _value(value), _end(end) {}

      bool has_child(const T &key) const {
         return _children.find(key) != _children.end();   
      }

      Node *get_child(const T &key) {
         return _children.at(key);
      }

      Node *add_child(const T &key, bool end) {
         _children[key] = new Node(this, key, end);
         return _children[key];
      }

      T value() const { return _value; }

      map<T, Node *>& children() { return _children; }

      bool operator==(const Node &other) const { 
         return _value == other._value;
      }

      bool operator!=(const Node &other) const { 
         return _value != other._value;
      }

      bool operator<(const Node &other) const { 
         return _value < other._value; 
      }

      bool operator>(const Node &other) const { 
         return _value > other._value; 
      }

      bool operator<=(const Node &other) const { 
         return _value <= other._value; 
      }

      bool operator>=(const Node &other) const { 
         return _value >= other._value; 
      }

      ~Node() {
         for (auto nit = _children.begin(); nit != _children.end(); ++nit)
            delete nit->second;
      }
   };

   Node *_root;
   function<vector<T>(T)> _split;
   typedef priority_queue<Node *, vector<Node *>, 
      function<bool(const Node *, const Node *)>> _NodePtrPQ; 

   string _dump(Node *n, const string &indent = "") {
      stringstream ss;
      ss << indent << n->value() << endl;
      auto children = n->children();
      
      _NodePtrPQ pq([](const Node *a, const Node *b) -> bool {
         return *a > *b; });

      for (auto cit = children.begin(); cit != children.end(); ++cit)
         pq.push(cit->second);
      while (!pq.empty()) {
         ss << _dump(pq.top(), indent + " ");
         pq.pop(); 
      }
      return ss.str(); 
   }

public:
   Trie(const function<vector<T>(T)> &split_algo): 
      _split(split_algo), _root(new Node()) {}

   void insert(const T &val) {
      vector<T> keys = _split(val);
      Node *curr = _root;
      for (auto key_it = keys.begin(); key_it != keys.end(); ++key_it) {
         if (curr->has_child(*key_it))
            curr = curr->get_child(*key_it);
         else
            curr = curr->add_child(*key_it,
               key_it == keys.end() - 1 ? true : false);
      }
   }

   string dump() {
      return _dump(_root);
   }
};

class TrieTest {
public:
   void node_test() {
      Trie<int>::Node root;
      Trie<int>::Node child_a(&root, 1, true);
      Trie<int>::Node child_b(&root, 2, false);
      Trie<int>::Node child_c(&child_b, 3, true);
      assert(child_a == child_a);
      assert(child_a != child_b);
      assert(child_a < child_b);
      assert(child_b > child_a);
      assert(child_a <= child_b);
      assert(child_b >= child_a);
   }

   void trie_insert_int_test() {
      Trie<int> t([](int val) -> vector<int> {
         vector<int> v;
         while (val) {
            v.emplace_back(val%10);
            val /= 10;   
         }
         reverse(v.begin(), v.end());
         return v; });

      t.insert(124);
      t.insert(123);
      t.insert(9821);
      t.insert(972);
      cout << t.dump();
      assert(t.dump() == 
         "0\n"
         " 1\n"
         "  2\n"
         "   3\n"
         "   4\n"
         " 9\n"
         "  7\n"
         "   2\n"
         "  8\n"
         "   2\n"
         "    1\n");
   }

   void trie_insert_string_test() {
      Trie<string> t([](string val) -> vector<string> {
         vector<string> v;
         regex period("\\.");
         regex_token_iterator<string::iterator> tokit(
            val.begin(), val.end(), period, -1);
         regex_token_iterator<string::iterator> tokend;
         while (tokit != tokend) v.emplace_back(*tokit++);
         return v;
      });

      t.insert("foo");
      t.insert("foo.bar");
      t.insert("mu");
      t.insert("mu.bar");
      t.insert("foo.baz");
      cout << t.dump();

      assert(t.dump() == 
         "\n"
         " foo\n"
         "  bar\n"
         "  baz\n"
         " mu\n"
         "  bar\n");
   }
};

int main() {
   TrieTest test;
   test.node_test();
   test.trie_insert_int_test();
   test.trie_insert_string_test();
}
