
#include <vector>
#include <string>
#include <iostream>
#include <unordered_map>
#include <utility>
#include <list>
#include <unordered_set>
#include <memory>

#include "test/substrings_input02.h"
#include "test/substrings_input04.h"
#include "test/substrings_input06.h"

#include "test/testing_utils.h"

#ifdef DEBUG_LOG_CXX
#define DEBUG_MSG(str) do { str ; } while( false )
#else
#define DEBUG_MSG(str) do { } while ( false )
#endif


#ifdef TRACE_LOG_CXX
#define TRACE_MSG(str) do { str ; } while( false )
#else
#define TRACE_MSG(str) do { } while ( false )
#endif

using std::string;

using suffix_pos_t=size_t;

static constexpr char TERM_CHAR='$';
static constexpr char NULL_CHAR='\0';
// END_MARKER must be > max size of string to be ingested
static constexpr suffix_pos_t END_MARKER=1000001;

class TrieNode;

struct TrieEdge;

class TrieContainer;

struct Suffix {
  suffix_pos_t from;
  suffix_pos_t to;
  Suffix(suffix_pos_t _from, suffix_pos_t _to):
    from{_from}, to{_to} {}

  suffix_pos_t getLength() {
    return to - from;
  }

  suffix_pos_t getComparativeLength(suffix_pos_t end) {
    return end - from;
  }

};

struct TrieEdge {

  Suffix suffix;
  std::shared_ptr<TrieNode> trieNode;
  int weight;

  public:

  TrieEdge() = delete;

  TrieEdge(Suffix _suffix, std::shared_ptr<TrieNode> _trieNode):
    suffix{_suffix}, trieNode{_trieNode}, weight{1} {}

  void printDetails(const std::string &trieString);

  int getWeight() {
    return weight;
  }

  std::shared_ptr<TrieNode> getTerminalNode() {
    return trieNode;
  }

  void setTerminalNode(std::shared_ptr<TrieNode> _trieNode) {
    trieNode = _trieNode;
  }

  struct EdgeSplitContext {
    suffix_pos_t len;
    suffix_pos_t begin;
    suffix_pos_t end;
  };

  static std::shared_ptr<TrieEdge> trieEdgeFactory(TrieContainer *container, const EdgeSplitContext &context) {
    auto edge = std::make_shared<TrieEdge>(
          Suffix{context.begin, END_MARKER},
          std::make_shared<TrieNode>(container, context.len)
        );
    return edge;
  }

  static std::shared_ptr<TrieEdge> trieEdgeSplitFactory(const EdgeSplitContext &context, std::shared_ptr<TrieNode> _node) {
    return std::make_shared<TrieEdge>(
          Suffix{
            context.begin, 
            context.end
          },
          _node
        );
  }

};


class TrieNode {

  friend class TrieContainer;

  private:
  int node_id;
  TrieContainer * container;
  
  suffix_pos_t length;
  size_t branchTotal;
  std::unordered_map<char, std::shared_ptr<TrieEdge>> suffixMap;
  std::shared_ptr<TrieNode> suffix_link;

  static int node_count;

  public:

  static int getNodeCount() {
    return TrieNode::node_count;
  }

  bool isRoot() {
    return node_id == 0;
  }

  TrieNode() = delete;

  void printDetails() {
    DEBUG_MSG(
      std::cout << "Node keys = ";
      auto key = suffixMap.cbegin(); 
      while (key != suffixMap.cend()) {
        std::cout << key->first << ", ";
        ++key;
      } 
      std::cout << "\n\n";
    );
  }

  std::shared_ptr<TrieNode> getSuffixLink() {
    return suffix_link;
  }

  void setSuffixLink(std::shared_ptr<TrieNode> _suffix_link) {
    suffix_link = _suffix_link;
  }

  struct NodeSplitContext {
    std::shared_ptr<TrieNode> existingNode;
    char existingKey; 
    char splitKey; 
    suffix_pos_t splitLength;
    suffix_pos_t splitBegin;
    suffix_pos_t splitEnd;
  };

  TrieNode(TrieContainer *_container, suffix_pos_t _length);

  bool containsKey(char key) {
    return suffixMap.find(key) != suffixMap.end();
  }

