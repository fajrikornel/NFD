#include "common/logger.hpp"
#include "cs-policy-lfu.hpp"
#include "cs.hpp"

namespace nfd {
namespace cs {
namespace lfu {

const std::string LfuPolicy::POLICY_NAME = "lfu";
NFD_REGISTER_CS_POLICY(LfuPolicy);

NFD_LOG_INIT(LfuStructure);

Node::Node(Entry node_key, size_t node_count)
{
  this->key = node_key;
  this->count = node_count;
  this->prev = nullptr;
  this->next = nullptr;
}

LfuStructure::LfuStructure()
{
  std::shared_ptr<const Data> data1 = std::make_shared<const Data>();
  bool isUnsolicited1 = false;
  Entry entry1(data1,isUnsolicited1);
  std::shared_ptr<const Data> data2 = std::make_shared<const Data>();
  bool isUnsolicited2 = false;
  Entry entry2(data2,isUnsolicited2);
  this->head = new Node(entry1, MAX);
  this->tail = new Node(entry2, MIN);
  this->head->next = tail;
  this->tail->prev = head;
}

void
LfuStructure::add(Entry key)
{
  NFD_LOG_DEBUG("add function called for name: " + key.getName().toUri());
  std::map<Entry, Node *>::iterator it;
  it = this->map.find(key);
  if (it != this->map.end())
  {
    return;
  }

  // Add the node to the hashmap
  Node *newNode = new Node(key, 0);
  this->map[key] = newNode;

  // Move new node to proper position
  move(newNode);
  NFD_LOG_DEBUG("Internal lookup index:");
  std::vector<std::pair<Entry,size_t>> cacheList = getAll();
  for (int i = 0; i < cacheList.size(); i++){
    auto curEntry = cacheList[i].first;
    auto curCount = cacheList[i].second;
    NFD_LOG_DEBUG("Cache:" + curEntry.getName().toUri());
    NFD_LOG_DEBUG("Count:" + std::to_string(curCount));
  }
}

void
LfuStructure::get(Entry key)
{
  NFD_LOG_DEBUG("get function called for name: " + key.getName().toUri());
  std::map<Entry, Node *>::iterator it;
  it = this->map.find(key);
  if (it == this->map.end())
  {
    return;
  }

  // Procedure to add count and reposition the cache

  // Reposition the cache
  it->second->prev->next = it->second->next;
  it->second->next->prev = it->second->prev;

  // Add cache
  it->second->count++;

  // Call move function to reposition the cache
  move(it->second);

  NFD_LOG_DEBUG("Internal lookup index:");
  std::vector<std::pair<Entry,size_t>> cacheList = getAll();
  for (int i = 0; i < cacheList.size(); i++){
    auto curEntry = cacheList[i].first;
    auto curCount = cacheList[i].second;
    NFD_LOG_DEBUG("Cache:" + curEntry.getName().toUri());
    NFD_LOG_DEBUG("Count:" + std::to_string(curCount));
  }
  return;
}

bool
LfuStructure::del(Entry key)
{
  NFD_LOG_DEBUG("del function called for name: " + key.getName().toUri());
  std::map<Entry, Node *>::iterator it;
  it = this->map.find(key);
  if (it == this->map.end())
  {
    return false; //Cache not found
  }

  // Delete cache from structure 
  this->map.erase(it);
  it->second->prev->next = it->second->next;
  it->second->next->prev = it->second->prev;

  return true;
}

std::vector<Entry>
LfuStructure::enforceCapacity(size_t capacity)
{
  NFD_LOG_DEBUG("enforceCapacity function called");
  std::vector<Entry> removed_entries;
  while (this->map.size() > capacity)
  { // Erase last element before tail
    std::map<Entry, Node *>::iterator it = this->map.find(this->tail->prev->key);
    removed_entries.push_back(it->second->key);
    this->map.erase(it);
    this->tail->prev = tail->prev->prev;
    this->tail->prev->next = tail;
    NFD_LOG_DEBUG("will remove name: " + it->second->key.getName().toUri());
  }
  return removed_entries;
}

void
LfuStructure::move(Node *node)
{
  NFD_LOG_DEBUG("move function called");
  Node *curr = this->head;
  while (curr != nullptr)
  { // Iterate from head to tail (highest to lowest count)
    if (curr->count > node->count)
    {                    // If current iteration still has higher count than the node
      curr = curr->next; // Continue to next node
    }
    else
    { // If current iteration has lower count than the node

      // Place the node before the current iteration
      node->prev = curr->prev;
      node->next = curr;
      node->next->prev = node;
      node->prev->next = node;
      break;
    }
  }
}

std::vector<std::pair<Entry,size_t>>
LfuStructure::getAll()
{
  std::vector<std::pair<Entry,size_t>> res;

  Node *curr = this->head->next;

  while (curr->next != nullptr)
  {
    res.push_back(std::make_pair(curr->key,curr->count));
    curr = curr->next;
  }

  return res;
}

LfuPolicy::LfuPolicy()
  : Policy(POLICY_NAME)
{
  lfu_structure = new LfuStructure();
}

void
LfuPolicy::doAfterInsert(EntryRef i)
{
  this->evictEntries(); //Evict entries first, with capacity assumed to be CsLimit - 1 (remove least frequently accessed cache)
  this->lfu_structure->add(*i); //Add new cache
}

void
LfuPolicy::doAfterRefresh(EntryRef i)
{
  this->lfu_structure->add(*i);
}

void
LfuPolicy::doBeforeErase(EntryRef i)
{
  this->lfu_structure->del(*i);
}

void
LfuPolicy::doBeforeUse(EntryRef i)
{
  this->lfu_structure->get(*i);
}

void
LfuPolicy::evictEntries()
{
  std::vector<Entry> removed_entries = this->lfu_structure->enforceCapacity(this->getLimit()-1); //Capacity is limit-1 to remove last cache before adding new one
  for (auto it = removed_entries.begin(); it != removed_entries.end(); it++) {
    for (auto ref = this->getCs()->begin(); ref != this->getCs()->end(); ref++) {
      if (ref->getData() == it->getData() && ref->isUnsolicited() == it->isUnsolicited()) {
          this->emitSignal(beforeEvict, ref);
          break;
      }
    }
  }
}


} // namespace lfu
} // namespace cs
} // namespace nfd
