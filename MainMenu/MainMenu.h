/*
 * MainMenu.h
 *
 *  Created on: May 6, 2015
 *      Author: practicing01
 */

#pragma once

#include <Urho3D/Urho3D.h>

#include <Urho3D/Core/Object.h>
#include "../Urho3DPlayer.h"

using namespace Urho3D;

class MainMenu : public Object
{
	OBJECT(MainMenu);
public:
	MainMenu(Context* context, Urho3DPlayer* main);
	~MainMenu();

	void HandleUpdate(StringHash eventType, VariantMap& eventData);
	void TouchDown(StringHash eventType, VariantMap& eventData);

	Urho3DPlayer* main_;
	float elapsedTime_;

	SharedPtr<Scene> scene_;
	SharedPtr<Node> cameraNode_;
};
