/*
 * Gameplay.h
 *
 *  Created on: May 6, 2015
 *      Author: practicing01
 */

#pragma once

#include <Urho3D/Urho3D.h>

#include <Urho3D/Core/Object.h>
#include "../Urho3DPlayer.h"

using namespace Urho3D;

EVENT(E_GAMEEVENT, GameEvent)
{
	PARAM(P_DATA, Data);
}

class Gameplay : public Object
{
	OBJECT(Gameplay);
public:
	Gameplay(Context* context, Urho3DPlayer* main);
	~Gameplay();

	void HandleUpdate(StringHash eventType, VariantMap& eventData);
	void HandleElementResize(StringHash eventType, VariantMap& eventData);
	void HandlePressed(StringHash eventType, VariantMap& eventData);
	void HandleReleased(StringHash eventType, VariantMap& eventData);
	void HandleHoverEnd(StringHash eventType, VariantMap& eventData);
	void HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData);
	void HandlePhysicsPreStep(StringHash eventType, VariantMap& eventData);
	void HandleFrameCollisionStart(StringHash eventType, VariantMap& eventData);
	void HandleGameEvent(StringHash eventType, VariantMap& eventData);
	void HandleEndFireCollisionStart(StringHash eventType, VariantMap& eventData);

	Urho3DPlayer* main_;
	float elapsedTime_;

	SharedPtr<Scene> scene_;
	SharedPtr<Node> cameraNode_;
	SharedPtr<Node> gillBalentine_;
	SharedPtr<Node> frame_;
	SharedPtr<Node> wheelBL_;
	SharedPtr<Node> wheelBR_;
	SharedPtr<Node> wheelFL_;
	SharedPtr<Node> wheelFR_;
	SharedPtr<Node> theGuy_;
	SharedPtr<Node> tongs_;
	SharedPtr<Node> gear_;
	SharedPtr<Node> endFire_;

	SharedPtr<UIElement> dpads_;

	IntVector2 previousExtents_;

	Vector<Node*> keyItems_;
	Vector<Node*> gasTanks_;

	float moveTorque_;
	float turnTorque_;
	char moving_;
	char turning_;

	bool endGame_;
};
