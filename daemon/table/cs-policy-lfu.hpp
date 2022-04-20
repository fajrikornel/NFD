#ifndef NFD_DAEMON_TABLE_CS_POLICY_LFU_HPP
#define NFD_DAEMON_TABLE_CS_POLICY_LFU_HPP

#include "cs-policy.hpp"

#include <map>
#include <limits>
#include <vector>
#include <utility>

namespace nfd {
namespace cs {
namespace lfu {

class Node 
{
public:
  Entry key = *(new Entry(std::make_shared<const Data>(),false));
  size_t count;
  Node* prev;
  Node* next;

public:
  Node(Entry node_key, size_t node_count);
};

class LfuStructure
{
private:
  std::map<Entry, Node *> map;
  Node *head;
  Node *tail;
  size_t MAX = std::numeric_limits<size_t>::max();
  size_t MIN = std::numeric_limits<size_t>::min();

public:
  LfuStructure();

public:
  void
  add(Entry key);

  void
  get(Entry key);

  bool
  del(Entry key);

  std::vector<Entry>
  enforceCapacity(size_t capacity);

private:
  void
  move(Node *node);

public:
  std::vector<std::pair<Entry,size_t>>
  getAll();
};


/** \brief Least-Frequently-Used (LFU) replacement policy
 */
class LfuPolicy final : public Policy
{
public:
  LfuPolicy();

public:
  static const std::string POLICY_NAME;

private:
  void
  doAfterInsert(EntryRef i) final;

  void
  doAfterRefresh(EntryRef i) final;

  void
  doBeforeErase(EntryRef i) final;

  void
  doBeforeUse(EntryRef i) final;

  void
  evictEntries() final;

private:
  LfuStructure* lfu_structure;
};



} // namespace lfu

using lfu::LfuPolicy;

} // namespace cs
} // namespace nfd

#endif // NFD_DAEMON_TABLE_CS_POLICY_LFU_HPP