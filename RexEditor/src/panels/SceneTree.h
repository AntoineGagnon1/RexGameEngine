#pragma once

#include <string>

#include <RexEngine.h>

#include "Panel.h"
#include "PanelManager.h"
#include "Inspector.h"

namespace RexEditor
{
	class SceneTreePanel : public Panel
	{
	private:

		struct EntityNode
		{
			Entity entity;
			std::vector<std::shared_ptr<EntityNode>> children;

			EntityNode(Entity e)
				: entity(e)
			{}
		};

	public:
		SceneTreePanel() : Panel("Scene Tree")
		{
		}

	protected:
		virtual void OnGui([[maybe_unused]] float deltaTime) override
		{
			auto scene = RexEngine::Scene::CurrentScene();
			if (!scene)
			{
				UI::Text t("No active scene !");
				return;
			}

			// Make the node tree
			auto root = std::make_shared<EntityNode>(Entity());
			std::unordered_map<Guid, std::shared_ptr<EntityNode>> nodes;

			auto transforms = scene->GetComponents<TransformComponent>();
			// This will create each entity and its parent if needed,
			// if a node already exists it means that it was added as a parent,
			// the parent of this parent node node is then added if needed
			for (auto&& [entity, transform] : transforms)
			{
				auto guid = entity.GetGuid();
				if (nodes.contains(guid))
				{
					// Fix orphans
					if (transform.parent)
					{
						auto parentGuid = transform.parent.GetGuid();
						if(!nodes.contains(parentGuid)) // Create the parent if needed, it will be an orphan for now
							nodes.insert({ parentGuid, std::make_shared<EntityNode>(transform.parent) });
						
						nodes[parentGuid]->children.push_back(nodes[guid]); // Add itself
					}
					else
					{ // No parent, add to root
						root->children.push_back(nodes[guid]);
					}
				}
				else
				{
					std::shared_ptr<EntityNode> addTo = root;
					if (transform.parent)
					{
						auto parentGuid = transform.parent.GetGuid();
						if (!nodes.contains(parentGuid))
						{ // Create the parent, this parent will be an orphan for now
							nodes.insert({ parentGuid, std::make_shared<EntityNode>(transform.parent) });
						}

						addTo = nodes[parentGuid];
					}

					auto node = std::make_shared<EntityNode>(entity);
					addTo->children.push_back(node);
					nodes.insert({ guid, node });
				}
			}

			// open the context menu as root
			if (Window()->IsClicked(RexEngine::MouseButton::Right))
				m_popupSelected = root->entity;

			// Display the tree
			for(auto& node : root->children)
				DrawNode(node);

			// Render the context menu
			if (UI::ContextMenu p("SceneTreeContext"); p.IsOpen())
			{
				if (UI::Menu create("Create"); create.IsOpen())
				{
					Entity created = Entity();
					if (UI::MenuItem item("Empty"); item.IsClicked())
						created = scene->CreateEntity("New Entity");

					if (created)
					{
						// Select the new Entity
						m_selected = created;
						if(auto panel = PanelManager::GetPanel<InspectorPanel>(); panel != nullptr)
							panel->InspectEntity(created.GetGuid());
					}
						

					if (created && m_popupSelected)
					{ // Not for the root, change the parent
						created.Transform().parent = m_popupSelected;
					}
				}

				if (m_popupSelected)
				{
					if (UI::MenuItem item("Delete"); item.IsClicked())
						scene->DestroyEntity(m_popupSelected, true);
				}
			}
		}

	private:
		Entity m_selected;
		Entity m_popupSelected; // Entity used for the context menu

		// Recursive
		void DrawNode(std::shared_ptr<EntityNode> node)
		{
			UI::TreeNodeFlags flags = UI::TreeNodeFlags::OpenOnArrow | UI::TreeNodeFlags::DefaultOpen;
			flags |= node->children.empty() ? UI::TreeNodeFlags::Leaf : UI::TreeNodeFlags::None;
			flags |= node->entity == m_selected ? UI::TreeNodeFlags::Selected : UI::TreeNodeFlags::None;

			UI::TreeNode n(node->entity.Name() + "##NodeName", flags); // ##NodeName because the name might be empty
			if (n.IsClicked() || n.IsClicked(RexEngine::MouseButton::Right)) // Also process the click if the node is closed
			{
				m_selected = node->entity;
				// Tell the inspector
				if (auto panel = PanelManager::GetPanel<InspectorPanel>(); panel != nullptr)
					panel->InspectEntity(node->entity.GetGuid());
			}

			// Context menu
			if (n.IsClicked(RexEngine::MouseButton::Right))
				m_popupSelected = node->entity;


			if(n.IsOpen())
			{
				for (auto child : node->children)
					DrawNode(child);
			}
		}

	};
}