  std::shared_ptr<TrieEdge> getEntry(char key) {
    TRACE_MSG( 
      std::cout << "getEntry() for node id = `" << node_id << "`, and char `" << key << "`\n"; 
      std::cout << "node contains " << suffixMap.size() << " keys\n\n\n";
    );
    return suffixMap[key];
  }

  void insertEntry(char key, std::shared_ptr<TrieEdge> value) {
    suffixMap[key] = value;
  }

  int getLength() {
    return length;
  }

  void setLength(suffix_pos_t _length) {
    length = _length;
  }

  size_t getBranchCount();

  int getScore() {
    return length * branchTotal;
  }

  int getId() {
    return node_id;
  }
  
};

void TrieEdge::printDetails(const std::string &trieString) {
  TRACE_MSG(
    std::cout << "######\n\n\n";
    std::cout << "Edge is from position " << suffix.from << ", to " << suffix.to 
    << ", comprising substring `" << trieString.substr(suffix.from, suffix.to - suffix.from) << "`, "
    << ", terminating at Node:\n\t";
    trieNode->printDetails();
    std::cout << "######\n\n\n"; 
  );
}

struct ActivePoint {
  std::shared_ptr<TrieNode> active_node;
  char active_edge;
  suffix_pos_t active_length;

  ActivePoint() = delete;

  explicit ActivePoint(std::shared_ptr<TrieNode> nodePtr): 
    active_node{nodePtr}, active_edge{NULL_CHAR}, active_length{0}
    {}

  bool hasNoEdge() {
    return active_edge == NULL_CHAR;
  }

  void setNoEdge() {
    active_edge = NULL_CHAR;
    active_length = 0;
  }

  suffix_pos_t getNewLength() {
    return active_node->getLength() + active_length;
  }

  void incrementActiveLength() {
    ++active_length;
  }

  suffix_pos_t getActiveLength() {
    return active_length;
  }

  void setActiveLength(suffix_pos_t _activeLength) {
    active_length = _activeLength;
  }

  char getActiveEdge() {
    return active_edge;
  }

  void setActiveEdge(char _c) {
    active_edge = _c;
  }

};




class TrieContainer {
  friend class TrieNode;
  int node_count;
  std::shared_ptr<TrieNode> root;
  const std::string trieString;
  ActivePoint activePoint;
  size_t remainder;

  

  suffix_pos_t charCount;
  suffix_pos_t charsProcessed;

  suffix_pos_t maxScore;

  size_t insertRecursionDepth;
  size_t normaliseRecursionDepth;

  std::vector<std::shared_ptr<TrieNode>> nodeStore;

  void updateMaxScore(std::shared_ptr<TrieNode> node) {
    auto newScore = node->getScore();
    if (newScore > maxScore)
      maxScore = newScore;
  }

  void updateMaxScore(TrieNode *node) {
    auto newScore = node->getScore();
    if (newScore > maxScore)
      maxScore = newScore;
  }

  public:

  suffix_pos_t getMaxScore() {
    return maxScore;
  }

  int getNodeCount() {
    return node_count;
  }

  explicit TrieContainer(std::string _trieString):
    node_count{0},
    root{std::make_shared<TrieNode>(this, 0)},
    trieString{_trieString},
    activePoint{root},
    remainder{0}, 
    charCount{0}, 
    charsProcessed{0}, 
    maxScore{trieString.size()}, 
    insertRecursionDepth{0}, normaliseRecursionDepth{0},
    nodeStore() {
    }

  char getSplitChar() {
    auto oldLength = activePoint.getActiveLength();
    auto edgeLength = activePoint.active_node->getEntry(activePoint.active_edge)->suffix.getLength();
    auto splitChar = getTrieStringChar(charCount - (oldLength - edgeLength));
    return splitChar;
  }

  char getSplitEdgeChar() {
    return getTrieStringChar(charCount - remainder);
  }

  suffix_pos_t getIndexAtNode(suffix_pos_t displacement) {
    return charCount - remainder + displacement;
  }

  char getSplitEdgeCharAtNode(suffix_pos_t edgeLength) {
    return getTrieStringChar(getIndexAtNode(edgeLength));
  }

  char getTrieStringChar(suffix_pos_t charCount) {
    return charCount < trieString.length() ? trieString[charCount] : TERM_CHAR;
  }

