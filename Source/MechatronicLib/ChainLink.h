#pragma once
#include "Mechanism.h"
#include "../Includes/tiny_obj_loader.h"
#include "../RenderEngine/Direct3D/Direct3DModel.h"


class ChainLink : public Mechanism
{

public:
	ChainLink(Returns& e_returns, CALLBACKS& v_callbacks, LdrMechanism mechanism_ldr, std::shared_ptr<ServoControl> servo_control, std::shared_ptr<KinematicChain> parent_chain = nullptr);
	~ChainLink();

	virtual bool Init() override;

	//bool CalibrateJoint(std::string chain_name, std::string joint_name);

private:
	//ADD FUNCTIONS
	bool AddChain(LdrChain chain_ldr);
	bool AddJoint(std::string chain_name, LdrJoint joint_ldr);
};

