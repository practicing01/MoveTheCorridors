/*
 * MainMenu.cpp
 *
 *  Created on: May 6, 2015
 *      Author: practicing01
 */

#include <Urho3D/Urho3D.h>

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/UI/Sprite.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/Graphics/Viewport.h>

#include "MainMenu.h"

#include "../Gameplay/Gameplay.h"

MainMenu::MainMenu(Context* context, Urho3DPlayer* main) :
    Object(context)
{
	main_ = main;
	elapsedTime_ = 0.0f;

	scene_ = new Scene(context_);
	cameraNode_ = new Node(context_);

	File loadFile(context_,main_->filesystem_->GetProgramDir()
			+ "Data/Scenes/mainMenu.xml", FILE_READ);
	scene_->LoadXML(loadFile);

	cameraNode_ = scene_->GetChild("camera");

	if (GetPlatform() == "Android")
	{
		main_->renderer_->SetMobileShadowBiasAdd(0.001);
	}

	main_->viewport_ = new Viewport(context_, scene_, cameraNode_->GetComponent<Camera>());
	main_->renderer_->SetViewport(0, main_->viewport_);
	main_->viewport_->SetScene(scene_);
	main_->viewport_->SetCamera(cameraNode_->GetComponent<Camera>());

	SubscribeToEvent(E_TOUCHBEGIN, HANDLER(MainMenu, TouchDown));
}

MainMenu::~MainMenu()
{
}

void MainMenu::TouchDown(StringHash eventType, VariantMap& eventData)
{
	using namespace TouchBegin;

	new Gameplay(context_, main_);

	delete this;
}
