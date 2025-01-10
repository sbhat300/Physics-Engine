#include "ImguiImplementation/imguiInitialize.h"
#include <GLFW/glfw3.h>
#include <FileLoader/objDataLoader.h>
#include <algorithm>
#include <iostream>
#include <glm/glm.hpp>

/*
TO COMPLETELY REMOVE FROM BUILD:
-Delete startingPosition/startingRotation/etc. from entity, polygon, and polygonCollider
-Delete everything starting with DataLoader:: and gui:: from main script
-Delete guiSave from entity
*/

float gui::angularImpulse = 0;
float gui::impulses[2] = {0, 0};

void gui::init(GLFWwindow *window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();
}
void gui::preLoop()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    {
        ImGui::Begin("Editor");
        ImGui::PushItemWidth(100);
        ImGui::Text("FPS %f", fps);
        ImGui::Checkbox("Save all", &saveAll);
        if(currentID != -1) ImGui::Checkbox("Save edits", &(*(*entityList)[currentID]).guiSave);
        ImGui::Checkbox("Paused", &paused);
        if(ImGui::Button("Step") && paused)
        {
            debugStep();
        }
        entityOptions();
        if(currentID != -1 && (*entityList)[currentID]->contain[0]) polygonOptions();
        if(currentID != -1 && (*entityList)[currentID]->contain[1]) polygonColliderOptions();
        if(currentID != -1 && (*entityList)[currentID]->contain[2]) polygonRigidbodyOptions();
        shg();
        ImGui::End();
    }
}
void gui::postLoop()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
void gui::terminate()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    std::string output;
    if(saveAll)
    {
        for(auto i = 0; i <= maxEntityCount; i++)
        {
            output += getEntityData(i, true);
        }
        DataLoader::setFileData(output);
    }
    else
    {
        for(auto i = 0; i <= maxEntityCount; i++)
        {
            output += getEntityData(i, (*(*entityList)[i]).guiSave);
        }
        DataLoader::setFileData(output);
    }
}
void gui::entityOptions()
{
    if(ImGui::TreeNode("Entity"))
    {
        ImGui::Text("Entity ID %d", currentID);
        ImGui::Spacing();
        if(currentID != -1)
        {
            ImGui::Text("Entity Name %s", (*(*entityList)[currentID]).label);
            float nums[2] = {(*(*entityList)[currentID]).position.x, (*(*entityList)[currentID]).position.y};
            if(ImGui::DragFloat2("Position", nums, 1))
                (*(*entityList)[currentID]).setPosition(nums[0], nums[1]);
            nums[0] = (*(*entityList)[currentID]).scale.x;
            nums[1] = (*(*entityList)[currentID]).scale.y;
            if(ImGui::DragFloat2("Scale", nums, 1))
                (*(*entityList)[currentID]).setScale(std::max(0.0f, nums[0]), std::max(0.0f, nums[1]));
            nums[0] = glm::degrees((*(*entityList)[currentID]).rotation);
            if(ImGui::DragFloat("Rotation", &nums[0], 1))
                (*(*entityList)[currentID]).setRotation(glm::radians(nums[0]));
        }
        ImGui::TreePop();
    } 
}  
void gui::polygonOptions()
{
    if(ImGui::TreeNode("Polygon"))
    {
        float nums[2] = {(*(*entityList)[currentID]).polygonInstance.positionOffset[0], 
                        (*(*entityList)[currentID]).polygonInstance.positionOffset[1]};
        if(ImGui::DragFloat2("Position offset", nums, 1))
            (*(*entityList)[currentID]).polygonInstance.setPositionOffset(nums[0], nums[1]);
        nums[0] = (*(*entityList)[currentID]).polygonInstance.scaleOffset.x;
        nums[1] = (*(*entityList)[currentID]).polygonInstance.scaleOffset.y;
        if(ImGui::DragFloat2("Scale offset", nums, 0.25f))
            (*(*entityList)[currentID]).polygonInstance.setScaleOffset(std::max(0.0f, nums[0]), std::max(0.0f, nums[1]));
        nums[0] = glm::degrees((*(*entityList)[currentID]).polygonInstance.rotationOffset);
        if(ImGui::DragFloat("Rotation offset", &nums[0], 1))
            (*(*entityList)[currentID]).polygonInstance.setRotationOffset(glm::radians(nums[0]));
        if((*(*entityList)[currentID]).contain[1])
        {
            if(ImGui::Button("Sync polygon/collider offsets"))
            {
                (*(*entityList)[currentID]).collider.setPositionOffset((*(*entityList)[currentID]).polygonInstance.positionOffset.x,
                                                                            (*(*entityList)[currentID]).polygonInstance.positionOffset.y);
                (*(*entityList)[currentID]).collider.setScaleOffset((*(*entityList)[currentID]).polygonInstance.scaleOffset.x,
                                                                            (*(*entityList)[currentID]).polygonInstance.scaleOffset.y);
                (*(*entityList)[currentID]).collider.setRotationOffset((*(*entityList)[currentID]).polygonInstance.rotationOffset);
            }
        }
        int layer = (*(*entityList)[currentID]).polygonInstance.layer;
        if(ImGui::InputInt("Layer", &layer, 1))
            (*(*entityList)[currentID]).polygonInstance.setLayer(layer);
        float color[3] = {  (*(*entityList)[currentID]).polygonInstance.color.x,
                            (*(*entityList)[currentID]).polygonInstance.color.y,
                            (*(*entityList)[currentID]).polygonInstance.color.z};
        if(ImGui::ColorEdit3("Polygon color", color))
            (*(*entityList)[currentID]).polygonInstance.setColor(glm::vec3(color[0], color[1], color[2]));
        ImGui::TreePop();
    }
}
void gui::polygonColliderOptions()
{
    
    if(ImGui::TreeNode("Polygon Collider"))
    {
        ImGui::Text("AABB: %s", (*(*entityList)[currentID]).collider.aabb ? "true" : "false");
        float nums[2] = {(*(*entityList)[currentID]).collider.positionOffset[0], 
                        (*(*entityList)[currentID]).collider.positionOffset[1]};
        if(ImGui::DragFloat2("Position offset", nums, 1))
            (*(*entityList)[currentID]).collider.setPositionOffset(nums[0], nums[1]);
        nums[0] = (*(*entityList)[currentID]).collider.scaleOffset.x;
        nums[1] = (*(*entityList)[currentID]).collider.scaleOffset.y;
        if(ImGui::DragFloat2("Scale offset", nums, 0.25f))
            (*(*entityList)[currentID]).collider.setScaleOffset(std::max(0.0f, nums[0]), std::max(0.0f, nums[1]));
        nums[0] = glm::degrees((*(*entityList)[currentID]).collider.rotationOffset);
        if(ImGui::DragFloat("Rotation offset", &nums[0], 1))
            (*(*entityList)[currentID]).collider.setRotationOffset(glm::radians(nums[0]));
        if((*(*entityList)[currentID]).contain[0])
        {
            if(ImGui::Button("Sync collider/polygon offsets"))
            {
                (*(*entityList)[currentID]).polygonInstance.setPositionOffset((*(*entityList)[currentID]).collider.positionOffset.x,
                                                                            (*(*entityList)[currentID]).collider.positionOffset.y);
                (*(*entityList)[currentID]).polygonInstance.setScaleOffset((*(*entityList)[currentID]).collider.scaleOffset.x,
                                                                            (*(*entityList)[currentID]).collider.scaleOffset.y);
                (*(*entityList)[currentID]).polygonInstance.setRotationOffset((*(*entityList)[currentID]).collider.rotationOffset);
            }
        }
        ImGui::Checkbox("Render bounds", &(*(*entityList)[currentID]).collider.shouldRenderBounds);
        ImGui::TreePop();
    }
}
void gui::polygonRigidbodyOptions()
{
    if(ImGui::TreeNode("Polygon Rigidbody"))
    {
        float num = (*entityList)[currentID]->rigidbody.mass;
        if(ImGui::DragFloat("Mass", &num, 1.0f)) (*entityList)[currentID]->rigidbody.setMass(num);
        num = (*entityList)[currentID]->rigidbody.momentOfInertia;
        if(ImGui::DragFloat("MOI", &num, 1.0f)) (*entityList)[currentID]->rigidbody.setMomentOfInertia(num);
        num = (*entityList)[currentID]->rigidbody.restitution;
        if(ImGui::DragFloat("COR", &num, 0.01f)) (*entityList)[currentID]->rigidbody.setRestitution(num);
        num = (*entityList)[currentID]->rigidbody.mu;
        if(ImGui::DragFloat("mu", &num, 0.01f)) (*entityList)[currentID]->rigidbody.setFriction(num);
        float nums[2] = {(*entityList)[currentID]->rigidbody.velocity.x, (*entityList)[currentID]->rigidbody.velocity.y};
        if(ImGui::DragFloat2("V", nums, 1.0f)) (*entityList)[currentID]->rigidbody.velocity = glm::vec2(nums[0], nums[1]);
        num = (*entityList)[currentID]->rigidbody.angularVelocity;
        if(ImGui::DragFloat("w", &num, 1.0f)) (*entityList)[currentID]->rigidbody.angularVelocity = num;
        ImGui::DragFloat2("Impulse", impulses, 1);
        if(ImGui::Button("Apply L")) (*entityList)[currentID]->rigidbody.addImpulse(impulses[0], impulses[1]);
        ImGui::DragFloat("Ang Impulse", &angularImpulse, 1);
        if(ImGui::Button("Apply J")) (*entityList)[currentID]->rigidbody.addAngularImpulse(angularImpulse);
        ImGui::TreePop();
    }
}
void gui::shg()
{
    if(ImGui::TreeNode("Spatial Hash Grid"))
    {
        if(ImGui::BeginTable("SHG", (int)(*spatialHash).numCells.x))
        {
            for(int i = (int)(*spatialHash).numCells.y - 1; i >= 0; i--)
            {
                for(int j = 0; j < (*spatialHash).numCells.x; j++)
                {
                    std::vector<polygonCollider*> colliders = (*spatialHash).grid[j][i];
                    std::string output;
                    if(colliders.size() == 0) output  = "N/A";
                    else 
                    {
                        output = "";
                        for(int i = 0; i < colliders.size() - 1; i++)
                        {
                            output += std::to_string((*colliders[i]).id) + ",";
                        }
                        output += std::to_string((*colliders[colliders.size() - 1]).id);
                    }
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", output.c_str());
                }
            }
            ImGui::EndTable();
        }
        ImGui::TreePop();
    }
}
std::string gui::getEntityData(int i, bool saved)
{
    std::string output;
    output += std::to_string((*(*entityList)[i]).id);
    if(saved)
        output += " p" + std::to_string((*(*entityList)[i]).position.x) + "," + std::to_string((*(*entityList)[i]).position.y) +
                " s" + std::to_string((*(*entityList)[i]).scale.x) + "," + std::to_string((*(*entityList)[i]).scale.y) +
                " r" + std::to_string((*(*entityList)[i]).rotation);

    else
        output += " p" + std::to_string((*(*entityList)[i]).startingPosition.x) + "," + std::to_string((*(*entityList)[i]).startingPosition.y) +
                " s" + std::to_string((*(*entityList)[i]).startingScale.x) + "," + std::to_string((*(*entityList)[i]).startingScale.y) +
                " r" + std::to_string((*(*entityList)[i]).startingRotation);
    if(!(*(*entityList)[i]).contain[0]) 
        output += " x x x x x";
    else
        if(saved)
            output += " po" + std::to_string((*(*entityList)[i]).polygonInstance.positionOffset.x) + "," + std::to_string((*(*entityList)[i]).polygonInstance.positionOffset.y) +
                    " so" + std::to_string((*(*entityList)[i]).polygonInstance.scaleOffset.x) + "," + std::to_string((*(*entityList)[i]).polygonInstance.scaleOffset.y) + 
                    " ro" + std::to_string((*(*entityList)[i]).polygonInstance.rotationOffset) + 
                    " col" + std::to_string((*(*entityList)[i]).polygonInstance.color.x) + "," + std::to_string((*(*entityList)[i]).polygonInstance.color.y) + "," + std::to_string((*(*entityList)[i]).polygonInstance.color.z) +
                    " l" + std::to_string((*(*entityList)[i]).polygonInstance.layer);
        else
            output += " po" + std::to_string((*(*entityList)[i]).polygonInstance.startPositionOffset.x) + "," + std::to_string((*(*entityList)[i]).polygonInstance.startPositionOffset.y) +
                    " so" + std::to_string((*(*entityList)[i]).polygonInstance.startScaleOffset.x) + "," + std::to_string((*(*entityList)[i]).polygonInstance.startScaleOffset.y) + 
                    " ro" + std::to_string((*(*entityList)[i]).polygonInstance.startRotationOffset) + 
                    " col" + std::to_string((*(*entityList)[i]).polygonInstance.startColor.x) + "," + std::to_string((*(*entityList)[i]).polygonInstance.startColor.y) + "," + std::to_string((*(*entityList)[i]).polygonInstance.startColor.z) +
                    " l" + std::to_string((*(*entityList)[i]).polygonInstance.startLayer);
    if(!(*(*entityList)[i]).contain[1])
        output += " x x x";
    else
        if(saved)
            output += " pco" + std::to_string((*(*entityList)[i]).collider.positionOffset.x) + "," + std::to_string((*(*entityList)[i]).collider.positionOffset.y) + 
                    " sco" + std::to_string((*(*entityList)[i]).collider.scaleOffset.x) + "," + std::to_string((*(*entityList)[i]).collider.scaleOffset.y) + 
                    " rco" + std::to_string((*(*entityList)[i]).collider.rotationOffset);
        else
            output += " pco" + std::to_string((*(*entityList)[i]).collider.startPositionOffset.x) + "," + std::to_string((*(*entityList)[i]).collider.startPositionOffset.y) + 
                " sco" + std::to_string((*(*entityList)[i]).collider.startScaleOffset.x) + "," + std::to_string((*(*entityList)[i]).collider.startScaleOffset.y) + 
                " rco" + std::to_string((*(*entityList)[i]).collider.startRotationOffset);
    if(!(*(*entityList)[i]).contain[2])
        output += " x x x x";
    else
        if(saved)
            output += " prm" + std::to_string((*entityList)[i]->rigidbody.mass) +
                      " pri" + std::to_string((*entityList)[i]->rigidbody.momentOfInertia) + 
                      " prr" + std::to_string((*entityList)[i]->rigidbody.restitution) + 
                      " pru" + std::to_string((*entityList)[i]->rigidbody.mu);
        else
            output += " prm" + std::to_string((*entityList)[i]->rigidbody.startMass) +
                      " pri" + std::to_string((*entityList)[i]->rigidbody.startMOI) + 
                      " prr" + std::to_string((*entityList)[i]->rigidbody.startRestitution) + 
                      " pru" + std::to_string((*entityList)[i]->rigidbody.startMu);

    output += "\n";
    return output;
}