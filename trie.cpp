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
#include <iterator>

using namespace std;

template<typename T> class TrieIterator;

template<typename T>
class Trie {
   friend class TrieTest;
   friend class TrieIterator<T>;

private:
   class Node {
   private:
      Node *_parent;
      map<T, Node *> _children;
      T _value;
      bool _end;

      void _deep_copy_tree(Node *src, Node *dst, Node *parent) {
         dst->_value = src->_value;
         dst->_end = src->_end;
         dst->_parent = parent;
         for (auto it = src->_children.begin(); 
            it != src->_children.end(); ++it) {
            Node *n = new Node();
            _deep_copy_tree(it->second, n, dst);
            dst->_children[n->_value] = n;
         }
      }

      void _delete_children() {
         for (auto nit = _children.begin(); nit != _children.end(); ++nit)
            delete nit->second;
      }

   public:
      Node(): _parent(nullptr), _value(T()), _end(false) {}

      Node(Node *parent, T value, bool end): 
         _parent(parent), _value(value), _end(end) {}

      Node(const Node &other) {
         _deep_copy_tree(const_cast<Node *>(&other), this, other._parent);
      }

      Node& operator=(const Node &other) {
         _delete_children();
         _deep_copy_tree(const_cast<Node *>(&other), this, other._parent);
      }

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

      string str() {
         stringstream ss;
         ss << "(parent: " << _parent << ", value: " << _value
            << ", end: " << _end << ", children: ";
         for (auto it = _children.begin(); it != _children.end(); ++it) {
            ss << it->second;
            ss << (it == prev(_children.end(), 1) ? "" : " ");
         }
         ss << ")";
         return ss.str();
      }

      ~Node() { _delete_children(); }
   };

   Node _root;
   function<vector<T>(T)> _split;
   typedef priority_queue<Node *, vector<Node *>, 
      function<bool(const Node *, const Node *)>> _NodePtrPQ; 

   static string _str(Node *n, const string &indent = "") {
      stringstream ss;
      ss << indent << n->value() << endl;
      auto children = n->children();
      
      _NodePtrPQ pq([](const Node *a, const Node *b) -> bool {
         return *a > *b; });

      for (auto cit = children.begin(); cit != children.end(); ++cit)
         pq.push(cit->second);
      while (!pq.empty()) {
         ss << _str(pq.top(), indent + " ");
         pq.pop(); 
      }
      return ss.str(); 
   }

public:
   class iterator: public TrieIterator<T> {
   private:
      T *_val;
   public:
      iterator(): _val(nullptr) {}
      iterator(T *val): _val(val) {}
      bool operator==(const iterator *other) { return _val == other._val; }
      bool operator!=(const iterator *other) { return _val != other._val; }
      T& operator*() { return *_val; }
   };

   Trie(const function<vector<T>(T)> &split_algo): 
      _split(split_algo) {}

   // TODO
   Trie(const Trie &other): _root(other) {}

   void insert(const T &key) {
      vector<T> subkeys = _split(key);
      Node *curr = &_root;
      for (auto key_it = subkeys.begin(); key_it != subkeys.end(); ++key_it) {
         if (curr->has_child(*key_it))
            curr = curr->get_child(*key_it);
         else
            curr = curr->add_child(*key_it,
               key_it == subkeys.end() - 1 ? true : false);
      }
   }

   // TODO: implement this after end()
   // T find(const T &key) {
   //    vector<T> subkeys = _split(key);
   //    Node *curr = &_root;
   //    for (auto key_it = subkeys.begin(); key_it != subkeys.end(); ++key_it) {
   //       if (curr_has)   
   //    }
   // }

   // TODO test this after copy ctor and assignment operator
   iterator end() { return iterator(); }

   string str() { return _str(&_root); }
};

template<typename T>
class TrieIterator: public iterator<random_access_iterator_tag, T> {

};

class TrieTest {
private:
   function<vector<int>(int)> get_int_split_func() {
      return [](int val) -> vector<int> {
         vector<int> v;
         while (val) {
            v.emplace_back(val%10);
            val /= 10;   
         }
         reverse(v.begin(), v.end());
         return v;
      };
   }

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
      cout << t.str();
      assert(t.str() == 
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
      cout << t.str();

      assert(t.str() == 
         "\n"
         " foo\n"
         "  bar\n"
         "  baz\n"
         " mu\n"
         "  bar\n");
   }

   void node_copy_ctor() {
      typedef Trie<int>::Node Node;
      typedef Trie<int> IntTrie;

      Node root;
      Node copy = root;
      assert(copy.children().empty());

      Node *child_a = new Node(&root, 1, true);
      root.children()[1] = child_a;

      Node copy2 = root;
      assert(root.children().at(1) != copy2.children().at(1));
      assert(root.children().at(1)->value() == copy2.children().at(1)->value());

      Node *child_b = new Node(&root, 2, true);
      root.children()[2] = child_b;
      Node *child_c = new Node(child_a, 3, true);
      child_a->children()[3] = child_c;

      Node copy3 = root;
      assert(root.str() != copy3.str());
      assert(IntTrie::_str(&copy3) == IntTrie::_str(&root));
   }

   void node_copy_assign() {
      typedef Trie<int>::Node Node;
      typedef Trie<int> IntTrie;

      Node root;
      Node copy;
      copy = root;
      assert(copy.children().empty());

      Node *child_a = new Node(&root, 1, true);
      root.children()[1] = child_a;

      Node copy2;
      copy2 = root;
      assert(root.children().at(1) != copy2.children().at(1));
      assert(root.children().at(1)->value() == copy2.children().at(1)->value());

      Node *child_b = new Node(&root, 2, true);
      root.children()[2] = child_b;
      Node *child_c = new Node(child_a, 3, true);
      child_a->children()[3] = child_c;

      Node copy3;
      copy3 = root;
      assert(root.str() != copy3.str());
      assert(IntTrie::_str(&copy3) == IntTrie::_str(&root));
   }
};

int main() {
   TrieTest test;
   test.node_test();
   test.trie_insert_int_test();
   test.trie_insert_string_test();
   test.node_copy_ctor();
   test.node_copy_assign();
}
