#include "pch.hpp"
#include "Character.hpp"

#include "Asset/StaticModelLoader.hpp"

#include "Physics/BulletEngine.hpp"

Character::Character(const std::string& modelPath)
{
    mModel = StaticModelLoader::get(modelPath);
    
    btTransform t;
    t.setOrigin(btVector3(0, 0, 0));
    btMotionState* motion = new btDefaultMotionState(t);
    btCapsuleShape* shape = new btCapsuleShape(0.3f, 1.7f);
    btRigidBody::btRigidBodyConstructionInfo info(1.0f, motion, shape);
	info.m_friction = 0.0f;
	info.m_restitution = 0.0f;
	info.m_linearDamping = 0.0f;
    mBody = BulletEngine::createRigidBody(info);
    mBody->setAngularFactor(0.0f);
    mBody->setActivationState(DISABLE_DEACTIVATION);

    mMoveDirection = {0, 0, 0};
}

void Character::update()
{
    mBody->setLinearVelocity(Utils::glmToBtVec3(mMoveDirection));
}

void Character::render() const
{
    vk::CommandBuffer &cmdBuffer = VulkanEngine::gData.commandBuffer;
    vk::Pipeline &pipeline = VulkanEngine::gData.staticModelPipeline.pipeline;
    vk::PipelineLayout &pipelineLayout = VulkanEngine::gData.staticModelPipeline.layout;

    cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
    cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, {VulkanEngine::gData.globalDescriptorSet}, {});
    cmdBuffer.bindVertexBuffers(0, mModel->mPositionBuffer.buffer, {0});
    cmdBuffer.bindVertexBuffers(1, mModel->mNormalBuffer.buffer, {0});
    cmdBuffer.bindVertexBuffers(2, mModel->mUvBuffer.buffer, {0});
    cmdBuffer.bindIndexBuffer(mModel->mIndexBuffer.buffer, 0, vk::IndexType::eUint32);
    
    glm::mat4x4 modelTransform = buildModelTransform();
    cmdBuffer.pushConstants(pipelineLayout,
                            vk::ShaderStageFlagBits::eVertex, 0, sizeof(modelTransform), &modelTransform);
    
    for (const auto &mesh : mModel->mMeshes)
    {
        const StaticModelData::Material &material = mModel->mMaterials[mesh.materialIndex];

        cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout,
                                     1, {material.mDiffuse.descriptorSet}, {});
        cmdBuffer.drawIndexed(mesh.numIndices, 1, mesh.baseIndex, mesh.baseVertex, 0);
    }
}

glm::mat4x4 Character::buildModelTransform() const
{
    glm::mat4x4 scaleM = glm::scale(glm::mat4x4(1.0f), {1, 1, 1});
    glm::mat4x4 positionM = glm::translate(glm::mat4x4(1.0f), getPosition());

    return positionM * scaleM;
}