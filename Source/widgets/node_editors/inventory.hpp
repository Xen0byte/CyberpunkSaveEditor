#pragma once
#include "node_editor.hpp"
#include "cserialization/cpnames.hpp"
#include "cserialization/cnodes/inventory.hpp"

// to be used with itemData struct
struct itemData_widget
{
  static inline void draw(itemData& item, bool* p_remove = nullptr)
  {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    bool treenode = ImGui::TreeNodeBehavior(window->GetID("itemData"), 0, item.name().c_str(), NULL);

    if (p_remove)
    {
      ImGui::SameLine();
      *p_remove = ImGui::Button("-", ImVec2(50, 0));
    }

    if (treenode)
    {
      auto e = node_editors_mgr::get().get_editor(item.raw);
      e->draw_widget();
      ImGui::TreePop();
    }
  }
};

// to be used with itemData node
class itemData_editor
  : public node_editor
{
  itemData item;

public:
  itemData_editor(const std::shared_ptr<const node_t>& node)
    : node_editor(node)
  {
    reload();
    // todo, listen to child nodes..
  }

  ~itemData_editor()
  {
  }

public:
  bool commit_impl() override
  {
    auto rebuilt = item.to_node();
    auto curnode = ncnode();

    if (!rebuilt)
      return false;

    curnode->assign_children(rebuilt->children());
    curnode->assign_data(rebuilt->data());
  }

  bool reload_impl() override
  {
    bool success = item.from_node(node());
    return success;
  }

protected:
  void draw_impl(const ImVec2& size) override
  {
    itemData_widget::draw(item);
  }
};

class inventory_editor
  : public node_editor
{
  // attrs
  inventory inv;

public:
  inventory_editor(const std::shared_ptr<const node_t>& node)
    : node_editor(node)
  {
    reload();
    // todo, listen to child nodes..
  }

  ~inventory_editor()
  {
  }

public:
  bool commit_impl() override
  {
    auto rebuilt = inv.to_node();
    auto curnode = ncnode();

    if (!rebuilt)
      return false;

    curnode->assign_children(rebuilt->children());
    curnode->assign_data(rebuilt->data());
  }

  bool reload_impl() override
  {
    bool success = inv.from_node(node());
    return success;
  }

protected:
  void draw_impl(const ImVec2& size) override
  {
    auto& cpn = cpnames::get();

    ImGui::BeginChild("##inventory_editor",
      ImVec2(size.x > 0 ? size.x : 800, size.y > 0 ? size.y : 600));

    

    for (size_t j = 0; j < inv.m_subinvs.size(); ++j)
    {
      auto& subinv = inv.m_subinvs[j];
      ImGui::PushID((int)j);
      std::stringstream ss;
      ss << "inventory_" << std::hex << std::uppercase << std::setfill('0') << std::setw(16) << subinv.uid;
      if (ImGui::CollapsingHeader(ss.str().c_str()))
      {
        if (ImGui::Button("dupe first item row") && subinv.items.size() > 0)
        {
          // todo: move that on the data side
          auto& first_item = subinv.items.front();
          auto new_node = first_item.to_node()->deepcopy();
          itemData item_data;
          item_data.from_node(new_node);
          subinv.items.insert(subinv.items.begin(), 1, item_data);
        }

        int torem_item = -1;
        for (size_t i = 0; i < subinv.items.size(); ++i)
        {
          ImGui::PushID((int)i);

          bool rem = false;
          auto& item_data = subinv.items[i];
          itemData_widget::draw(item_data, &rem);
          if (rem)
            torem_item = (int)i;

          ImGui::PopID();
        }
        if (torem_item >= 0)
          subinv.items.erase(subinv.items.begin() + torem_item);
      }
      ImGui::PopID();
    }
    ImGui::EndChild();
  }
};
