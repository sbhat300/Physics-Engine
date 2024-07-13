#include "imgui/imguiInitialize.h"
#include <GLFW/glfw3.h>
#include <FileLoader/objDataLoader.h>
#include <string>

/*
TO COMPLETELY REMOVE FROM BUILD:
-Delete startingPosition/startingRotation/etc. from entity, polygon, and polygonCollider
-Delete everything starting with DataLoader:: and gui:: from main script
-Delete guiSave from entity
*/
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
        ImGui::Checkbox("Save all", &saveAll);
        if(currentID != -1) ImGui::Checkbox("Save edits", &(*(*entityList)[currentID]).guiSave);
        entityOptions();
        if(currentID != -1 && (*(*entityList)[currentID]).contain[0]) polygonOptions();
        if(currentID != -1 && (*(*entityList)[currentID]).contain[1]) polygonColliderOptions();
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
            output += std::to_string((*(*entityList)[i]).id) + 
                        " p" + std::to_string((*(*entityList)[i]).position.x) + "," + std::to_string((*(*entityList)[i]).position.y) +
                        " s" + std::to_string((*(*entityList)[i]).scale.x) + "," + std::to_string((*(*entityList)[i]).scale.y) +
                        " r" + std::to_string((*(*entityList)[i]).rotation) + "\n";
        }
        DataLoader::setFile(output);
    }
    else
    {
        for(auto i = 0; i <= maxEntityCount; i++)
        {
            output += std::to_string((*(*entityList)[i]).id);
            if((*(*entityList)[i]).guiSave)
                output += " p" + std::to_string((*(*entityList)[i]).position.x) + "," + std::to_string((*(*entityList)[i]).position.y) +
                        " s" + std::to_string((*(*entityList)[i]).scale.x) + "," + std::to_string((*(*entityList)[i]).scale.y) +
                        " r" + std::to_string((*(*entityList)[i]).rotation);

            else
                output += " p" + std::to_string((*(*entityList)[i]).startingPosition.x) + "," + std::to_string((*(*entityList)[i]).startingPosition.y) +
                        " s" + std::to_string((*(*entityList)[i]).startingScale.x) + "," + std::to_string((*(*entityList)[i]).startingScale.y) +
                        " r" + std::to_string((*(*entityList)[i]).startingRotation);
            output += "\n";
        }
        DataLoader::setFile(output);
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
                (*(*entityList)[currentID]).setScale(nums[0], nums[1]);
            nums[0] = (*(*entityList)[currentID]).rotation;
            if(ImGui::DragFloat("Rotation", &nums[0], 1))
                (*(*entityList)[currentID]).setRotation(nums[0]);
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
            (*(*entityList)[currentID]).polygonInstance.setScaleOffset(nums[0], nums[1]);
        nums[0] = (*(*entityList)[currentID]).polygonInstance.rotationOffset;
        if(ImGui::DragFloat("Rotation offset", &nums[0], 1))
            (*(*entityList)[currentID]).polygonInstance.setRotationOffset(nums[0]);
        if((*(*entityList)[currentID]).contain[1])
        {
            if(ImGui::Button("Sync polygon/collider offsets"))
            {
                (*(*entityList)[currentID]).polygonColliderInstance.setPositionOffset((*(*entityList)[currentID]).polygonInstance.positionOffset.x,
                                                                            (*(*entityList)[currentID]).polygonInstance.positionOffset.y);
                (*(*entityList)[currentID]).polygonColliderInstance.setScaleOffset((*(*entityList)[currentID]).polygonInstance.scaleOffset.x,
                                                                            (*(*entityList)[currentID]).polygonInstance.scaleOffset.y);
                (*(*entityList)[currentID]).polygonColliderInstance.setRotationOffset((*(*entityList)[currentID]).polygonInstance.rotationOffset);
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
        float nums[2] = {(*(*entityList)[currentID]).polygonColliderInstance.positionOffset[0], 
                        (*(*entityList)[currentID]).polygonColliderInstance.positionOffset[1]};
        if(ImGui::DragFloat2("Position offset", nums, 1))
            (*(*entityList)[currentID]).polygonColliderInstance.setPositionOffset(nums[0], nums[1]);
        nums[0] = (*(*entityList)[currentID]).polygonColliderInstance.scaleOffset.x;
        nums[1] = (*(*entityList)[currentID]).polygonColliderInstance.scaleOffset.y;
        if(ImGui::DragFloat2("Scale offset", nums, 0.25f))
            (*(*entityList)[currentID]).polygonColliderInstance.setScaleOffset(nums[0], nums[1]);
        nums[0] = (*(*entityList)[currentID]).polygonColliderInstance.rotationOffset;
        if(ImGui::DragFloat("Rotation offset", &nums[0], 1))
            (*(*entityList)[currentID]).polygonColliderInstance.setRotationOffset(nums[0]);
        if((*(*entityList)[currentID]).contain[0])
        {
            if(ImGui::Button("Sync collider/polygon offsets"))
            {
                (*(*entityList)[currentID]).polygonInstance.setPositionOffset((*(*entityList)[currentID]).polygonColliderInstance.positionOffset.x,
                                                                            (*(*entityList)[currentID]).polygonColliderInstance.positionOffset.y);
                (*(*entityList)[currentID]).polygonInstance.setScaleOffset((*(*entityList)[currentID]).polygonColliderInstance.scaleOffset.x,
                                                                            (*(*entityList)[currentID]).polygonColliderInstance.scaleOffset.y);
                (*(*entityList)[currentID]).polygonInstance.setRotationOffset((*(*entityList)[currentID]).polygonColliderInstance.rotationOffset);
            }
        }
        ImGui::Checkbox("Render bounds", &(*(*entityList)[currentID]).polygonColliderInstance.shouldRenderBounds);
        ImGui::TreePop();
    }
}