/*
 * RigidBodyMoveTo.cpp
 *
 *  Created on: Dec 25, 2014
 *      Author: practicing01
 */

#include <Urho3D/Urho3D.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Math/BoundingBox.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Core/Variant.h>

#include "RigidBodyMoveTo.h"

RigidBodyMoveTo::RigidBodyMoveTo(Context* context) :
		LogicComponent(context)
{
	isMoving_ = false;
	// Only the physics update event is needed: unsubscribe from the rest for optimization
	SetUpdateEventMask(USE_FIXEDUPDATE);
}

void RigidBodyMoveTo::OnMoveToComplete()
{
	VariantMap vm;
	vm[RigidBodyMoveToComplete::P_NODE] = node_;
	SendEvent(E_RIGIDBODYMOVETOCOMPLETE,vm);
}

void RigidBodyMoveTo::MoveTo(Vector3 dest, float speed, bool stopOnCompletion)
{
	moveToSpeed_ = speed;
	moveToDest_ = dest;
	moveToLoc_ = node_->GetComponent<RigidBody>()->GetPosition();
	moveToDir_ = dest - moveToLoc_;
	moveToDir_.Normalize();
	moveToTravelTime_ = (moveToDest_ - moveToLoc_).Length() / speed;
	moveToElapsedTime_ = 0.0f;
	moveToStopOnTime_ = stopOnCompletion;
	isMoving_ = true;
	node_->GetComponent<RigidBody>()->SetLinearVelocity(moveToDir_ * speed);
}

void RigidBodyMoveTo::FixedUpdate(float timeStep)
{
	if (isMoving_ == true)
	{
		moveToElapsedTime_ += timeStep;
		if (moveToElapsedTime_ >= moveToTravelTime_)
		{
			moveToLoc_ = node_->GetComponent<RigidBody>()->GetPosition();
			if (moveToLoc_ != moveToDest_ && (moveToLoc_ - moveToDest_).Length() > 1.0f)
			{
				MoveTo(moveToDest_, moveToSpeed_, moveToStopOnTime_);
				return;
			}
			isMoving_ = false;
			if (moveToStopOnTime_ == true)
			{
				node_->GetComponent<RigidBody>()->SetLinearVelocity(Vector3::ZERO);
			}
			OnMoveToComplete();
		}
	}
}
