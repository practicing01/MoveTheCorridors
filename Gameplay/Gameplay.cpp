/*
 * Gameplay.cpp
 *
 *  Created on: May 6, 2015
 *      Author: practicing01
 */

#include <Urho3D/Urho3D.h>

#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Navigation/Navigable.h>
#include <Urho3D/Navigation/NavigationMesh.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/UI/Sprite.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Audio/Audio.h>
#include <Urho3D/Audio/SoundListener.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Audio/SoundSource3D.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/Graphics/Viewport.h>

#include "Gameplay.h"
#include "TheGuy.h"
#include "RigidBodyMoveTo.h"
#include "../MainMenu/MainMenu.h"

Gameplay::Gameplay(Context* context, Urho3DPlayer* main) :
    Object(context)
{
	main_ = main;
	context->RegisterFactory<TheGuy>();
	context->RegisterFactory<RigidBodyMoveTo>();

	elapsedTime_ = 0.0f;
	previousExtents_ = IntVector2(800, 480);

	moveTorque_ = 50.0f;
	turnTorque_ = 1.0f;
	moving_ = 0;
	turning_ = 0;

	scene_ = new Scene(context_);
	cameraNode_ = new Node(context_);

	File loadFile(context_,main_->filesystem_->GetProgramDir()
			+ "Data/Scenes/corridors.xml", FILE_READ);
	scene_->LoadXML(loadFile);

	cameraNode_ = scene_->GetChild("cameras")->GetChild("camera");

	if (GetPlatform() == "Android")
	{
		//main_->renderer_->SetReuseShadowMaps(false);
		//main_->renderer_->SetShadowQuality(SHADOWQUALITY_LOW_16BIT);
		//main_->renderer_->SetMobileShadowBiasMul(2.0f);
		main_->renderer_->SetMobileShadowBiasAdd(0.001);
	}

	main_->viewport_ = new Viewport(context_, scene_, cameraNode_->GetComponent<Camera>());
	main_->renderer_->SetViewport(0, main_->viewport_);
	main_->viewport_->SetScene(scene_);
	main_->viewport_->SetCamera(cameraNode_->GetComponent<Camera>());

	SubscribeToEvent(E_RESIZED, HANDLER(Gameplay, HandleElementResize));
	dpads_ = main_->ui_->LoadLayout(main_->cache_->GetResource<XMLFile>("UI/dpads.xml"));
    main_->ui_->GetRoot()->AddChild(dpads_);

    for (int x = 0; x < dpads_->GetNumChildren(); x++)
    {
    	SubscribeToEvent(dpads_->GetChild(x), E_PRESSED, HANDLER(Gameplay, HandlePressed));
    	SubscribeToEvent(dpads_->GetChild(x), E_RELEASED, HANDLER(Gameplay, HandleReleased));
    	SubscribeToEvent(dpads_->GetChild(x), E_HOVEREND, HANDLER(Gameplay, HandleHoverEnd));
    }

    frame_ = scene_->GetChild("frame");
    wheelBL_ = frame_->GetChild("wheel.bl");
    wheelBR_ = frame_->GetChild("wheel.br");
    wheelFL_ = frame_->GetChild("wheel.fl");
    wheelFR_ = frame_->GetChild("wheel.fr");

    gillBalentine_ = scene_->GetChild("TheGirl");

    theGuy_ = scene_->GetChild("TheGuy");

    theGuy_->AddComponent(new RigidBodyMoveTo(context_), 0, LOCAL);

    TheGuy* _TheGuy = new TheGuy(context_);
    _TheGuy->main_ = main_;
    _TheGuy->gameplay_ = this;
    theGuy_->AddComponent(_TheGuy, 0, LOCAL);

    tongs_ = scene_->GetChild("tongs");

    gear_ = scene_->GetChild("gear");

    endFire_ = scene_->GetChild("endFire");

    endGame_ = false;

    main_->audio_->SetListener(gillBalentine_->GetComponent<SoundListener>());

    SubscribeToEvent(E_UPDATE, HANDLER(Gameplay, HandleUpdate));

    SubscribeToEvent(E_POSTRENDERUPDATE, HANDLER(Gameplay, HandlePostRenderUpdate));

    SubscribeToEvent(E_PHYSICSPRESTEP, HANDLER(Gameplay, HandlePhysicsPreStep));

    SubscribeToEvent(frame_, E_NODECOLLISIONSTART, HANDLER(Gameplay, HandleFrameCollisionStart));

    SubscribeToEvent(endFire_, E_NODECOLLISIONSTART, HANDLER(Gameplay, HandleEndFireCollisionStart));

    SubscribeToEvent(E_GAMEEVENT, HANDLER(Gameplay, HandleGameEvent));
}

