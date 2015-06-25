/*
 * TheGuy.cpp
 *
 *  Created on: Jun 7, 2015
 *      Author: practicing01
 */

#include <Urho3D/Urho3D.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/ParticleEmitter.h>
#include <Urho3D/Graphics/ParticleEffect.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Core/Variant.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/Resource/XMLFile.h>

#include "TheGuy.h"
#include "RigidBodyMoveTo.h"

TheGuy::TheGuy(Context* context) :
		LogicComponent(context)
{
	speed_ = 1.0f;
	elapsedTime_ = 0.0f;
	moveToInterval_ = 1.0f;
}

TheGuy::~TheGuy()
{
	UnsubscribeFromEvent(E_RIGIDBODYMOVETOCOMPLETE);
}

void TheGuy::Start()
{
	SubscribeToEvent(E_RIGIDBODYMOVETOCOMPLETE, HANDLER(TheGuy, HandleRigidBodyMoveToComplete));

	Vector3 victoria = gameplay_->gillBalentine_->GetComponent<RigidBody>()->GetPosition();
	victoria.y_ = node_->GetComponent<RigidBody>()->GetPosition().y_;

	node_->LookAt(victoria);

	node_->GetComponent<RigidBodyMoveTo>()->MoveTo(victoria, speed_, true);
	node_->GetComponent<AnimationController>()->PlayExclusive("Models/TheGuyWalk.ani", 0, true, 0.0f);
}

void TheGuy::Update(float timeStep)//todo logic based on triggers?
{//todo may never reach destination with moveTo cus of obstacles

	elapsedTime_ += timeStep;

	if (!node_->GetComponent<RigidBodyMoveTo>()->isMoving_)
	{
		Vector3 victoria = gameplay_->gillBalentine_->GetComponent<RigidBody>()->GetPosition();

		if ((node_->GetComponent<RigidBody>()->GetPosition() - victoria).Length() > 1.0f)
		{
			victoria.y_ = node_->GetComponent<RigidBody>()->GetPosition().y_;
			node_->LookAt(victoria);
			node_->GetComponent<RigidBodyMoveTo>()->MoveTo(victoria, speed_, true);
			node_->GetComponent<AnimationController>()->PlayExclusive("Models/TheGuyWalk.ani", 0, true, 0.0f);
		}
	}
	else if (elapsedTime_ >= moveToInterval_)
	{
		elapsedTime_ = 0.0f;
		Vector3 victoria = gameplay_->gillBalentine_->GetComponent<RigidBody>()->GetPosition();
		victoria.y_ = node_->GetComponent<RigidBody>()->GetPosition().y_;

		node_->LookAt(victoria);

		node_->GetComponent<RigidBodyMoveTo>()->MoveTo(victoria, speed_, true);
		node_->GetComponent<AnimationController>()->PlayExclusive("Models/TheGuyWalk.ani", 0, true, 0.0f);
	}
}

void TheGuy::HandleRigidBodyMoveToComplete(StringHash eventType, VariantMap& eventData)
{
	using namespace RigidBodyMoveToComplete;

	SharedPtr<Node> node = SharedPtr<Node>(static_cast<Node*>(eventData[P_NODE].GetPtr()));

	if (node == node_)
	{
		Vector3 victoria = gameplay_->gillBalentine_->GetComponent<RigidBody>()->GetPosition();
		victoria.y_ = node_->GetComponent<RigidBody>()->GetPosition().y_;

		node_->LookAt(victoria);

		if ((node_->GetComponent<RigidBody>()->GetPosition() - victoria).Length() < 1.0f)
		{
			node_->GetComponent<AnimationController>()->PlayExclusive("Models/TheGuyStand.ani", 0, true, 0.0f);
			return;
		}

		node_->GetComponent<RigidBodyMoveTo>()->MoveTo(victoria, speed_, true);
		node_->GetComponent<AnimationController>()->PlayExclusive("Models/TheGuyWalk.ani", 0, true, 0.0f);
	}
}
