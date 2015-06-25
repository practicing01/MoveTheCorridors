/*
 * TheGuy.h
 *
 *  Created on: Jun 7, 2015
 *      Author: practicing01
 */

#pragma once

#include <Urho3D/Urho3D.h>
#include <Urho3D/Core/Object.h>
#include <Urho3D/Scene/LogicComponent.h>

#include "../Urho3DPlayer.h"
#include "Gameplay.h"

// All Urho3D classes reside in namespace Urho3D
using namespace Urho3D;

class TheGuy: public LogicComponent
{
	OBJECT(TheGuy);
public:
	TheGuy(Context* context);
	~TheGuy();
	/// Handle scene update. Called by LogicComponent base class.
	virtual void Update(float timeStep);
	virtual void Start();

	void HandleRigidBodyMoveToComplete(StringHash eventType, VariantMap& eventData);

	Urho3DPlayer* main_;
	Gameplay* gameplay_;

	float speed_;
	float elapsedTime_;
	float moveToInterval_;
};
