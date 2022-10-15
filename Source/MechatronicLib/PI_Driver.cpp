/**
    (c)2015-2020 Physik Instrumente (PI) GmbH & Co. KG
    Software products that are provided by PI are subject to the General Software License Agreement of 
    Physik Instrumente (PI) GmbH & Co. KG and may incorporate and/or make use of third-party software components. 
    For more information, please read the General Software License Agreement and the Third Party Software Note linked below.

    General Software License Agreement :
    http://www.physikinstrumente.com/download/EULA_PhysikInstrumenteGmbH_Co_KG.pdf
    Third Party Software Note :
    http://www.physikinstrumente.com/download/TPSWNote_PhysikInstrumenteGmbH_Co_KG.pdf
**/

/**
    !! MOTION WARNING !!

    This sample connects to a controller using a connection dialog.
    It shows how to determine min/max position limits, and how to do absolute and relative moves.
    The positioner has to be referenced before starting this sample.
    
**/

#include "PI_Driver.h"


//int PI_Driver::unit_test()
//{
//
//    int iD = OpenConnection (Dialog);
//
//    if (iD < 0)
//    {
//        std::cout << "Unable to connect.";
//        return 1;
//    }
//
//    try
//    {
//        PrintControllerIdentification (iD);
//
//        std::string sAxis = "1";
//
//        SetServoState (iD, sAxis, SERVO_ON);
//
//        double minPosLimit = GetMinPositionLimit (iD, sAxis);
//        double maxPosLimit = GetMaxPositionLimit (iD, sAxis);
//
//        std::cout << "Successfully determined min (" << minPosLimit << ") and max (" << maxPosLimit << ") position limits." << std::endl;
//
//        double range = maxPosLimit - minPosLimit;
//        double targetPos = minPosLimit + (range * 0.1);
//
//        // do absolute motions
//        if (!PI_MOV (iD, sAxis.c_str (), &targetPos))
//        {
//            throw std::runtime_error ("Unable to approach minimum position limit.");
//        }
//
//        WaitForMotionDone (iD, sAxis.c_str ());
//        std::cout << "Successfully approached minimum position limit, to " << targetPos << std::endl;
//
//        targetPos = maxPosLimit - (range * 0.1);
//
//        if (!PI_MOV (iD, sAxis.c_str (), &targetPos))
//        {
//            throw std::runtime_error ("Unable to approach maximum position limit.");
//        }
//
//        WaitForMotionDone (iD, sAxis.c_str ());
//        std::cout << "Successfully approached maximum position limit, to " << targetPos << std::endl;
//
//        targetPos = maxPosLimit * 0.5 * -1;
//
//        // do relative motions
//        if (!PI_MVR (iD, sAxis.c_str (), &targetPos))
//        {
//            throw std::runtime_error ("Unable to do relative move backward.");
//        }
//
//        WaitForMotionDone (iD, sAxis.c_str ());
//        std::cout << "Successfully executed relative move backward by " << targetPos << " units" << std::endl;
//
//        targetPos = targetPos * -1;
//
//        if (!PI_MVR (iD, sAxis.c_str (), &targetPos))
//        {
//            throw new std::runtime_error ("Unable to do relative move forward.");
//        }
//
//        WaitForMotionDone (iD, sAxis.c_str ());
//        std::cout << "Successfully executed relative move forward by " << targetPos << " units" << std::endl;
//
//        SetServoState (iD, sAxis.c_str (), SERVO_OFF);
//        PI_CloseConnection (iD);
//        std::cout << "Closing connection." << std::endl;
//    }
//    catch (std::runtime_error e)
//    {
//        CloseConnectionWithComment (iD, e.what ());
//        return 1;
//    }
//
//    return 0;
//}

