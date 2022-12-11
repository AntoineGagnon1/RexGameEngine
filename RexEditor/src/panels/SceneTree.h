#pragma once

#include <string>

#include <RexEngine.h>

#include "Panel.h"
#include "Inspector.h"
#include "../inspectors/EntityInspector.h"

namespace RexEditor
{
	class SceneTreePanel : public Panel
	{
	private:

		struct EntityNode
		{
			Entity entity;
			std::string tag;
			std::vector<std::shared_ptr<EntityNode>> children;

			EntityNode(const std::string& tag, Entity e)
				: tag(tag), entity(e)
			{}
		};

	public:
		SceneTreePanel() : Panel("Scene Tree")
		{
		}

	protected:
		virtual void OnGui(float deltaTime) override
		{
			auto scene = RexEngine::SceneManager::CurrentScene();
			if (!scene.IsValid())
			{
				UI::Text t("No scene loaded !");
				return;
			}

			// Make the node tree
			auto root = std::make_shared<EntityNode>("Active scene", Entity());
			std::unordered_map<std::string, std::shared_ptr<EntityNode>> nodes;

			auto transforms = scene.GetComponents<TransformComponent>();
			for (auto&& [entity, transform] : transforms)
			{
				auto tag = entity.Name();
				if (nodes.contains(tag))
				{
					// Fix orphans
					if (transform.parent)
					{
						auto parentTag = transform.parent.Name();
						if(!nodes.contains(parentTag)) // Create the parent if needed, it will be an orphan for now
							nodes.insert({ parentTag, std::make_shared<EntityNode>(parentTag, transform.parent) });
						
						nodes[parentTag]->children.push_back(nodes[tag]); // Add itself
					}
					else
					{ // No parent, add to root
						root->children.push_back(nodes[tag]);
					}
				}
				else
				{
					std::shared_ptr<EntityNode> addTo = root;
					if (transform.parent)
					{
						auto parentTag = transform.parent.Name();
						if (!nodes.contains(parentTag))
						{ // Create the parent, this parent will be an orphan for now
							nodes.insert({ parentTag, std::make_shared<EntityNode>(parentTag, transform.parent) });
						}

						addTo = nodes[parentTag];
					}

					auto node = std::make_shared<EntityNode>(tag, entity);
					addTo->children.push_back(node);
					nodes.insert({ tag, node });
				}
			}

			// Display the tree
			DrawNode(root);
		}

	private:
		Entity m_selected;

		// Recursive
		void DrawNode(std::shared_ptr<EntityNode> node)
		{
			UI::TreeNodeFlags flags = UI::TreeNodeFlags::OpenOnArrow;
			flags |= node->children.empty() ? UI::TreeNodeFlags::Leaf : UI::TreeNodeFlags::None;
			flags |= node->entity == m_selected ? UI::TreeNodeFlags::Selected : UI::TreeNodeFlags::None;

			UI::TreeNode n(node->tag, flags);
			if (n.IsClicked() && node->entity) // Also process the click if the node is closed
			{
				m_selected = node->entity;
				// Tell the inspector
				InspectorPanel::InspectElement(std::bind(InspectEntity, std::placeholders::_1, node->entity));
			}


			if(n.IsOpen())
			{
				for (auto child : node->children)
					DrawNode(child);
			}
		}

	};
}