  void normalisePosition() {

    ++normaliseRecursionDepth;

    if (activePoint.hasNoEdge()) {
      --normaliseRecursionDepth;
      return;
    }
    if (activePoint.getActiveLength() == 0) {
      activePoint.setNoEdge();
      --normaliseRecursionDepth;
      return;
    }
    
    auto activeEdge = activePoint.active_node->getEntry(activePoint.getActiveEdge());
    
    if (activeEdge == nullptr) {
      --normaliseRecursionDepth;
      return;
    }

    auto stringLen = charCount + 1;
    auto activeEdgeLen = activeEdge->suffix.to == END_MARKER ? stringLen - activeEdge->suffix.from : activeEdge->suffix.to - activeEdge->suffix.from;
    auto edgeSteps = activePoint.getActiveLength();
    
    if (edgeSteps < activeEdgeLen) {
      --normaliseRecursionDepth;
      return;
    }

    auto newActiveIdx{stringLen - edgeSteps + activeEdgeLen - 1}; 
    auto newActiveChar = getTrieStringChar(newActiveIdx);
    activePoint.active_node = activeEdge->getTerminalNode();
    activePoint.setActiveEdge(newActiveChar);
    activePoint.setActiveLength(edgeSteps - activeEdgeLen);
    normalisePosition();
    --normaliseRecursionDepth;
  }

  void advanceActivePoint() {
    
    if (activePoint.active_node->getSuffixLink() != nullptr) {
      activePoint.active_node = activePoint.active_node->getSuffixLink();
      return; 
    }
    activePoint.active_node = root;
    if (activePoint.hasNoEdge()) {
      if (remainder == 1) {
        // null active edge
        activePoint.setNoEdge();
        return;
      }
      else {
        activePoint.setActiveEdge(getTrieStringChar(charCount - remainder + 2));
        activePoint.setActiveLength(remainder - 2);
        return;
      }
    }
    else {
      activePoint.setActiveEdge(getTrieStringChar(charCount - remainder + 2));
      activePoint.setActiveLength(remainder - 2);
      return;
    }
    
  }

  std::shared_ptr<TrieNode> splitNodeOut(const char c, suffix_pos_t &i) {

    // single-threaded fundamentally

    // for debug purposes, these could be generated at time of use really
    auto currentEdge = activePoint.active_node->getEntry(activePoint.active_edge);

    // struggling...
    auto currentStringIndex = currentEdge->suffix.from + activePoint.getActiveLength();
    // The character that the current active point represents
    auto activePointChar = getTrieStringChar(currentStringIndex);


    // node which will form the split point
    auto newNode1 = std::make_shared<TrieNode>(this, activePoint.active_node->getLength() + activePoint.active_length);
    // node at terminus of new edge 
    auto newNode2 = std::make_shared<TrieNode>(this, activePoint.active_node->getLength() + charCount);


    // new Edge
    TrieEdge::EdgeSplitContext splitContext1{
      .len = charCount,
      .begin = i,
      .end = END_MARKER
    };
    auto newEdge1 = TrieEdge::trieEdgeSplitFactory(splitContext1, newNode2);
    newNode1->insertEntry(c, newEdge1);
    
     
    // edge from split point to existing node
    TrieEdge::EdgeSplitContext splitContext2{
      .len = charCount,
      .begin = currentStringIndex,
      .end = currentEdge->suffix.to
    };
    auto newEdge2 = TrieEdge::trieEdgeSplitFactory(splitContext2, currentEdge->getTerminalNode());
    newNode1->insertEntry(activePointChar, newEdge2);

    TrieEdge::EdgeSplitContext splitContext3{
      .len = charCount,
      .begin = currentEdge->suffix.from,
      .end = currentStringIndex
    };
    auto newEdge3 = TrieEdge::trieEdgeSplitFactory(splitContext3, newNode1);

    activePoint.active_node->insertEntry(activePoint.active_edge, newEdge3);
    auto newEdge = activePoint.active_node->getEntry(activePoint.active_edge);

    if (nodeStore.size()) {
      nodeStore.back()->setSuffixLink(newNode1);
      nodeStore.clear();
    }


    // debug help
    TRACE_MSG(
      currentEdge->printDetails(trieString);
      currentEdge->getTerminalNode()->printDetails();
    );
    
    // where to go next?
    return newNode1;
  }