Gameplay::~Gameplay()
{
	main_->ui_->GetRoot()->RemoveChild(dpads_);
}

void Gameplay::HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData)
{
	//main_->renderer_->DrawDebugGeometry(true);
	//scene_->GetComponent<PhysicsWorld>()->DrawDebugGeometry(true);
	//wheelBL_->GetComponent<CollisionShape>()->DrawDebugGeometry(main_->GetSubsystem<DebugRenderer>(), true);
	//wheelBR_->GetComponent<CollisionShape>()->DrawDebugGeometry(main_->GetSubsystem<DebugRenderer>(), true);
}

void Gameplay::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
	using namespace Update;

	float timeStep = eventData[P_TIMESTEP].GetFloat();

	//LOGERRORF("client loop");
	elapsedTime_ += timeStep;

	if (main_->input_->GetKeyDown(SDLK_ESCAPE))
	{
		main_->GetSubsystem<Engine>()->Exit();
	}

	if (endGame_)
	{
		new MainMenu(context_, main_);//todo make end scrolling credits
		delete this;
	}
}

void Gameplay::HandlePhysicsPreStep(StringHash eventType, VariantMap& eventData)
{
	using namespace PhysicsPreStep;

	float timeStep = eventData[P_TIMESTEP].GetFloat();

	Quaternion quarterOnion = frame_->GetComponent<RigidBody>()->GetRotation();

	if (moving_ == 0 && turning_ == 0)
	{
		wheelBL_->GetComponent<RigidBody>()->SetAngularDamping(0.5f);
		wheelBR_->GetComponent<RigidBody>()->SetAngularDamping(0.5f);

		wheelFL_->GetComponent<RigidBody>()->SetAngularDamping(0.5f);
		wheelFR_->GetComponent<RigidBody>()->SetAngularDamping(0.5f);

		frame_->GetChild("wheelsfx")->GetComponent<SoundSource3D>()->Stop();
		return;
	}
	else
	{
		wheelBL_->GetComponent<RigidBody>()->SetAngularDamping(0.0f);
		wheelBR_->GetComponent<RigidBody>()->SetAngularDamping(0.0f);

		wheelFL_->GetComponent<RigidBody>()->SetAngularDamping(0.0f);
		wheelFR_->GetComponent<RigidBody>()->SetAngularDamping(0.0f);

		String sound = frame_->GetChild("wheelsfx")->GetVar("Sound").GetString();
		frame_->GetChild("wheelsfx")->GetComponent<SoundSource3D>()->Play(
				main_->cache_->GetResource<Sound>(sound));
	}

	if (moving_ > 0)//Forward
	{
		Vector3 victoria = quarterOnion * Vector3(0.0f, 0.0f, -moveTorque_ * timeStep);

		wheelBL_->GetComponent<RigidBody>()->ApplyTorque(victoria);
		wheelBR_->GetComponent<RigidBody>()->ApplyTorque(victoria);

		wheelFL_->GetComponent<RigidBody>()->ApplyTorque(victoria);
		wheelFR_->GetComponent<RigidBody>()->ApplyTorque(victoria);
	}
	else if (moving_ < 0)//Backward
	{
		Vector3 victoria = quarterOnion * Vector3(0.0f, 0.0f, moveTorque_ * timeStep);

		wheelBL_->GetComponent<RigidBody>()->ApplyTorque(victoria);
		wheelBR_->GetComponent<RigidBody>()->ApplyTorque(victoria);

		wheelFL_->GetComponent<RigidBody>()->ApplyTorque(victoria);
		wheelFR_->GetComponent<RigidBody>()->ApplyTorque(victoria);
	}
	else if (turning_ > 0)//Right
	{
		Vector3 forward = quarterOnion * Vector3(0.0f, 0.0f, -moveTorque_ * timeStep);
		Vector3 backward = quarterOnion * Vector3(0.0f, 0.0f, moveTorque_ * timeStep);

		wheelBL_->GetComponent<RigidBody>()->ApplyTorque(backward);
		wheelBR_->GetComponent<RigidBody>()->ApplyTorque(forward);

		wheelFL_->GetComponent<RigidBody>()->ApplyTorque(backward);
		wheelFR_->GetComponent<RigidBody>()->ApplyTorque(forward);
	}
	else if (turning_ < 0)//Left
	{
		Vector3 forward = quarterOnion * Vector3(0.0f, 0.0f, -moveTorque_ * timeStep);
		Vector3 backward = quarterOnion * Vector3(0.0f, 0.0f, moveTorque_ * timeStep);

		wheelBL_->GetComponent<RigidBody>()->ApplyTorque(forward);
		wheelBR_->GetComponent<RigidBody>()->ApplyTorque(backward);

		wheelFL_->GetComponent<RigidBody>()->ApplyTorque(forward);
		wheelFR_->GetComponent<RigidBody>()->ApplyTorque(backward);
	}

	//quarterOnion = frame_->GetComponent<RigidBody>()->GetRotation();

	//Vector3 localVelocity = quarterOnion.Inverse() * frame_->GetComponent<RigidBody>()->GetLinearVelocity();
	//frame_->GetComponent<RigidBody>()->ApplyForce(quarterOnion * Vector3::DOWN * Abs(localVelocity.z_) * 10.0f);
}

