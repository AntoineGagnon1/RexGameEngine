#pragma once

#include <string>

#include <RexEngine.h>

#include "Panel.h"
#include "ui/Gui.h"

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
				Imgui::Text("No scene loaded !");
				return;
			}

			// Make the node tree
			auto root = std::make_shared<EntityNode>("Active scene", Entity());
			std::unordered_map<std::string, std::shared_ptr<EntityNode>> nodes;

			auto transforms = scene.GetComponents<TransformComponent>();
			for (auto&& [entity, transform] : transforms)
			{
				auto tag = entity.GetComponent<TagComponent>().tag;
				if (nodes.contains(tag))
				{
					// Fix orphans
					if (transform.parent)
					{
						auto parentTag = transform.parent.GetComponent<TagComponent>().tag;
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
						auto parentTag = transform.parent.GetComponent<TagComponent>().tag;
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
			bool showNode = Imgui::TreeNode(node->tag, node->children.empty(), true, node->entity == m_selected);
			if (Imgui::IsItemClicked()) // Also process the click if the node is closed
			{
				m_selected = node->entity; // TODO : tell the inspector
			}


			if(showNode)
			{
				for (auto child : node->children)
					DrawNode(child);

				Imgui::TreePop();
			}
		}

	};
}