  void addNewEdge(const char c, suffix_pos_t &i) {
    auto ctx = TrieEdge::EdgeSplitContext{ .len = charCount, .begin = i };
    auto trieEdge = TrieEdge::trieEdgeFactory(this, ctx);
    activePoint.active_node->insertEntry(
      c, 
      trieEdge
    );
  }

  bool addCharNew(char c) {

    ++insertRecursionDepth;

    normalisePosition();
    auto i = charCount;
    
    bool isFinalChar = charCount == trieString.size();
    if (isFinalChar) {
      c = TERM_CHAR;
    }
    else if (charCount > trieString.size()) {
      --insertRecursionDepth;
      return false;
    }
    
    // in the case where there is no active edge, must either find one or insert one
    if (activePoint.hasNoEdge()) {
      // in the case where an edge exists
      if (activePoint.active_node->containsKey(c)) {
        activePoint.setActiveEdge(c);
        activePoint.setActiveLength(1);
        ++remainder;
        nodeStore.clear();
        ++charsProcessed;
        normalisePosition();
      }
      // otheriwse, if the edge does not exist...
      else {
        addNewEdge(c, i);
        if (remainder == 1) {
          
          ++charsProcessed;
          nodeStore.clear();
          --insertRecursionDepth;
          return !(charCount == trieString.size());
        }
        else {
          
          advanceActivePoint();
          --remainder;
          normalisePosition();
          addCharNew(c);
        }
      }
      --insertRecursionDepth;
    }

    // where an active edge DOES exist, must either continue along it or split it
    else {
      auto currentEdge = activePoint.active_node->getEntry(activePoint.active_edge);
      auto putativeIdx = currentEdge->suffix.from + activePoint.getActiveLength();
      auto expectedNextChar = getTrieStringChar(putativeIdx);
      if (c == expectedNextChar) {
        activePoint.incrementActiveLength();
        ++remainder;
        ++charsProcessed;
        nodeStore.clear();
        normalisePosition();
      }
      else {
        // add new leaf suffix for new char
        auto newNode = splitNodeOut(c, i);
          
        if (activePoint.active_node->isRoot()) {
          auto activeEdgeToBe = getTrieStringChar(i - remainder + 2);
          activePoint.setActiveEdge(activeEdgeToBe);
          activePoint.setActiveLength(remainder - 2);
        }
        else {
          advanceActivePoint();
        }
        --remainder;
        normalisePosition();
        addCharNew(c);
      }
    }
    return !(charCount == trieString.size());
  }

  void completeStep(suffix_pos_t begin_idx) {
    nodeStore.clear();
    char c = getTrieStringChar(0);
    remainder = 1;
    
    while(addCharNew(c)) {
      // do something
      c = getTrieStringChar(++charCount);
    }
    root->getBranchCount();
  }

  bool isParsed() {
    return charCount >= trieString.size();
  }

  void parseString() {
    while (!isParsed()) {
      completeStep(charCount);
    }
    DEBUG_MSG(std::cout << "\n\n Run completed, Total Nodes created = " << getNodeCount() << "\n\n";);
  }

};

TrieNode::TrieNode(TrieContainer *_container, suffix_pos_t _length):
    node_id{_container->node_count++},
    container{_container},
    length{_length},
    branchTotal{0},
    suffixMap{},
    suffix_link{nullptr} {} 


size_t TrieNode::getBranchCount() {
  TRACE_MSG(std::cout << "\tCalled getBranchCount() on node id= " << node_id << ", with length = " << length << '\n');
  int count = 0;
  auto keyCount = suffixMap.size();
  if (keyCount == 0)
    return 1;
  for (auto &key: suffixMap) {
    count = count + key.second->getTerminalNode()->getBranchCount();
  }
  
  branchTotal = keyCount > 0 ? count : 1;
  if (branchTotal > 1)
    container->updateMaxScore(this);
  return branchTotal;
}

std::pair<suffix_pos_t, int> maxValue(string t) {
  TrieContainer trie(t);
  trie.parseString();
  return {trie.getMaxScore(), trie.getNodeCount()};
}

void printTestResult(string name, int expected, int received) {
  DEBUG_MSG(
    std::cout << name << " : Expected = " << expected << ", received = " << received << '\n';
  );
}