void Gameplay::HandleElementResize(StringHash eventType, VariantMap& eventData)
{
	using namespace Resized;

	UIElement* ele = static_cast<UIElement*>(eventData[ElementAdded::P_ELEMENT].GetPtr());

	IntVector2 rootExtent = main_->ui_->GetRoot()->GetSize();

	IntVector2 scaledExtent;

	scaledExtent.x_ = ( ele->GetWidth() *  rootExtent.x_ ) / previousExtents_.x_;
	scaledExtent.y_ = ( ele->GetHeight() *  rootExtent.y_ ) / previousExtents_.y_;

	ele->SetSize(scaledExtent);

	IntVector2 scaledPosition = IntVector2(
			( ele->GetPosition().x_ *  rootExtent.x_ ) / previousExtents_.x_,
			( ele->GetPosition().y_ *  rootExtent.y_ ) / previousExtents_.y_);

	ele->SetPosition(scaledPosition);
}

void Gameplay::HandlePressed(StringHash eventType, VariantMap& eventData)
{
	using namespace Pressed;

	UIElement* ele = static_cast<UIElement*>(eventData[ElementAdded::P_ELEMENT].GetPtr());

	if (ele->GetName() == "upButt1" || ele->GetName() == "upButt2")
	{
		moving_++;
	}
	else if (ele->GetName() == "downButt1" || ele->GetName() == "downButt2")
	{
		moving_--;
	}
	else if (ele->GetName() == "leftButt2" || ele->GetName() == "leftButt1")
	{
		turning_++;
	}
	else if (ele->GetName() == "rightButt2" || ele->GetName() == "rightButt1")
	{
		turning_--;
	}
	else if (ele->GetName() == "menuButt")
	{
		new MainMenu(context_, main_);
		delete this;
		return;
	}
}

void Gameplay::HandleReleased(StringHash eventType, VariantMap& eventData)
{
	using namespace Released;

	UIElement* ele = static_cast<UIElement*>(eventData[ElementAdded::P_ELEMENT].GetPtr());

	if (ele->GetName() == "upButt1" || ele->GetName() == "upButt2")
	{
		if (moving_ > 0)
		{
			moving_--;
		}
	}
	else if (ele->GetName() == "downButt1" || ele->GetName() == "downButt2")
	{
		if (moving_ < 0)
		{
			moving_++;
		}
	}
	else if (ele->GetName() == "leftButt2" || ele->GetName() == "leftButt1")
	{
		if (turning_ > 0)
		{
			turning_--;
		}
	}
	else if (ele->GetName() == "rightButt2" || ele->GetName() == "rightButt1")
	{
		if (turning_ < 0)
		{
			turning_++;
		}
	}
}

