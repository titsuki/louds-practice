#define _USE_MATH_DEFINES
#define INF 0x3f3f3f3f

#include <iostream>
#include <cstdio>
#include <sstream>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <queue>
#include <stack>
#include <limits>
#include <map>
#include <string>
#include <cstring>
#include <set>
#include <deque>
#include <bitset>
#include <list>
#include <cctype>
#include <utility>
#include <assert.h>

using namespace std;
  
typedef long long ll;
typedef pair <int,int> P;
typedef pair <int,P> PP;
  
static const double EPS = 1e-8;
  
static const int tx[] = {0,1,0,-1};
static const int ty[] = {-1,0,1,0};

class BitVector {
private:
  vector<bool> bv;
public:
  int operator[](int pos) {
    return (bv[pos] == true ? 1 : 0);
  }

  void push(int elem) {
    if (elem == 1 || elem == 0) {
      bv.push_back((bool) elem);
    }
  }
  void push_n(int elem, int n) {
    if (elem == 1 || elem == 0) {
      for (int i = 0; i < n; i++) {
	bv.push_back((bool) elem);
      }
    }
  }

  int size() const {
    return bv.size();
  }
};

class SuccinctBitVector {
private:
  BitVector bv;
public:

  int operator[](int pos) {
    return bv[pos];
  }

  void push(int elem) {
    bv.push(elem);
  }

  void push_n(int elem, int n) {
    bv.push_n(elem, n);
  }

  int size() const {
    return bv.size();
  }
  
  int rank(int pos, int b) {
    int count = 0;
    for (int i = 0; i <= pos; i++) {
      if (bv[i] == b) {
	count++;
      }
    }
    return count;
  }

  int select(int count, int b) {
    if (count <= 0) {
      return -1;
    }
    for (int i = 0; i < bv.size(); i++) {
      if (bv[i] == b) {
	count--;
      }
      if (count == 0) {
	return i;
      }
    }
    return -1;
  }

  int rank0(int pos) {
    return rank(pos, 0);
  }
  
  int rank1(int pos) {
    return rank(pos, 1);
  }

  int select0(int pos) {
    return select(pos, 0);
  }
  
  int select1(int pos) {
    return select(pos, 1);
  }

  void debug() {
    for (int i = 0; i < bv.size(); i++){
      printf("%d", bv[i]);
    }
    printf("\n");
  }
};


class LOUDSTrie {
private:
  SuccinctBitVector lbs;
  SuccinctBitVector terminal;
  vector<char> edge;

  void build(vector<string>& words) {
    sort(words.begin(), words.end());
    words.erase(unique(words.begin(), words.end()), words.end());
    lbs.push(1);
    lbs.push(0);

    map<char,bool> dict;
    for (int i = 0; i < words.size(); i++) {
      dict[words[i][0]] = true;
    }
    lbs.push_n(1, dict.size());
    lbs.push(0);
    bfs(words);
  }

  vector<char> child_nodes(const vector<string>& words, int depth, int left, int right) {
    vector<char> children;
    for (int i = left; i < right; i++) {
      if (depth + 1 >= words[i].size()) {
	continue;
      }
      char character = words[i][depth + 1];
      if (children.size() > 0 && children[children.size() - 1] == character) {
	continue;
      }
      children.push_back(character);
    }
    return children;
  }

  class State {
  public:
    int depth;
    int left;
    int right;
    State(int depth, int left, int right) : depth(depth), left(left), right(right) {}
  };

  void bfs(const vector<string>& words) {
    queue<State> que;
    que.push(State(0, 0, words.size()));

    while(!que.empty()) {
      State s = que.front();
      que.pop();
      int begin_pos = s.left;
      int end_pos = s.right;
      int depth = s.depth;
      vector<int> node_starts;
      char prev = '$';
      int pos = begin_pos;
      while (pos < end_pos) {
	if (prev != words[pos][depth]) {
	  node_starts.push_back(pos);
	  prev = words[pos][depth];
	}
	pos++;
      }

      for (int i = 0; i < node_starts.size(); i++) {
	int left, right;
	
	left = node_starts[i];
	if (i + 1 < node_starts.size()) {
	  right = node_starts[i + 1];
	} else {
	  right = end_pos;
	}

	// build terminal
	if (words[left].size() <= depth + 1) {
	  terminal.push(1);
	} else {
	  terminal.push(0);
	}

	// build lbs
	vector<char> children = child_nodes(words, depth, left, right);
	lbs.push_n(1, children.size());
	lbs.push(0);
	edge.push_back(words[left][depth]);

	if (left < words.size() && depth + 1 < words[left].size()) {
	  que.push(State(depth + 1, left, right));
	} else if(right - 1 >= 0 && right - 1 < words.size() && depth + 1 < words[right - 1].size()) {
	  que.push(State(depth + 1, left + 1, right));
	}
      }
    }
  }
  
public:
  LOUDSTrie(vector<string>& words) {
    build(words);
  }
  LOUDSTrie(SuccinctBitVector lbs,
	    SuccinctBitVector terminal,
	    vector<char> edge) {
    this->lbs = lbs;
    this->terminal = terminal;
    this->edge = edge;
  }