int main() {

  auto t2expected = getTest02Result();
  auto t4expected = getTest04Result();
  auto t6expected = getTest06Result();


  testWrapperBasic(
    []() -> void {  
      std::cout << "\n\nTesting tkK1, size = 9\n\n\n";
      auto outcome = maxValue("aacbbabaaaabbbbcaca"); 
      std::cout << "\n\nTesting tkk1, outcome = " << outcome.first << ", expecting 12\n\n\n";
      assert(outcome.first == 19); 
      assert(outcome.second == 32);
    }
  );

 testWrapperBasic(
    []() -> void {  
      std::cout << "\n\nTesting tWeird, size = 6\n\n\n";
      auto outcome = maxValue("aacbbabbabbab");
      std::cout << "\n\nTesting tWeird, outcome = " << outcome.first << ", expecting 14\n\n\n";
      assert(outcome.first == 14); 
      assert(outcome.second == 23);
    }
  );

 testWrapperBasic(
    []() -> void {  
      std::cout << "\n\nTesting tSmallWeird, size = 6\n\n\n";
      auto outcome = maxValue("aacbbabbab");
      std::cout << "\n\nTesting tSmallWeird, outcome = " << outcome.first << ", expecting > 12\n\n\n";
      assert(outcome.first == 10); 
      assert(outcome.second == 17);
    }
  );

testWrapperBasic(
    []() -> void {  
      std::cout << "\n\nTesting t0, size = 6\n\n\n";
      auto outcome = maxValue("aaaaaa");
      std::cout << "\n\nTesting t0, outcome = " << outcome.first << ", expecting 12\n\n\n";
      assert(outcome.first == 12); 
      assert(outcome.second == 13);
    }
  );

  testWrapperBasic(
    []() -> void {  
      std::cout << "\n\nTesting tk0, size = 6\n\n\n";
      auto outcome = maxValue("ababab"); 
      std::cout << "\n\nTesting tk0, outcome = " << outcome.first << ", expecting 8\n\n\n";
      assert(outcome.first == 8); 
      assert(outcome.second == 12);
    }
  );

  testWrapperBasic(
    []() -> void {  
      std::cout << "\n\nTesting t1, size = 9\n\n\n";
      auto outcome = maxValue("abcabcddd"); 
      std::cout << "\n\nTesting t1, outcome = " << outcome.first << ", expecting 9\n\n\n";
      assert(outcome.first == 9); 
      assert(outcome.second == 16);
    }
  );

  testWrapperBasic(
    []() -> void {  
      std::cout << "\n\nTesting tk1, size = 9\n\n\n";
      auto outcome = maxValue("abcabcabc"); 
      std::cout << "\n\nTesting tk1, outcome = " << outcome.first << ", expecting 12\n\n\n";
      assert(outcome.first == 12); 
      assert(outcome.second == 17);
    }
  );

  testWrapperBasic(
    [t2expected]() -> void {  
      
      auto t2input = getTest02();
      std::cout << "\n\nTesting t2, size = " << t2input.size() << "\n\n\n";
      auto outcome = maxValue(t2input); 
      std::cout << "\n\nTesting t2, outcome = " << outcome.first << ", expecting " << t2expected << " \n\n\n";
      assert(outcome.first == t2expected); 
    }
  );

  testWrapperBasic(
    [t4expected]() -> void {  
      std::cout << "\n\nTesting t4, size = " << getTest04().size() << "\n\n\n";
      auto outcome = maxValue(getTest04()); 
      std::cout << "\n\nTesting t1, outcome = " << outcome.first << ", expecting " << t4expected << " \n\n\n";
      assert(outcome.first == t4expected); 
    }
  );

  testWrapperBasic(
    [t6expected]() -> void {  
      std::cout << "\n\nTesting t6\n\n\n";
      auto t6input = getTest06();
      
      TrieContainer trie(t6input);
      try {
        
        try {
          trie.parseString();
          std::pair<size_t, suffix_pos_t> outcome = {trie.getMaxScore(), trie.getNodeCount()};
          std::cout << "\n\nTesting t1, outcome = " << outcome.first << ", expecting  " << t6expected << "\n\n\n";
          assert(outcome.first == t6expected); 
        } catch (const std::exception& e) {
          std::cerr << e.what() << '\n';
        }
      } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
      }
    }
  );

}