void Gameplay::HandleFrameCollisionStart(StringHash eventType, VariantMap& eventData)
{
	using namespace NodeCollisionStart;

	Node* noed = static_cast<RigidBody*>(eventData[P_BODY].GetPtr())->GetNode();
	Node* OtherNode = static_cast<Node*>(eventData[P_OTHERNODE].GetPtr());
	bool trigger = eventData[P_TRIGGER].GetBool();

	if (trigger)
	{
		int type = OtherNode->GetVar("Type").GetInt();

		if (type == 0)//camera
		{
			cameraNode_ = scene_->GetChild("cameras")->GetChild(OtherNode->GetVar("camera").GetString());
			main_->viewport_->SetCamera(cameraNode_->GetComponent<Camera>());
		}
		else if (type == 1)//key item
		{
			if (OtherNode == tongs_)
			{
				keyItems_.Push(OtherNode);
				OtherNode->SetEnabledRecursive(false);

				String sound2 = theGuy_->GetChild("whereareyou")->GetVar("Sound").GetString();
				theGuy_->GetChild("whereareyou")->GetComponent<SoundSource3D>()->Play(
						main_->cache_->GetResource<Sound>(sound2));
			}
			else if (OtherNode == gear_)
			{
				if (keyItems_.Contains(tongs_))
				{
					keyItems_.Push(OtherNode);
					OtherNode->SetEnabledRecursive(false);
				}
			}
		}
		else if (type == 2)//event
		{
			int event = OtherNode->GetVar("Event").GetInt();
			VariantMap eventData;
			eventData["node"] = noed;
			eventData["otherNode"] = OtherNode;
			eventData["event"] = event;
			SendEvent(E_GAMEEVENT, eventData);
		}
	}
}

void Gameplay::HandleEndFireCollisionStart(StringHash eventType, VariantMap& eventData)
{
	using namespace NodeCollisionStart;

	Node* noed = static_cast<RigidBody*>(eventData[P_BODY].GetPtr())->GetNode();
	Node* OtherNode = static_cast<Node*>(eventData[P_OTHERNODE].GetPtr());

	if (OtherNode->GetName() == "gasTank")
	{
		if (!gasTanks_.Contains(OtherNode))
		{
			gasTanks_.Push(OtherNode);
		}
	}
	else if (OtherNode->GetName() == "TheGuy")
	{
		if (gasTanks_.Size() == 2)
		{
			for (int x = 0; x < gasTanks_.Size(); x++)
			{
				gasTanks_[x]->GetChild("explosion")->SetEnabled(true);

				String sound = theGuy_->GetChild("screamsfx")->GetVar("Sound").GetString();
				theGuy_->GetChild("screamsfx")->GetComponent<SoundSource3D>()->Play(
						main_->cache_->GetResource<Sound>(sound));

				String sound2 = noed->GetChild("explosionsfx")->GetVar("Sound").GetString();
				noed->GetChild("explosionsfx")->GetComponent<SoundSource3D>()->Play(
						main_->cache_->GetResource<Sound>(sound2));
			}

			endGame_ = true;
		}
	}
}

void Gameplay::HandleHoverEnd(StringHash eventType, VariantMap& eventData)
{
	using namespace HoverEnd;

	UIElement* ele = static_cast<UIElement*>(eventData[ElementAdded::P_ELEMENT].GetPtr());

	if (ele->GetName() == "upButt1" || ele->GetName() == "upButt2")
	{
		if (moving_ > 0)
		{
			moving_--;
		}
	}
	else if (ele->GetName() == "downButt1" || ele->GetName() == "downButt2")
	{
		if (moving_ < 0)
		{
			moving_++;
		}
	}
	else if (ele->GetName() == "leftButt2" || ele->GetName() == "leftButt1")
	{
		if (turning_ > 0)
		{
			turning_--;
		}
	}
	else if (ele->GetName() == "rightButt2" || ele->GetName() == "rightButt1")
	{
		if (turning_ < 0)
		{
			turning_++;
		}
	}
}

