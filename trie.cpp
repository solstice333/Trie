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
         _children.clear();
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

      Node *add_child(const T &key, bool end = false) {
         _children[key] = new Node(this, key, end);
         return _children[key];
      }

      Node *parent() { return _parent; }

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

      bool end() { return _end; }

      void end(bool val) { _end = val; }

      vector<T> subkeys() {
         vector<T> skeys;
         for (Node *curr = this; curr->_parent; curr = curr->_parent)
            skeys.emplace_back(curr->_value);
         reverse(skeys.begin(), skeys.end());
         return skeys;
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

   Node *_root;
   function<vector<T>(T)> _split;
   function<T(const vector<T>&)> _concat;

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
   class iterator: public std::iterator<random_access_iterator_tag, Node> {
      friend class Trie<T>;
   private:
      typedef function<T(const vector<T>&)> ConcatFunc;
      Node *_val;
      vector<T> _subkeys;
      ConcatFunc _concat;

      iterator(): _val(nullptr) {}
      iterator(Node *val, const ConcatFunc &concat_algo): 
         _val(val), _subkeys(val->subkeys()), _concat(concat_algo) {}
      iterator _parent() const { return iterator(_val->parent(), _concat); }
      bool _is_root() { return !_val->parent(); }

   public:
      bool operator==(const iterator &other) { return _val == other._val; }
      bool operator!=(const iterator &other) { return _val != other._val; }
      T operator*() { return _concat(_subkeys); }
   };

   Trie(const function<vector<T>(T)> &split_algo, 
      const function<T(vector<T>)> &concat_algo):
      _root(new Node()), _split(split_algo), _concat(concat_algo) {}

   Trie(const Trie<T> &other): 
      _root(new Node()), _split(other._split), _concat(other._concat) {
      *_root = *other._root;
   }

   Trie& operator=(const Trie<T> &other) {
      delete _root;
      _root = new Node();
      *_root = *other._root;
      _split = other._split;
      _concat = other._concat;
   }

   void insert(const T &key) {
      vector<T> subkeys = _split(key);
      Node *curr = _root;
      for (auto key_it = subkeys.begin(); key_it != subkeys.end(); ++key_it)
         curr = curr->has_child(*key_it) ? 
            curr->get_child(*key_it) : curr->add_child(*key_it);
      curr->end(true);
   }

   iterator find(const T &key) {
      vector<T> subkeys = _split(key);
      Node *curr = _root;
      for (auto key_it = subkeys.begin(); key_it != subkeys.end(); ++key_it) {
         if (curr->has_child(*key_it))
            curr = curr->get_child(*key_it);
         else
            return end();
      }
      return curr->end() ? iterator(curr, _concat) : end();
   }

   iterator find_parent(const iterator &it) { 
      iterator itr = it._parent();
      return itr._is_root() ? end() : itr;
   }

   iterator end() { return iterator(); }
   string str() { return _str(_root); }
   ~Trie() { delete _root; }
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

   function<int(const vector<int>&)> get_int_concat_func() {
      return [](const vector<int> &v) -> int {
         int val = 0;
         for (auto it = v.begin(); it != v.end(); ++it) {
            val += *it;
            if (it != prev(v.end(), 1))
               val *= 10;
         }
         return val;
      };
   }

   function<vector<string>(string)> get_string_split_func() {
      return [](string val) -> vector<string> {
         vector<string> v;
         regex period("\\.");
         regex_token_iterator<string::iterator> tokit(
            val.begin(), val.end(), period, -1);
         regex_token_iterator<string::iterator> tokend;
         while (tokit != tokend) v.emplace_back(*tokit++);
         return v;
      };
   }

   function<string(const vector<string>&)> get_string_concat_func() {
      return [](const vector<string> &v) -> string {
         stringstream ss;
         for (auto i = v.begin(); i != v.end(); ++i) {
            ss << *i;
            if (i != prev(v.end(), 1))
               ss << ".";
         }
         return ss.str();
      };
   }

   Trie<int> get_mock_int_trie() {
      return Trie<int>(get_int_split_func(), get_int_concat_func());
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
      Trie<int> t(get_int_split_func(), get_int_concat_func());

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
      Trie<string> t(get_string_split_func(), get_string_concat_func());

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

   void trie_copy_ctor() {
      typedef Trie<int> IntTrie;
      typedef Trie<int>::Node IntNode;

      IntTrie t(get_int_split_func(), get_int_concat_func());
      t.insert(123);
      t.insert(122);
      IntTrie t2 = t;

      assert(t.str() == t2.str());

      t.insert(23);
      t2.insert(13);

      assert(t.str() != t2.str());
      assert(t._root != t2._root);
      assert(t._root->children().at(1) != t2._root->children().at(1));

      assert(*t2.find(123) == 123);
      assert(*t.find(123) == 123);
      assert(*t2.find(13) == 13);
      assert(*t.find(23) == 23);
   }

   void trie_copy_assign() {
      typedef Trie<int> IntTrie;
      typedef Trie<int>::Node IntNode;

      IntTrie t(get_int_split_func(), get_int_concat_func());
      t.insert(123);
      t.insert(122);
      IntTrie t2(
         [](int x) -> vector<int> { return vector<int>(); }, 
         [](const vector<int> &v) -> int { return 0; });
      t2 = t;

      assert(t.str() == t2.str());

      t.insert(23);
      t2.insert(13);

      assert(t.str() != t2.str());
      assert(t._root != t2._root);
      assert(t._root->children().at(1) != t2._root->children().at(1));

      assert(*t2.find(123) == 123);
      assert(*t.find(123) == 123);
      assert(*t2.find(13) == 13);
      assert(*t.find(23) == 23);
   }

   void trie_get_mock() {
      auto t = get_mock_int_trie();
      t.insert(143);
      t.insert(132);
      assert(t.str() == 
         "0\n"
         " 1\n"
         "  3\n"
         "   2\n"
         "  4\n"
         "   3\n");
   }

   void trie_end_iter() {
      auto t = get_mock_int_trie();
      assert(t.end() == t.end());
   }

   void trie_find() {
      auto t = get_mock_int_trie();
      t.insert(143);
      t.insert(132);
      assert(t.find(143) != t.end());
      assert(*t.find(143) == 143);
      assert(t.find(132) != t.end());
      assert(*t.find(132) == 132);

      assert(t.find(25) == t.end());
      assert(t.find(400) == t.end());
      assert(t.find(13) == t.end());
      assert(t.find(0) == t.end());

      t.insert(13);
      assert(t.find(13) != t.end());
      assert(t.find(1) == t.end());

      t.insert(1);
      assert(t.find(1) != t.end());
   }

   void trie_find_string_test() {
      Trie<string> t(get_string_split_func(), get_string_concat_func());

      t.insert("foo");
      t.insert("foo.bar");
      t.insert("mu");
      t.insert("mu.bar");
      t.insert("foo.baz");
 
      assert(t.find("foo") != t.end());
      assert(t.find("foo.bar") != t.end());
      assert(t.find("mu") != t.end());
      assert(t.find("mu.bar") != t.end());
      assert(t.find("foo.baz") != t.end());
      assert(t.find("mu.baz") == t.end());

      assert(*t.find("foo") == "foo");
      assert(*t.find("foo.bar") == "foo.bar");
      assert(*t.find("mu") == "mu");
      assert(*t.find("mu.bar") == "mu.bar");
      assert(*t.find("foo.baz") == "foo.baz");
   }

   void trie_find_parent() {
      Trie<int> t(get_int_split_func(), get_int_concat_func());
      t.insert(482);
      t.insert(410);
      
      auto it = t.find(482);
      it = t.find_parent(it);
      assert(*it == 48);
      it = t.find_parent(it);
      assert(*it == 4);

      it = t.find(410);
      it = t.find_parent(it);
      assert(*it == 41);
      it = t.find_parent(it);
      assert(*it == 4);
      assert(it != t.end());

      it = t.find_parent(it);
      assert(it == t.end());
   }
};

int main() {
   TrieTest test;
   test.node_test();
   test.trie_insert_int_test();
   test.trie_insert_string_test();
   test.node_copy_ctor();
   test.node_copy_assign();
   test.trie_copy_ctor();
   test.trie_copy_assign();
   test.trie_get_mock();
   test.trie_end_iter();
   test.trie_find();
   test.trie_find_string_test();
   test.trie_find_parent();
}
