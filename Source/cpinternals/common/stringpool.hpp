#pragma once
#include <inttypes.h>
#include <optional>
#include <vector>
#include <array>
#include <unordered_map>
#include "iarchive.hpp"

namespace cp {

struct stringpool
{
  constexpr static size_t block_size = 0x10000;
  using block_type = std::array<char, block_size>;

  stringpool()
    : m_curpos(block_size)
  {
  }

  bool has_string(std::string_view s) const
  {
    return m_viewsmap.find(s) != m_viewsmap.end();
  }

  size_t size() const
  {
    return m_views.size();
  }

  size_t register_string(std::string_view s)
  {
    auto it = m_viewsmap.find(s);
    if (it != m_viewsmap.end())
    {
      return it->second;
    }

    const size_t ssize = s.size();
    auto new_view = allocate_view(ssize);
    std::copy(s.begin(), s.end(), new_view.begin());

    const size_t idx = m_views.size();
    m_views.emplace_back(new_view);
    m_viewsmap.emplace(s, idx);

    return idx;
  }

  std::optional<size_t> find(std::string_view s) const
  {
    auto it = m_viewsmap.find(s);
    if (it != m_viewsmap.end())
    {
      return std::make_optional<size_t>(it->second);
    }
    return std::nullopt;
  }

  const std::string_view at(uint32_t idx) const
  {
    return m_views[idx];
  }

  // These serialization methods are used by the CSAV's System nodes
  bool serialize_in(iarchive& reader, uint32_t descs_size, uint32_t pool_size, uint32_t descs_offset = 0);
  bool serialize_out(iarchive& writer, uint32_t& descs_size, uint32_t& pool_size);

protected:
  bool allocate_block()
  {
    m_blocks.emplace_back(std::make_unique<block_type>());
    m_curpos = 0;
  }

  std::string_view allocate_view(size_t len)
  {
    if (m_curpos + len > block_size)
      allocate_block();

    const char* data = m_blocks.back()->data() + m_curpos;
    m_curpos += len;

    return std::string_view(data, len);
  }

  std::vector<std::unique_ptr<block_type>> m_blocks;
  size_t m_curpos;

  std::vector<std::string_view> m_views;

  // accelerated search

  std::unordered_map<std::string_view, size_t> m_viewsmap;
};

} // namespace cp