  vector<int> common_prefix_search(const string& str) {
    vector<int> result;
    int n = 0;
    for(int i = 0; i < str.size(); i++) {
      char c = str[i];
      n = traverse(n, c);
      if (n == -1) {
	break;
      }

      int m = lbs.rank1(n) - 2;
      if (terminal[m] != 0) {
	result.push_back(m);
      }
    }
    return result;
  }
  
  int parent(int x) {
    return lbs.select1(lbs.rank0(x));
  }
  
  int first_child(int x) {
    int y = lbs.select0(lbs.rank1(x)) + 1;
    if (lbs[y] == 0) {
      return -1;
    }
    else {
      return y;
    }
  }

  int traverse(int pos, char c) {
    int child_pos = first_child(pos);
    if (child_pos == -1) {
      return -1;
    }

    int m = lbs.rank1(child_pos) - 2;
    while (child_pos < lbs.size() && lbs[child_pos] == 1 && m < edge.size()) {
      if (edge[m] == c) {
	return child_pos;
      }
      child_pos++;
      m++;
    }
    return -1;
  }

  string id2word(int id) {
    string result = "";
    int pos = lbs.select1(id + 2);

    do {
      int m = lbs.rank1(pos) - 2;
      result = edge[m] + result;
      pos = parent(pos);
    } while(pos != 0);
    return result;
  }
};

void test1() {
  SuccinctBitVector sbv;

  // test push
  sbv.push(1);
  sbv.push(0);
  sbv.push(1);
  sbv.push(0);
  sbv.push(0);

  assert(sbv[0] == 1);
  assert(sbv[1] == 0);
  assert(sbv[2] == 1);
  assert(sbv[3] == 0);
  assert(sbv[4] == 0);

  // test rank select
  assert(sbv.rank1(3) == 2);
  assert(sbv.rank0(3) == 2);
  assert(sbv.select1(3) == -1);
  assert(sbv.select0(3) == 4);

  SuccinctBitVector terminal;
  terminal.push(1);

  vector<char> edge;
  edge.push_back('a');

  LOUDSTrie ltrie(sbv, terminal, edge);

  // test first_child
  assert(ltrie.first_child(0) == 2);
  assert(ltrie.first_child(1) == 2);
  assert(ltrie.first_child(2) == -1);

  // test common_prefix_serch
  vector<int> nodes = ltrie.common_prefix_search("a");
  assert(nodes.size() == 1);
  assert(nodes[0] == 0);
  assert(ltrie.id2word(nodes[0]) == "a");
}

void test2() {
  vector<string> words;
  words.push_back("perl6");
  LOUDSTrie ltrie(words);
  vector<int> nodes = ltrie.common_prefix_search("perl6");
  assert(nodes.size() == 1);
  assert(nodes[0] == 4);
  assert(ltrie.id2word(nodes[0]) == "perl6");
}

void test3() {
  vector<string> words;
  words.push_back("perl6");
  LOUDSTrie ltrie(words);
  vector<int> nodes = ltrie.common_prefix_search("perl6 is fun");
  assert(nodes.size() == 1);
  assert(nodes[0] == 4);
  assert(ltrie.id2word(nodes[0]) == "perl6");
}

void test4() {
  vector<string> words;
  words.push_back("perl6");
  words.push_back("is");
  LOUDSTrie ltrie(words);
  vector<int> nodes1 = ltrie.common_prefix_search("is");
  assert(nodes1.size() == 1);
  assert(nodes1[0] == 2);
  assert(ltrie.id2word(nodes1[0]) == "is");

  vector<int> nodes2 = ltrie.common_prefix_search("perl6");
  assert(nodes2.size() == 1);
  assert(nodes2[0] == 6);
  assert(ltrie.id2word(nodes2[0]) == "perl6");
}

void test5() {
  vector<string> words;
  words.push_back("perl6");
  words.push_back("perl");
  LOUDSTrie ltrie(words);
  vector<int> nodes1 = ltrie.common_prefix_search("perl6");
  assert(nodes1.size() == 2);
  assert(nodes1[0] == 3);
  assert(nodes1[1] == 4);
  assert(ltrie.id2word(nodes1[0]) == "perl");
  assert(ltrie.id2word(nodes1[1]) == "perl6");
}

int main() {
  test1();
  test2();
  test3();
  test4();
  test5();
}
