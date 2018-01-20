#include <iostream>
#include <cassert>
#include <functional>
#include <regex>
#include <vector>
#include <string>
#include <iterator>
#include "trie.h"

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

   function<vector<string>(const string&)> get_string_split_func() {
      return [](const string &val) -> vector<string> {
         vector<string> v;
         regex period("\\.");
         regex_token_iterator<string::const_iterator> tokit(
            val.begin(), val.end(), period, -1);
         regex_token_iterator<string::const_iterator> tokend;
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

      Trie<string>::iterator it;
      it = t.find("foo");
      assert(*it == "foo");
 
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
      assert(it == t.end());

      it = t.find(410);
      assert(t.find_parent(it) == t.end());

      t.insert(41);
      it = t.find_parent(it);
      assert(it != t.end());
      assert(*it == 41);
      assert(t.find_parent(it) == t.end());

      t.insert(4);
      it = t.find_parent(it);
      assert(it != t.end());
      assert(*it == 4);
      assert(t.find_parent(it) == t.end());
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