void Gameplay::HandleGameEvent(StringHash eventType, VariantMap& eventData)
{
	using namespace GameEvent;

	SharedPtr<Node> node = SharedPtr<Node>(static_cast<Node*>(eventData["node"].GetPtr()));
	SharedPtr<Node> otherNode = SharedPtr<Node>(static_cast<Node*>(eventData["otherNode"].GetPtr()));
	int event = eventData["event"].GetInt();

	if (event == 0)//Statue puzzle.
	{
		if (keyItems_.Contains(gear_))
		{
			scene_->GetChild("statueA")->SetPosition(scene_->GetChild("statueADest")->GetPosition());
			scene_->GetChild("statueB")->SetPosition(scene_->GetChild("statueBDest")->GetPosition());

			String sound = scene_->GetChild("statueC")->GetVar("Sound").GetString();
			scene_->GetChild("statueC")->GetComponent<SoundSource3D>()->Play(
					main_->cache_->GetResource<Sound>(sound));

			String sound2 = theGuy_->GetChild("dontleaveme")->GetVar("Sound").GetString();
			theGuy_->GetChild("dontleaveme")->GetComponent<SoundSource3D>()->Play(
					main_->cache_->GetResource<Sound>(sound2));
		}
	}
	else if (event == 1)//End fire whimper.
	{
		String sound = gillBalentine_->GetChild("whimpersfx")->GetVar("Sound").GetString();
		gillBalentine_->GetChild("whimpersfx")->GetComponent<SoundSource3D>()->Play(
				main_->cache_->GetResource<Sound>(sound));

		String sound2 = theGuy_->GetChild("ineedyou")->GetVar("Sound").GetString();
		theGuy_->GetChild("ineedyou")->GetComponent<SoundSource3D>()->Play(
				main_->cache_->GetResource<Sound>(sound2));
	}
	else if (event == 2)//Guy teleporter.
	{
		theGuy_->GetComponent<RigidBody>()->SetPosition(otherNode->GetChild("destination")->GetComponent<RigidBody>()->GetPosition());

		String sfx = otherNode->GetChild("sfx")->GetVar("sfx").GetString();
		String sound = theGuy_->GetChild(sfx)->GetVar("Sound").GetString();
		theGuy_->GetChild(sfx)->GetComponent<SoundSource3D>()->Play(
				main_->cache_->GetResource<Sound>(sound));

		otherNode->SetEnabledRecursive(false);

		Vector3 victoria = gillBalentine_->GetWorldPosition();
		victoria.y_ = theGuy_->GetWorldPosition().y_;

		theGuy_->LookAt(victoria);

		theGuy_->GetComponent<RigidBodyMoveTo>()->MoveTo(victoria, 1.0f, true);
		theGuy_->GetComponent<AnimationController>()->PlayExclusive("Models/TheGuyWalk.ani", 0, true, 0.0f);
	}
	else if (event == 3)//First fire scream.
	{
		String sound = gillBalentine_->GetChild("screamsfx")->GetVar("Sound").GetString();
		gillBalentine_->GetChild("screamsfx")->GetComponent<SoundSource3D>()->Play(
				main_->cache_->GetResource<Sound>(sound));
		otherNode->GetComponent<CollisionShape>()->SetEnabled(false);

		String sound2 = theGuy_->GetChild("dontrunaway")->GetVar("Sound").GetString();
		theGuy_->GetChild("dontrunaway")->GetComponent<SoundSource3D>()->Play(
				main_->cache_->GetResource<Sound>(sound2));
	}
	else if (event == 4)
	{

	}
	else if (event == 5)//huhh.
	{
		String sound = gillBalentine_->GetChild("huhsfx")->GetVar("Sound").GetString();
		gillBalentine_->GetChild("huhsfx")->GetComponent<SoundSource3D>()->Play(
				main_->cache_->GetResource<Sound>(sound));
		otherNode->GetComponent<CollisionShape>()->SetEnabled(false);
	}
}