//int PI_Driver::OpenConnection(ConnectionType type, std::string hostName)
//{
//    int iD = 0;
//    std::string connType;
//
//    switch (type)
//    {
//        case Dialog:
//            iD = PI_InterfaceSetupDlg ("");
//            connType = "Dialog";
//            break;
//        case RS232:
//            iD = PI_ConnectRS232 (1, 115200);
//            connType = "RS232";
//            break;
//        case TCPIP:
//            iD = PI_ConnectTCPIP (hostName.c_str (), 50000);
//            connType = "TCPIP";
//            break;
//        case USB:
//            char connectedUsbController[1024];
//            int noDevices = PI_EnumerateUSB (connectedUsbController, 1024, "");
//            std::cout << "Found " << noDevices << " controllers connected via USB, connecting to the first one:" << std::endl << connectedUsbController << std::endl;
//            char* pos = strchr (connectedUsbController, '\n');
//
//            if (pos)
//            {
//                *pos = '\0';
//            }
//
//            iD = PI_ConnectUSB (connectedUsbController);
//            connType = "USB";
//            break;
//    }
//
//    if (iD < 0)
//    {
//        ReportError (iD);
//        std::cout << "Connect " << connType << ": ERROR" << std::endl;
//    }
//
//    return iD;
//}

//void PI_Driver::PrintControllerIdentification(int iD)
//{
//    char szIDN[200];
//
//    if (!PI_qIDN (iD, szIDN, 199))
//    {
//        std::runtime_error ("qIDN failed. Exiting.");
//    }
//    std::cout << "qIDN returned: " << szIDN << std::endl;
//}
//
//void PI_Driver::CloseConnectionWithComment(int iD, std::string comment)
//{
//    std::cout << comment << std::endl;
//
//    ReportError (iD);
//    PI_CloseConnection (iD);
//}

double PI_Driver::GetMinPositionLimit(int ID, std::string axis)
{
    double minPosLimit = 0;

    if (!PI_qTMN (ID, axis.c_str (), &minPosLimit))
    {
        throw std::runtime_error ("Unable to get minimum position limit.");
    }

    return minPosLimit;
}

double PI_Driver::GetMaxPositionLimit(int ID, std::string axis)
{
    double maxPosLimit;

    if (!PI_qTMX (ID, axis.c_str (), &maxPosLimit))
    {
        throw std::runtime_error ("Unable to get maximum position limit.");
    }

    return maxPosLimit;
}

bool PI_Driver::WaitForMotionDone(int ID, std::string axis)
{
    BOOL isMoving = TRUE;

    while (isMoving)
    {
        if (!PI_IsMoving (ID, axis.c_str (), &isMoving))
        {
            throw std::runtime_error ("Unable to retrieve motion state.");
        }

        Sleep (100);
    }

    return true;
}


//void PI_Driver::ReportError(int iD)
//{
//    int err = PI_GetError (iD);
//    char szErrMsg[300];
//
//    if (PI_TranslateError (err, szErrMsg, 299))
//    {
//        std::cout << "Error " << err << " occurred: " << szErrMsg << std::endl;
//    }
//}

PI_Driver::PI_Driver(Returns & e_returns, int device_id, std::string servo_name, float steps_per_unit, bool clock_wise, uint16_t current_limit, uint16_t max_velocity, float min_range, float max_range) : IServoDriver(e_returns)
{
}

bool PI_Driver::InitServo(std::shared_ptr<Serializer> p_serializer, bool & servo_needs_cal)
{	
	std::string sAxis = std::to_string(deviceId);// "1";
	minPosLimit = GetMinPositionLimit(pConnection->iD, sAxis);
	maxPosLimit = GetMaxPositionLimit(pConnection->iD, sAxis);
	range = maxPosLimit - minPosLimit;
	return true;
}

bool PI_Driver::UnInitServo()
{
	return true;
}

bool PI_Driver::Calibrate()
{
	return true;
}

bool PI_Driver::MoveServoUnitsAbs(float new_position_units)
{
	double targetPos = minPosLimit + (range * 0.1);
	std::string sAxis = std::to_string(deviceId);
	// do absolute motions
	if (!PI_MOV(pConnection->iD, sAxis.c_str(), &targetPos))
	{
		return false;
		// throw std::runtime_error("Unable to approach minimum position limit.");
	}

	WaitForMotionDone(pConnection->iD, sAxis.c_str());
	std::cout << "Successfully approached minimum position limit, to " << targetPos << std::endl;
	return true;
}

bool PI_Driver::GetPosition(bool wait_for_stop)
{
	return true;
}

bool PI_Driver::CutTorque()
{
	return true;
}

bool PI_Driver::SetSerialNumber(uint16_t serial_number)
{
	return true;
}

bool PI_Driver::GetSerialNumber(uint16_t & serial_number)
{
	return true;
}
