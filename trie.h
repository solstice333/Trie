#include <functional>
#include <map>
#include <vector>
#include <algorithm>
#include <sstream>
#include <queue>
#include <string>
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
         return *this;
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
   class iterator: public std::iterator<bidirectional_iterator_tag, Node> {
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
      bool _is_leaf() { return _val->end(); }

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
      return *this;
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
      return itr._is_root() || !itr._is_leaf() ? end() : itr;
   }

   iterator end() { return iterator(); }
   string str() { return _str(_root); }
   ~Trie() { delete _root; }
};
